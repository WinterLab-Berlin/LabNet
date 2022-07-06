using Akka.Actor;
using LabNetProt;
using LabNetProt.Client;
using LabNetProt.Server;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PerfTest
{
    internal class ReadAndSetTest : ReceiveActor
    {
        IActorRef _client;
        List<uint> _inPins;
        List<uint> _outPins;
        Dictionary<uint, uint> _inOutMap;


        public ReadAndSetTest()
        {
            Become(WaitForConnection);
            
            _inPins = new List<uint>(new uint[] { 29, 28, 27, 26, 31, 11, 10, 6, 5, 4, 1, 16, 15, 8 });
            _outPins = new List<uint>(new uint[] { 25, 24, 23, 22, 21, 30, 14, 13, 12, 3, 2, 0, 7, 9 });
            _inOutMap = new Dictionary<uint, uint>();
            for (int i = 0; i < _inPins.Count; i++)
            {
                _inOutMap.Add(_inPins[i], _outPins[i]);
            }
        }

        private void WaitForConnection()
        {
            Receive<TcpDataClientActor.ConnectedMsg>(mes =>
            {
                _client = Sender;
                _client.Tell(new TcpDataClientActor.SetMessageReceiver(Self));

                Become(ResetState);
            });

            Receive<TcpDataClientActor.ConnectionFailed>(mes =>
            {

            });
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

        int _pinInitNbr = 0;
        private void InitDigitalOutState()
        {
            GpioWiringPiInitDigitalOut cwm = new GpioWiringPiInitDigitalOut { Pin = _outPins[_pinInitNbr], IsInverted = false };
            _pinInitNbr++;
            _client.Tell(cwm);

            Receive<DigitalOutInitResult>(swm =>
            {
                if (swm.IsSucceed)
                {
                    if(_pinInitNbr >= _inPins.Count)
                    {
                        Become(InitDigitalInState);
                    }
                    else
                    {
                        cwm = new GpioWiringPiInitDigitalOut { Pin = _outPins[_pinInitNbr], IsInverted = false };
                        _pinInitNbr++;
                        _client.Tell(cwm);
                    }
                }
                else
                {
                    Console.WriteLine("gpio digital out init error");
                }
            });
        }

        private void InitDigitalInState()
        {
            _pinInitNbr = 0;
            GpioWiringPiInitDigitalIn cwm = new GpioWiringPiInitDigitalIn { Pin = _inPins[_pinInitNbr], IsInverted = false, ResistorState = GpioWiringPiInitDigitalIn.Types.Resistor.Off };
            _pinInitNbr++;
            _client.Tell(cwm);

            Receive<DigitalInInitResult>(swm =>
            {
                if (swm.IsSucceed)
                {
                    if (_pinInitNbr >= _inPins.Count)
                    {
                        Console.WriteLine("start gpio test");

                        Become(TestState);
                    }
                    else
                    {
                        cwm = new GpioWiringPiInitDigitalIn { Pin = _inPins[_pinInitNbr], IsInverted = false, ResistorState = GpioWiringPiInitDigitalIn.Types.Resistor.Off };
                        _pinInitNbr++;
                        _client.Tell(cwm);
                    }
                }
                else
                {
                    Console.WriteLine("gpio digital in init error");
                }
            });
        }

        private void TestState()
        {
            Receive<DigitalInState>(swm =>
            {
                uint pin = swm.Pin.Pin;
                if (_inOutMap.ContainsKey(pin))
                {
                    pin = _inOutMap[pin];

                    if (swm.State)
                    {


                        DigitalOutSet cwm = new DigitalOutSet
                        {
                            Id = new PinId
                            {
                                Interface = Interfaces.InterfaceGpioWiringpi,
                                Pin = pin
                            },
                            State = true
                        };
                        _client.Tell(cwm);
                    }
                    else
                    {
                        DigitalOutSet cwm = new DigitalOutSet
                        {
                            Id = new PinId
                            {
                                Interface = Interfaces.InterfaceGpioWiringpi,
                                Pin = pin
                            },
                            State = false
                        };
                        _client.Tell(cwm);
                    }
                }
            });
        }

        public static Props Props() => Akka.Actor.Props.Create(() => new ReadAndSetTest());
    }
}
