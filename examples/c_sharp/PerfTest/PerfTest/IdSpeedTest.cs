using Akka.Actor;
using LabNetProt.Client;
using MathNet.Numerics.Statistics;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PerfTest
{
    class IdSpeedTest : ReceiveActor
    {
        #region out messages
        public class IdSpeedResults
        {
            public IdSpeedResults(List<double> latencies)
            {
                Latencies = latencies;
            }

            public IEnumerable<double> Latencies { get; private set; }
        }
        #endregion

        IActorRef _client;
        IActorRef _testHub;
        List<double> _stats;
        int _counter;
        long _tics;
        int _runs = 10_000;

        public IdSpeedTest(IActorRef client, IActorRef testHub, int runs)
        {
            _client = client;
            _testHub = testHub;
            _runs = runs;
            _stats = new List<double>(_runs);
            _counter = 0;

            Console.WriteLine("start id test");

            LabNetIdRequest cwm = new LabNetIdRequest();
            _client.Tell(new TcpDataClientActor.SetMessageReceiver(Self));
            _client.Tell(cwm);
            _tics = HPT.GetTime;

            Become(TestState);
        }

        private void TestState()
        {
            Receive<LabNetProt.Server.LabNetIdReply>(swm =>
            {
                var curTime = HPT.GetTime;
                var dur = curTime - _tics;
                _stats.Add(HPT.ToMillisesonds(dur));

                _counter++;

                if (_counter < _runs)
                {
                    Console.Write($"\r{_counter} from {_runs}");
                    LabNetIdRequest cwm = new LabNetIdRequest();

                    _tics = HPT.GetTime;
                    _client.Tell(cwm);
                }
                else
                {
                    double mean = Statistics.Mean(_stats);
                    double median = Statistics.Median(_stats);
                    double sdv = Statistics.StandardDeviation(_stats);
                    double p25 = Statistics.Quantile(_stats, 0.25);
                    double p75 = Statistics.Quantile(_stats, 0.75);
                    double p97_5 = Statistics.Quantile(_stats, 0.975);
                    double p2_5 = Statistics.Quantile(_stats, 0.025);

                    Console.WriteLine($"\rmean: {mean:0.00} std: {sdv:0.00} median: {median:0.00} p25: {p25:0.00} p75: {p75:0.00} p2.5: {p2_5:0.00} p97.5: {p97_5:0.00}");
                    _testHub.Tell(new IdSpeedResults(_stats));
                }
            });
        }


        public static Props Props(IActorRef client, IActorRef testHub, int runs) => Akka.Actor.Props.Create(() => new IdSpeedTest(client, testHub, runs));
    }
}
