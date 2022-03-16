using Akka.Actor;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
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

        IEnumerable<double> _idLatencies;
        IEnumerable<double> _setLatencies;
        IEnumerable<double> _setAndReadLatencies;

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
                _test = Context.ActorOf(IdSpeedTest.Props(_client, Context.Self, _runs));

                Become(WaitForIdTestState);
            });

            Receive<TcpDataClientActor.ConnectionFailed>(mes =>
            {

            });
        }

        private void WaitForIdTestState()
        {
            Receive<IdSpeedTest.IdSpeedResults>(mes =>
            {
                Context.Stop(_test);
                _idLatencies = mes.Latencies;
                _test = Context.ActorOf(SetDigitalOutTest.Props(_client, Context.Self, _runs));

                Become(WaitForSetTestState);
            });
        }

        private void WaitForSetTestState()
        {
            Receive<SetDigitalOutTest.SetDigitalOutResults>(mes =>
            {
                Context.Stop(_test);
                _setLatencies = mes.Latencies;
                _test = Context.ActorOf(SetAndReadTest.Props(_client, Context.Self, _runs));

                Become(WaitForSetAndReadTestState);
            });
        }

        private void WaitForSetAndReadTestState()
        {
            Receive<SetAndReadTest.SetAndReadResults>(mes =>
            {
                _setAndReadLatencies = mes.Latencies;
                Context.Stop(_test);

                Console.WriteLine("all tests done");

                Console.WriteLine("save latencies");
                try
                {
                    using (StreamWriter writer = new StreamWriter("latencies.csv", false, Encoding.ASCII))
                    {
                        CultureInfo culture = new CultureInfo("en-US");
                        Thread.CurrentThread.CurrentCulture = culture;

                        writer.WriteLine("IdTest;SetTest;SetAndRead");

                        int lNbr = Math.Min(_idLatencies.Count(), _setLatencies.Count());
                        lNbr = Math.Min(lNbr, _setAndReadLatencies.Count());

                        for (int i = 0; i < lNbr; i++)
                        {
                            writer.Write(_idLatencies.ElementAt(i).ToString("N3"));
                            writer.Write(";");
                            writer.Write(_setLatencies.ElementAt(i).ToString("N3"));
                            writer.Write(";");
                            writer.Write(_setAndReadLatencies.ElementAt(i).ToString("N3"));
                            writer.WriteLine();
                        }
                    }

                    Console.WriteLine("save latencies done");
                }
                catch (Exception ex)
                {
                    Console.WriteLine("save latencies error: " + ex.Message);
                }
            });
        }

        public static Props Props(int runs) => Akka.Actor.Props.Create(() => new TestHubActor(runs));
    }
}
