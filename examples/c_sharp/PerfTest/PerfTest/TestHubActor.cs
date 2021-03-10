using Akka.Actor;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PerfTest
{
    class TestHubActor : ReceiveActor
    {
        #region in messages
        public class IdTestDone { }
        public class SetTestDone { }
        public class SetAndReadTestDone { }
        #endregion

        IActorRef _client;
        IActorRef _test;
        int _runs;

        public TestHubActor(int runs)
        {
            _runs = runs;
            Become(WaitForConnection);
        }

        private void WaitForConnection()
        {
            Receive<TcpDataClientActor.ConnectedMsg>(mes =>
            {
                _client = Sender;
                _test = Context.ActorOf(IdSpeedTest.Props(_client, _runs));
                Context.Watch(_test);

                Become(WaitForIdTestState);
            });

            Receive<TcpDataClientActor.ConnectionFailed>(mes =>
            {

            });
        }

        private void WaitForIdTestState()
        {
            Receive<Terminated>(mes =>
            {
                _test = Context.ActorOf(SetDigitalOutTest.Props(_client, _runs));
                Context.Watch(_test);

                Become(WaitForSetTestState);
            });
        }

        private void WaitForSetTestState()
        {
            Receive<Terminated>(mes =>
            {
                _test = Context.ActorOf(SetAndReadTest.Props(_client, _runs));
                Context.Watch(_test);

                Become(WaitForSetAndReadTestState);
            });
        }

        private void WaitForSetAndReadTestState()
        {
            Receive<Terminated>(mes =>
            {
                Console.WriteLine("all tests done");
            });
        }

        public static Props Props(int runs) => Akka.Actor.Props.Create(() => new TestHubActor(runs));
    }
}
