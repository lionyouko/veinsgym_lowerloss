import org.car2x.veins.base.connectionManager.ConnectionManager;
import GymConnection;



simple Loss
{
    parameters:
        @display("i=block/routing"); //add a default icon
        double dropPercentage = default(0);
        

    gates:
        //input in1;
        //input in2 @loose;
        //output out1;
        output out2 @loose;
        input ins[];
        //output outs[];
}

simple Loss70 extends Loss
{
    parameters:
        @display("i=,red");
        dropPercentage = 70;
}

simple Loss30 extends Loss
{
    parameters:
        @display("i=,green");
        dropPercentage = 30;
}

simple Acker
{
    parameters:
        @display("i=block/routing");
    gates:
        input in[];
        output out[];
        output hotlineSender[];
}

simple Sender
{
    parameters:
        @display("i=block/routing");
    gates:
        input in[];
        output out[];
}

simple AgentSender
{
    parameters:
        @display("i=block/routing");
    gates:
        input in[];
        output out[];
}


network Lowloss
{
    @display("bgb=400,400");
    
    int numAgents = default(1);
    
    types:
        channel C extends ned.DatarateChannel
        {
            datarate = 10Mbps;
            delay = 10ms;
        }

    submodules:
//        sender: Sender {
//            @display("p=200,50;i=,cyan");
//        }
        acker: Acker {
            @display("p=200,350;i=,gold");
        }
        loss70: Loss70 {

            @display("p=250,200");
        }
        loss30: Loss30 {
            @display("p=150,200");
        }

        //veins-gym section
//        connectionManager: ConnectionManager {
//            parameters:
//                @display("p=57,89;i=abstract/multicast");
//        }

        // Multiples Gyms and Agents
        gym_connection[numAgents]: GymConnection {
            @display("p=57,161");
        }

        agentsender[numAgents]: AgentSender {

            @display("p=200,50;i=,cyan");
        }
    connections:
        // Outs sender
        //sender.out++ --> C --> loss70.in1;
        //sender.out++ --> C --> loss30.in1;

        // Outs loss'es part1
        loss70.out2 --> C --> acker.in++;
        loss30.out2 --> C --> acker.in++;

        // Outs loss'es part2 to sender
        //loss70.out2 --> C --> sender.in++;
        //loss30.out2 --> C --> sender.in++;

        // Outs acker to losses and sender
        acker.out++ --> C --> loss70.ins++;
        acker.out++ --> C --> loss30.ins++;
        //cheating, must create a proper message object with src and dest
        //acker.hotlineSender --> C --> sender.in++;


        // Veins-gym section
        for i=0..numAgents-1 {
        	agentsender[i].out++ --> C --> loss70.ins++;
        	agentsender[i].out++ --> C --> loss30.ins++;
        	acker.hotlineSender++ --> C --> agentsender[i].in++;
        }


}
