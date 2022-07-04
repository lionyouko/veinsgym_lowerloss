# veinsgym_lowerloss
This repo is an example of scenario using veinsgym tool

If you already have veinsgym, gym, omnet, protobuf and their dependencies installed, just import this project to omnet and it may work just fine.
You can also simply start the venv you use to run the python scripts and run TrivialRL.

If you aren't familiar with all above, try the following steps (that are guided to linux/ubuntu, so you may need to adapt to your system):

To run this project, it is needed to:

1. Install omnetpp dependencies 
2. Install omnet++ 
3. Download INET 
4. Download Veins 
5. Import INET and Veins to omnet++ (using gui, right click in Project Explorer > import).
6. Build INET and Veins (Right Click each > Build Configurations > Build Selected).
7. Download or clone this Project.
8. Import this project to omnet.
9. If not Project Referenced, right click the project, then Properties > Projects Reference > INET + Veins.
10. Install ZMQ and Protobuf dependencies: 10.1. ZMQ: 10.2. First check protobuf version on your machine with: 10.3. Download the same version using: 10.4. Build protobuf. You can skip this part if you have working protobuf.
11. Go to the project, Properties > Omnet++ > Makemake > click src:makemake(deep, recursive) and Build > Options, then: 11.1 At Compile tab, check all. 11.2 At Link tab > additional libraries > add "zmq" and "protobuf". 11.3 At Custom tab add: MSGC:=$(MSGC) --msg6
12. Build the project
13. Go to agent dir, start a new venv, activate it and pip install gym and veinsgym

Now it is ready to be used by AI Agent.

14. With venv activated at agent dir, just type python TrivialRL.py and run. 

You may want to recompile/rebuild if not working with the binaries here.

Attention that there is no snakemake or similar, so it must be compiled that way for now.
