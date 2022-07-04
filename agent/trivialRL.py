##!/usr/bin/env python3
# !/home/parallels/tools_thesis/omnetpp-5.6.2/samples/lowerlossRL1/venv/bin/python3
"""
Trivial example agent for the dcc environment.
"""
import time
import logging
import os
import random
import sys
import gym
import veins_gym
from veins_gym import veinsgym_pb2

"""
	This function needs to be adapted for your necessities
"""


def serialize_action(actions):
    """Searialize a list of floats into an action."""
    reply = veinsgym_pb2.Reply()
    if not hasattr(actions, '__iter__'):
        actions = [actions]
    reply.action.box.values.extend(actions)

    return reply.SerializeToString()


gym.register(
    id="veins-v1",
    entry_point="veins_gym:VeinsEnv",
    kwargs={
        "scenario_dir": os.path.relpath(
            os.path.join(
                os.path.dirname(os.path.abspath(__file__)), "..", "properlowerloss"
            )
        ),

        "timeout": 30.0,
        "print_veins_stdout": True,
        "action_serializer": serialize_action,
        "run_veins": True,  # do not start veins through Veins-Gym
        "port": 5551,  # pick a port to use
        # to run in a GUI, use:
        "user_interface": "Cmdenv",
        # "timeout": -1,
    },
)


def main():
    """
    Run the trivial agent.
    """
    logging.basicConfig(level=logging.DEBUG)

    env = gym.make("veins-v1")
    logging.info("Env created")

    # env.reset()
    # logging.info("Env reset")
    done = False
    # fixed_action = [random.randint(0, 1)]
    episodes = 5
    rewards = []

    for episode in range(0, episodes):
        env.reset()
        logging.info("Env reset for episode %s", episode)
        observation, reward, done, info = env.step(random.randint(0, 1))

        while not done:
        
            r_action = random.randint(0, 1)
            observation, reward, done, info = env.step(r_action)
            rewards.append(reward)
            # lion 1604022> the last action sent by omnetpp to veinsgym will be a shutdown
            # and that will make the done set to true. In the veinsgym code that this happens, it will
            # also generates a random of the observation (_parse request function)
            # that will be sent in step function
            # and must then be discarded (as it is an undesired step + 1 anyway)
            if not done:
                logging.debug(
                    "Last action: %s, Reward: %.3f, Observation: %s",
                    r_action,
                    reward,
                    observation,
                )
        print("Number of steps taken:", len(rewards))
        print("Mean reward:", sum(rewards) / len(rewards))
        rewards = []
        time.sleep(0.059)


if __name__ == "__main__":
    main()
