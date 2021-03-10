using Akka.Actor;
using Akka.IO;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PerfTest
{
    class Program
    {
        static void Main(string[] args)
        {
            ActorSystem system = ActorSystem.Create("TestLabNet");
            IActorRef tcpManager = system.Tcp();

            IActorRef tester = system.ActorOf(TestHubActor.Props(10_000));
            IActorRef client = system.ActorOf(TcpDataClientActor.Props("192.168.137.101", 8080, tester));

            Console.ReadLine();
        }
    }
}
