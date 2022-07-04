/*
 * loss.cc
 *
 *  Created on: Mar 15, 2022
 *      Author: ly
 */

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

/**
 * In this step we'll introduce random numbers. We change the delay from 1s
 * to a random value which can be set from the NED file or from omnetpp.ini.
 * In addition, we'll "lose" (delete) the packet with a small probability.
 */
class Loss : public cSimpleModule
{
  private:
    double dropPercentage;

    int dropped = 0;
    int total = 0;

  public:
    Loss();
    virtual ~Loss();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;
};

Define_Module(Loss);

Loss::Loss()
{

}

Loss::~Loss()
{
}

void Loss::initialize()
{
    dropPercentage = par("dropPercentage");
    dropPercentage /= 100;
}

void Loss::handleMessage(cMessage *msg)
{
    // if message comes from acker, send it back to sender
    // I never used this if as acker has hotline to sender
    if ((strcmp((*msg).getName(),"acker") == 0)) {
        //send(msg,"out1");
        //send(msg,"outs");
    }
    else {
        // "Lose" the message with DropPecentage probability:
        ++total;
        if (uniform(0, 1) < dropPercentage) {
            ++dropped;
            bubble("Losing message!");
            delete msg;
        }
        else {
            //Send to acker

            EV << "Message sent forward to acker\n";
            send(msg,"out2");

        }
    }
}

void Loss::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "drp: %d rcsndr: %d", dropped, total);
    getDisplayString().setTagArg("t", 0, buf);
}
