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
        IActorRef _client;
        List<double> _stats;
        int _counter;
        long _tics;
        int _runs = 10_000;

        public IdSpeedTest(IActorRef client, int runs)
        {
            _client = client;
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
                    double percentile95 = Statistics.Percentile(_stats, 95);
                    double min = Statistics.Minimum(_stats);
                    double max = Statistics.Maximum(_stats);

                    Console.WriteLine($"\rmean: {mean:0.00} std: {sdv:0.00} median: {median:0.00} p95: {percentile95:0.00} min: {min:0.00} max: {max:0.00}");
                    Context.Stop(Self);
                }
            });
        }


        public static Props Props(IActorRef client, int runs) => Akka.Actor.Props.Create(() => new IdSpeedTest(client, runs));
    }
}
