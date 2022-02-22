using Akka.Actor;
using LabNetProt;
using LabNetProt.Client;
using LabNetProt.Server;
using MathNet.Numerics.Statistics;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PerfTest
{
    class SetDigitalOutTest : ReceiveActor
    {
        IActorRef _client;
        int _runs;
        int _counter = 0;
        List<double> _stats;
        long _tics;

        public SetDigitalOutTest(IActorRef client, int runs)
        {
            _client = client;
            _runs = runs;
            _stats = new List<double>(_runs);

            _client.Tell(new TcpDataClientActor.SetMessageReceiver(Self));

            Become(ResetState);
        }

        private void ResetState()
        {
            LabNetResetRequest reset = new LabNetResetRequest();
            _client.Tell(reset);

            Receive<LabNetResetReply>(mes =>
            {
                Become(InitGpioState);
            });
        }

        private void InitGpioState()
        {
            GpioWiringPiInit cwm = new GpioWiringPiInit();
            _client.Tell(cwm);

            Receive<InterfaceInitResult>(swm =>
            {
                if (swm.IsSucceed)
                {
                    Become(InitDigitalOutState);
                }
                else
                {
                    Console.WriteLine("gpio init error");
                }
            });
        }

        private void InitDigitalOutState()
        {
            GpioWiringPiInitDigitalOut cwm = new GpioWiringPiInitDigitalOut { Pin = 5, IsInverted = false };
            _client.Tell(cwm);

            Receive<DigitalOutInitResult>(swm =>
            {
                if (swm.IsSucceed)
                {
                    Console.WriteLine("start set out test");

                    DigitalOutSet set = new DigitalOutSet
                    {
                        Id = new PinId
                        {
                            Interface = Interfaces.InterfaceGpioWiringpi,
                            Pin = 5
                        },
                        State = true
                    };
                    _tics = HPT.GetTime;
                    _client.Tell(set);

                    Become(TestState);
                }
                else
                {
                    Console.WriteLine("gpio digital out init error");
                }
            });
        }

        private void TestState()
        {
            Receive<DigitalOutState>(swm =>
            {
                var curTime = HPT.GetTime;
                var dur = curTime - _tics;
                _stats.Add(HPT.ToMillisesonds(dur));
                _counter++;

                if (_counter < _runs)
                {
                    Console.Write($"\r{_counter} from {_runs}");
                    DigitalOutSet cwm = new DigitalOutSet
                    {
                        Id = new PinId
                        {
                            Interface = Interfaces.InterfaceGpioWiringpi,
                            Pin = 5
                        },
                        State = !swm.State
                    };

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
                    //double min = Statistics.Minimum(_stats);
                    //double max = Statistics.Maximum(_stats);

                    Console.WriteLine($"\rmean: {mean:0.00} std: {sdv:0.00} median: {median:0.00} p25: {p25:0.00} p75: {p75:0.00} p2.5: {p2_5:0.00} p97.5: {p97_5:0.00}");
                    Context.Stop(Self);
                }
            });
        }

        public static Props Props(IActorRef client, int runs) => Akka.Actor.Props.Create(() => new SetDigitalOutTest(client, runs));
    }
}
