﻿using Akka.Actor;
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
    class SetAndReadTest : ReceiveActor
    {
        IActorRef _client;
        bool _isOn = false;
        int _runs;
        int _counter;
        long _tics;
        List<double> _stats;

        public SetAndReadTest(IActorRef client, int runs)
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
            IoBoardInit cwm = new IoBoardInit();
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
            IoBoardInitDigitalOut cwm = new IoBoardInitDigitalOut { Pin = 1, IsInverted = false };
            _client.Tell(cwm);

            Receive<DigitalOutInitResult>(swm =>
            {
                if (swm.IsSucceed)
                {
                    Become(InitDigitalInState);
                }
                else
                {
                    Console.WriteLine("gpio digital out init error");
                }
            });
        }

        private void InitDigitalInState()
        {
            IoBoardInitDigitalIn cwm = new IoBoardInitDigitalIn { Pin = 1, IsInverted = false, ResistorState = IoBoardInitDigitalIn.Types.Resistor.Off };
            _client.Tell(cwm);

            Receive<DigitalInState>(swm =>
            {
                _isOn = !swm.State;

                Console.WriteLine("start gpio test");

                DigitalOutSet set = new DigitalOutSet
                {
                    Id = new PinId
                    {
                        Interface = Interfaces.InterfaceIoBoard,
                        Pin = 1
                    },
                    State = _isOn
                };
                _tics = HPT.GetTime;
                _client.Tell(set);

                Become(TestState);
            });
        }

        private void TestState()
        {
            Receive<DigitalInState>(swm =>
            {
                if (swm.State == _isOn)
                {
                    var curTime = HPT.GetTime;
                    var dur = curTime - _tics;
                    _stats.Add(HPT.ToMillisesonds(dur));
                    _counter++;

                    if (_counter < _runs)
                    {
                        Console.Write($"\r{_counter} from {_runs}");
                        _isOn = !_isOn;
                        DigitalOutSet cwm = new DigitalOutSet
                        {
                            Id = new PinId
                            {
                                Interface = Interfaces.InterfaceIoBoard,
                                Pin = 1
                            },
                            State = _isOn
                        };

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
                }
            });
        }

        public static Props Props(IActorRef client, int runs) => Akka.Actor.Props.Create(() => new SetAndReadTest(client, runs));
    }
}
