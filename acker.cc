/*
 * acker.cc
 *
 *  Created on: Mar 15, 2022
 *      Author: lion-silva
 */


#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

/**
 * Sends back an acknowledgement -- or not.
 */
class Acker : public cSimpleModule
{
  protected:
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Acker);

void Acker::handleMessage(cMessage *msg)
{

    EV << msg << " received, sending back an acknowledgment.\n";
    cMessage *ack = new cMessage("acker");
    ack->addPar("index");
    msg->par("index").setLongValue(int(msg->par("index")));
    send(ack, "hotlineSender", int(msg->par("index")));
    delete msg;
}
