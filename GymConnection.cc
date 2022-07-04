//
// Copyright (C) 2020 Dominik S. Buse <buse@ccs-labs.org>, Max Schettler <schettler@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "GymConnection.h"

Define_Module(GymConnection);

void GymConnection::initialize() {

    this->connectionEnable = par("enable").boolValue();

    // do we want to use this at all?
    if (!par("enable")) {
        return;
    }

    //determine host and port from params and ENV variables
    this->host = par("host").stdstringValue();
    this->port = par("port").intValue();
    if (host == "") { // param empty, check environment
        if (std::getenv("VEINS_GYM_HOST") != nullptr) {
            host = std::getenv("VEINS_GYM_HOST");
        } else {
            // param and environment empty, fail
            throw omnetpp::cRuntimeError("Gym host not configured! Change your ini file or set VEINS_GYM_HOST");
        }
    }

    if (port < 0) { // param "empty", check environment
        if (std::getenv("VEINS_GYM_PORT") != nullptr) {
            port = std::atoi(std::getenv("VEINS_GYM_PORT"));
        } else {
            // param and environment empty, fail
            throw omnetpp::cRuntimeError(
                    "Gym port not configured! Change your ini file or set VEINS_GYM_PORT");
        }
    }

    //180622 -> to multiple agents
    EV_INFO << "Connecting to server 'tcp://" << this->host << ":" << this->port << "'\n";
    this->address = "tcp://" + this->host + ":" + std::to_string(this->port);
    socket.connect(address);

    veinsgym::proto::Request init_request;
    *(init_request.mutable_init()->mutable_observation_space_code()) = par(
            "observation_space").stdstringValue();
    *(init_request.mutable_init()->mutable_action_space_code()) = par(
            "action_space").stdstringValue();
    communicate(init_request); // ignore (empty) reply

    // now talk to the agent again to get the first action
    EV_INFO << "GymConnection asking the agent for the initial config\n";
    veinsgym::proto::Request action_request;
    action_request.set_id(0);
    std::array<double, 3> observations = { 0.0, 0.0, 0.0 };
    for (size_t i = 0; i < observations.size(); ++i) {
        action_request.mutable_step()->mutable_observation()->mutable_box()->mutable_values()->Add();
        action_request.mutable_step()->mutable_observation()->mutable_box()->set_values(
                i, observations[i]);
    }
    action_request.mutable_step()->mutable_reward()->mutable_box()->mutable_values()->Add();
    action_request.mutable_step()->mutable_reward()->mutable_box()->set_values(
            0.0, 0.0);
    auto reply = communicate(action_request);
    EV_INFO << "GymConnection got action values: ";
    size_t index = 0;
    for (auto value : reply.action().box().values()) {

        ++index;
        EV_INFO << value << ", ";
    }
    EV_INFO << std::endl;
    ASSERT(index == 4);
}


// 16062022
GymConnection::GymConnection()
{
    //std::cout << "i constructor am being called?" << std::endl;
    context = zmq::context_t(1);
    socket = zmq::socket_t(context, zmq::socket_type::req);
    i = 0;
}

// 16062022 - 210622
GymConnection::~GymConnection()
{   //std::cout << "i destructor am being called?" << std::endl;
    if (socket.connected()) {
        socket.unbind(this->address);
        socket.close();
    }
}

void GymConnection::finish()
{
//1   6062022 - 210622 trying to get hid of the zmq error
//    socket.unbind(this->address);
//    socket.close();
}

bool GymConnection::isConnectionEnable() const {
    return this->connectionEnable;
}

veinsgym::proto::Reply GymConnection::communicate(
        veinsgym::proto::Request request) {
    std::string request_msg = request.SerializeAsString();
    //std::cout << "socket no c --- " << i  << std::endl;
    //i++;
    socket.send(zmq::message_t(request_msg.data(), request_msg.size()),
            zmq::send_flags::none);
    zmq::message_t response_msg;
    socket.recv(response_msg);
    std::string response(static_cast<char*>(response_msg.data()),
            response_msg.size());
    veinsgym::proto::Reply reply;
    reply.ParseFromString(response);
    return reply;
}

int GymConnection::getPort() const {
    return this->port;
}
