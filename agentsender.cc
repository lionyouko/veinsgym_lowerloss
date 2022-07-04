/*
 * agentsender.cc
 *
 *  Created on: Mar 23, 2022
 *      Author: ly
 */

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "GymConnection.h"
#include "veins/base/utils/FindModule.h"
using namespace omnetpp;



/**
 * Sender starts the simulation by sending the first message. Its role is to send X messages
 * following what AI agent tells to do by the nodes lossY and lossZ and check if the packages got lost or back
 * using simple timeouts given guarantees of delay and speed in the network,
 * and sending to the agent observations of the environment.
 */
class AgentSender : public cSimpleModule
{
  private:
    simtime_t timeout;  // timeout
    cMessage *timeoutEvent;  // holds pointer to the timeout self-message

    // evaluates randomly new time to schedule to send messages (every time = volatile)
    simtime_t newMessageTimeout;
    cMessage *newMessageEvent;

    // sequence number and messages to be sent
    int seq;  // message sequence number
    cMessage *message;

    //holds how many sent and how many are expired now and how many where expired last complete event cycle
    int total = 0;
    int expired = 0;
    int expired_before = 0;

    //maximum number of messages (review if can be property in .ned or .ini)
    const int MAX_MESSAGES = 1000;

    //veinsgym connection wizard and related functions
    GymConnection *gymCon = nullptr;

    //if no connection, then go random
    bool useVeinsGym = false;

    veinsgym::proto::Request serializeObservation(const std::array<double, 3> &observation, double reward) const;
    std::array<double,3> computeObservation () const;
    double computeReward() const;
    int action() const;

  public:
    AgentSender();
    virtual ~AgentSender();

  protected:
    virtual cMessage *generateNewMessage();
    virtual void sendCopyOf(cMessage *msg, const char* gate);
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;
    virtual void forwardMessage(cMessage *msg);
    virtual void finish() override;


};

Define_Module(AgentSender);

AgentSender::AgentSender()
{
    timeoutEvent = message = newMessageEvent = nullptr;
}

AgentSender::~AgentSender()
{
    cancelAndDelete(timeoutEvent);
    cancelAndDelete(newMessageEvent);
    //210622 trying to get fixed the following:
    //<!> Error: zmq::error_t: Interrupted system call --
    // in module (AgentSender) Lowloss.agentsender[0] (id=M), at t=Xs, event #N
    delete message;

    // 16042022 -> Important omnetpp lesson: when simulation finishes, there is no destruction of
    // the objects. No destructor is called. Only when we rebuild the simulation is that the destructors
    // are called and then rebuild then constructors again.

    // stop veins-gym gymcomm
    // lion 16042022 -> if called at finish no need to put it here
    // later on we need to see if it will cause any problem when statistics
//    if (gymCon) {
//            veinsgym::proto::Request request;
//            request.set_id(1);
//            *(request.mutable_shutdown()) = {};
//            auto response = gymCon->communicate(request);
//    }
}

void AgentSender::initialize()
{
    // Initialize variables.
    seq = 0;
    timeout = 1.0;
    timeoutEvent = new cMessage("timeoutEvent");

    newMessageTimeout = 2.0;
    newMessageEvent = new cMessage("newMessageEvent");

    //init veinsgym connection

    //190622 -> seems it work finding gymconnector n
    gymCon = veins::FindModule<GymConnection*>::findSubModules(getParentModule())[getIndex()];
    EV << "Found module ---------------" + gymCon->getPort();
    ASSERT(gymCon);

    this->useVeinsGym = gymCon->isConnectionEnable();

    // Generate and send initial message.
    EV << "Sending initial message\n";

    message = generateNewMessage();
    forwardMessage(message);
    delete message;

    scheduleAt(simTime()+timeout, timeoutEvent);
    scheduleAt(simTime()+newMessageTimeout,newMessageEvent);

}

void AgentSender::finish()
{

    if (gymCon) {
            veinsgym::proto::Request request;
            request.set_id(1);
            *(request.mutable_shutdown()) = {};
            auto response = gymCon->communicate(request);
    }
}

void AgentSender::handleMessage(cMessage *msg)
{


    if (msg == timeoutEvent) {
        // If we receive the timeout event, that means the packet hasn't
        // arrived in time.
        EV << "Timeout expired, expired messages counter + 1 and restarting timer\n";
        //sendCopyOf(message);

        ++expired;


    }
    else if (msg == newMessageEvent) {
        if (total < MAX_MESSAGES) {
            message = generateNewMessage();
            forwardMessage(message);
            delete message;

            // schedule new message to present time plus 2 seconds
            cancelEvent(newMessageEvent);
            scheduleAt(simTime()+newMessageTimeout, newMessageEvent);

            // when sending message, also schedule new timeout for it
            scheduleAt(simTime()+timeout, timeoutEvent);

        }
    }
    else {

        // message arrived
        // Acknowledgement received!
        expired_before = expired;
        //210622 -> added the par so we show the index to see if the gate is correct
        EV << "Received: " << msg->getName() << " and " << int(msg->par("index")) << " and my index: "  << this->getIndex() << "\n";
        delete msg;

        // Also delete the stored message and cancel the timeout event.
        EV << "Timer cancelled.\n";
        cancelEvent(timeoutEvent);


        // Ready to send another one.
        // message = generateNewMessage();
        // sendCopyOf(message);
        // scheduleAt(simTime()+timeout, timeoutEvent);
    }
}

cMessage *AgentSender::generateNewMessage()
{
    // Generate a message with a different name every time.
    char msgname[20];
    sprintf(msgname, "sender-%d", ++seq);
    cMessage *msg = new cMessage(msgname);
    //210622
    msg->addPar("index");
    msg->par("index").setLongValue(this->getIndex());

    return msg;
}

void AgentSender::sendCopyOf(cMessage *msg, const char* gate)
{
    // Duplicate message and send the copy.
    cMessage *copy = (cMessage *)msg->dup();
    send(copy, gate);
    ++total;
}

void AgentSender::forwardMessage(cMessage *msg)
{
    // In this example, we just pick gate chosen by the RL Agent.
    // We draw a number between 0 (Loss30) and 1 (Loss70).

    int n = gateSize("out");
    //int k = intuniform(0, n-1);
    int next_action = -1;
    if (useVeinsGym){
        next_action = action();
    } else {
        next_action = intuniform(0, n-1);
    }

    // Duplicate message and send the copy.
    cMessage *copy = (cMessage *)msg->dup();

    EV << "Forwarding message " << copy << " on port out[" << next_action << "]\n";
    //send(copy, "out", k);
    send(copy, "out", next_action);
    ++total;
}

void AgentSender::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "exp: %d snt: %d", expired, total);
    getDisplayString().setTagArg("t", 0, buf);
}

double AgentSender::computeReward() const
{
    // If from last step the expired number didn't change, so message was sent
    // successfully, then reward is positive and 1, otherwise no reward
    return expired_before == expired ? 1.0 : 0.0;
}

std::array<double,3> AgentSender::computeObservation () const
{
    // Current observation is simply the expired and total messages sent
    return {double(expired), double(expired_before), double(total)};
}

veinsgym::proto::Request AgentSender::serializeObservation(const std::array<double, 3> &observation, const double reward) const {
    veinsgym::proto::Request request;
    //request.set_id(1);
    auto *values = request.mutable_step()->mutable_observation()->mutable_box()->mutable_values();
    *values = {observation.begin(), observation.end()};
    request.mutable_step()->mutable_reward()->mutable_box()->mutable_values()->Add();
    request.mutable_step()->mutable_reward()->mutable_box()->set_values(0, reward);
    return request;
}

int AgentSender::action() const
{
//    gymCon = veins::FindModule<GymConnection*>::findGlobalModule();
//    ASSERT(gymCon);
    //std::cout << "i am being called? 2" << std::endl;
    const auto observation = computeObservation();
    const auto reward = computeReward();
    const auto request = serializeObservation(observation, reward);
    auto response = gymCon->communicate(request);
    // since serialize_action in python code uses action.box.values, we need to unbox as the same
    // to be able to get the values. The protocol buffer capsule must be followed.
    auto result = response.action().box().values()[0];
    EV << "GOT ACTION: " << result <<endl;
    return result;
}


