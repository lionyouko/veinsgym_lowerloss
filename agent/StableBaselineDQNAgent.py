##!/usr/bin/env python3
# !/home/parallels/tools_thesis/omnetpp-5.6.2/samples/lowerlossRL1/venv/bin/python3
"""
DQN base example agent for lowerloss environment.
"""
import time
import logging
import os
import random
import sys
import gym
import veins_gym
from veins_gym import veinsgym_pb2

from stable_baselines3 import DQN
from stable_baselines3.common.env_checker import check_env
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
    # stuff that goes here needs to be the same in the init of the env class
    kwargs={
        "scenario_dir": os.path.relpath(
            os.path.join(
                os.path.dirname(os.path.abspath(__file__)), "..", "lowerloss"
            )
        ),

        "timeout": 50.0,
        "print_veins_stdout": True,
        "action_serializer": serialize_action,
        "run_veins": True,  # start veins through Veins-Gym
        "port": 5551,  # pick a port to use
        # to run in a GUI, use:
        "user_interface": "Cmdenv",
        # "timeout": -1,
        "config": "MultipleGymsOne"
    },
)


def main():
    """
    Run the trivial agent.
    """
    logging.basicConfig(level=logging.DEBUG)

    env = gym.make("veins-v1")
    observation = env.reset()
    
    print("Observation space:", env.observation_space)
    print("Shape:", env.observation_space.shape)
    print("Action space:", env.action_space)

    check_env(env)
    logging.info("Env created")

    # RSU_ID*, Task(maxDelay,NumberCycles, Size), Vehicle(Coordinates, Speed), Queues(T,N)* observation space Float32
    # action space Discrete(2) stay with the task, or send it to another RSU, to which RSU to send

    model = DQN('MlpPolicy', env, verbose=1)
    model.learn(total_timesteps=2500)
    
    episodes = 1
    rewards = []

    for episode in range(0, episodes):
        observation = env.reset()
        logging.info("Env reset for episode %s", episode)

        action, _state = model.predict(observation,deterministic=True)
        
        observation, reward, done, info = env.step(action)

        while not done:
        
            action, _state = model.predict(observation,deterministic=True)
            
            observation, reward, done, info = env.step(action)
            #env.render()
            
            rewards.append(reward)
            # lion 1604022> the last action sent by omnetpp to veinsgym will be a shutdown
            # and that will make the done set to true. In the veinsgym code that this happens, it will
            # also generates a random of the observation (_parse request function)
            # that will be sent in step function
            # and must then be discarded (as it is an undesired step + 1 anyway)
            if not done:
                logging.debug(
                    "Last action: %s, Reward: %.3f, Observation: %s",
                    action,
                    reward,
                    observation,
                )
        print("Number of steps taken:", len(rewards))
        print("Mean reward:", sum(rewards) / len(rewards))
        print()
        rewards = []
        time.sleep(0.059)


if __name__ == "__main__":
    main()

