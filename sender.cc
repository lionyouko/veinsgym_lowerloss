/*
 * sender.cc
 *
 *  Created on: Mar 15, 2022
 *      Author: ly
 */

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

/**
 * Sender starts the simulation by sending the firdt message. Its role is to send X messages
 * randomly by the nodes lossY and lossZ and check if the packages got lost or back
 * using simple timeouts given guarantees of delay and speed in the network.
 */
class Sender : public cSimpleModule
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

    //holds how many sent and how many expired
    int total = 0;
    int expired = 0;

    //maximum number of messages (review if can be property in .ned or .ini)
    const int MAX_MESSAGES = 1000;

  public:
    Sender();
    virtual ~Sender();

  protected:
    virtual cMessage *generateNewMessage();
    virtual void sendCopyOf(cMessage *msg, const char* gate);
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;
    virtual void forwardMessage(cMessage *msg);
};

Define_Module(Sender);

Sender::Sender()
{
    timeoutEvent = message = newMessageEvent = nullptr;
}

Sender::~Sender()
{
    cancelAndDelete(timeoutEvent);
    cancelAndDelete(newMessageEvent);
    //delete message;
}

void Sender::initialize()
{
    // Initialize variables.
    seq = 0;
    timeout = 1.0;
    timeoutEvent = new cMessage("timeoutEvent");

    newMessageTimeout = 2.0;
    newMessageEvent = new cMessage("newMessageEvent");

    // Generate and send initial message.
    EV << "Sending initial message\n";
    message = generateNewMessage();
    forwardMessage(message);
    delete message;

    scheduleAt(simTime()+timeout, timeoutEvent);
    scheduleAt(simTime()+newMessageTimeout,newMessageEvent);
}

void Sender::handleMessage(cMessage *msg)
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
        EV << "Received: " << msg->getName() << "\n";
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

cMessage *Sender::generateNewMessage()
{
    // Generate a message with a different name every time.
    char msgname[20];
    sprintf(msgname, "sender-%d", ++seq);
    cMessage *msg = new cMessage(msgname);
    return msg;
}

void Sender::sendCopyOf(cMessage *msg, const char* gate)
{
    // Duplicate message and send the copy.
    cMessage *copy = (cMessage *)msg->dup();
    send(copy, gate);
    ++total;
}

void Sender::forwardMessage(cMessage *msg)
{
    // In this example, we just pick a random gate to send it on.
    // We draw a random number between 0 and the size of gate `out[]'.
    int n = gateSize("out");
    int k = intuniform(0, n-1);

    // Duplicate message and send the copy.
    cMessage *copy = (cMessage *)msg->dup();

    EV << "Forwarding message " << copy << " on port out[" << k << "]\n";
    send(copy, "out", k);
    ++total;
}

void Sender::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "exp: %d snt: %d", expired, total);
    getDisplayString().setTagArg("t", 0, buf);
}



