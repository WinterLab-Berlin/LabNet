using Akka.Actor;
using Akka.IO;
using Google.Protobuf;
using LabNetProt.Client;
using LabNetProt.Server;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace PerfTest
{
    class TcpDataClientActor : ReceiveActor
    {
        #region in messages
        public class SetMessageReceiver
        {
            public SetMessageReceiver(IActorRef actorRef)
            {
                ActorRef = actorRef;
            }

            public IActorRef ActorRef { get; }
        }
        #endregion

        #region out messages
        public class ConnectedMsg
        {
        }

        public class ConnectionFailed { }
        #endregion

        #region Variables
        IActorRef _connection;
        IActorRef _tester;
        string _host;
        int _port;

        List<byte> _buffer;
        #endregion

        public TcpDataClientActor(string host, int port, IActorRef tester)
        {
            _host = host;
            _port = port;
            _tester = tester;
            _buffer = new List<byte>();

            var server = System.Net.Dns.GetHostAddresses(host);
            IPEndPoint endpoint = new IPEndPoint(server[0], port);

            Context.System.Tcp().Tell(new Tcp.Connect(endpoint, null, null, TimeSpan.FromMilliseconds(1), false));
            Become(ConnectiongState);
        }

        private void ConnectiongState()
        {
            Receive<Tcp.Connected>(x =>
            {
                _connection = Sender;
                _tester.Tell(new ConnectedMsg());
                Sender.Tell(new Tcp.Register(Self));

                Become(ConnectedState);
            });

            Receive<Tcp.CommandFailed>(x =>
            {
                _tester.Tell(new ConnectionFailed());
                Become(DisconnectedState);
            });
        }

        private void ConnectedState()
        {
            Receive<Tcp.PeerClosed>(x =>
            {
                _tester.Tell(new ConnectionFailed());
                Become(DisconnectedState);
            });

            Receive<Tcp.ErrorClosed>(x =>
            {
                _tester.Tell(new ConnectionFailed());
                Become(DisconnectedState);
            });

            Receive<SetMessageReceiver>(mes =>
            {
                _tester = mes.ActorRef;
            });

            Receive<Tcp.Received>(x =>
            {
                _buffer.AddRange(x.Data.ToArray());

                bool parse = true;
                while (parse)
                {
                    parse = false;
                    int dataStart = GetMesTypeAndLength(out int type, out int length);
                    if (dataStart > -1)
                    {
                        if (_buffer.Count >= (dataStart + length))
                        {
                            byte[] data = _buffer.GetRange(dataStart, length).ToArray();

                            switch ((LabNetProt.Server.ServerMessageType)type)
                            {
                                case ServerMessageType.None:
                                    break;
                                case ServerMessageType.DigitalOutState:
                                {
                                    var state = LabNetProt.Server.DigitalOutState.Parser.ParseFrom(data);
                                    _tester.Tell(state);
                                }
                                break;
                                case ServerMessageType.DigitalInState:
                                {
                                    var state = LabNetProt.Server.DigitalInState.Parser.ParseFrom(data);
                                    _tester.Tell(state);
                                }
                                break;
                                case ServerMessageType.NewByteData:
                                {
                                    var byteData = LabNetProt.Server.NewByteData.Parser.ParseFrom(data);
                                    _tester.Tell(byteData);
                                }
                                break;
                                case ServerMessageType.DataWriteComplete:
                                {
                                    var dataWrite = LabNetProt.Server.DataWriteComplete.Parser.ParseFrom(data);
                                    _tester.Tell(dataWrite);
                                }
                                break;
                                case ServerMessageType.InterfaceInitResult:
                                {
                                    var init = LabNetProt.Server.InterfaceInitResult.Parser.ParseFrom(data);
                                    _tester.Tell(init);
                                }
                                break;
                                case ServerMessageType.DigitalInInitResult:
                                {
                                    var init = LabNetProt.Server.DigitalInInitResult.Parser.ParseFrom(data);
                                    _tester.Tell(init);
                                }
                                break;
                                case ServerMessageType.DigitalOutInitResult:
                                {
                                    var init = LabNetProt.Server.DigitalOutInitResult.Parser.ParseFrom(data);
                                    _tester.Tell(init);
                                }
                                break;
                                case ServerMessageType.OnlyOneConnectionAllowed:
                                {
                                    var onlyOne = LabNetProt.Server.OnlyOneConnectionAllowed.Parser.ParseFrom(data);
                                    _tester.Tell(onlyOne);
                                }
                                break;
                                case ServerMessageType.LabnetResetReply:
                                {
                                    var reset = LabNetProt.Server.LabNetResetReply.Parser.ParseFrom(data);
                                    _tester.Tell(reset);
                                }
                                break;
                                case ServerMessageType.LabnetIdReply:
                                {
                                    var id = LabNetProt.Server.LabNetIdReply.Parser.ParseFrom(data);
                                    _tester.Tell(id);
                                }
                                break;
                                case ServerMessageType.InterfaceLost:
                                {
                                    var lost = LabNetProt.Server.InterfaceLost.Parser.ParseFrom(data);
                                    _tester.Tell(lost);
                                }
                                break;
                                case ServerMessageType.InterfaceReconnected:
                                {
                                    var recon = LabNetProt.Server.InterfaceReconnected.Parser.ParseFrom(data);
                                    _tester.Tell(recon);
                                }
                                break;
                                case ServerMessageType.DigitalOutLoopStartResult:
                                {
                                    var loop = LabNetProt.Server.DigitalOutLoopStartResult.Parser.ParseFrom(data);
                                    _tester.Tell(loop);
                                }
                                break;
                                case ServerMessageType.DigitalOutLoopStopped:
                                {
                                    var loop = LabNetProt.Server.DigitalOutLoopStopped.Parser.ParseFrom(data);
                                    _tester.Tell(loop);
                                }
                                break;
                                default:
                                    break;
                            }

                            _buffer.RemoveRange(0, dataStart + length);
                            parse = true;
                        }
                    }
                }
            });

            Receive<IoBoardInit>(msg =>
            {
                PackMessageAndSend(msg, ClientMessageType.IoBoardInit);
            });

            Receive<IoBoardInitDigitalOut>(msg =>
            {
                PackMessageAndSend(msg, ClientMessageType.IoBoardInitDigitalOut);
            });

            Receive<IoBoardInitDigitalIn>(msg =>
            {
                PackMessageAndSend(msg, ClientMessageType.IoBoardInitDigitalIn);
            });

            Receive<DigitalOutSet>(msg =>
            {
                PackMessageAndSend(msg, ClientMessageType.DigitalOutSet);
            });

            Receive<LabNetIdRequest>(msg =>
            {
                PackMessageAndSend(msg, ClientMessageType.LabnetIdRequest);
            });

            Receive<LabNetResetRequest>(msg =>
            {
                PackMessageAndSend(msg, ClientMessageType.LabnetResetRequest);
            });
        }

        private void DisconnectedState()
        {

        }

        private void PackMessageAndSend(Google.Protobuf.IMessage msg, ClientMessageType msgType)
        {
            MemoryStream ms = new MemoryStream();

            Google.Protobuf.CodedOutputStream outputStream = new CodedOutputStream(ms);
            outputStream.WriteEnum((int)msgType);
            outputStream.WriteMessage(msg);
            outputStream.Flush();

            _connection.Tell(Tcp.Write.Create(Akka.IO.ByteString.FromBytes(ms.ToArray())));
        }

        private int GetMesTypeAndLength(out int type, out int length)
        {
            type = 0;
            length = 0;

            bool typePresent = false;
            int shift = 0;
            for (int i = 0; i < _buffer.Count; i++)
            {
                if ((_buffer[i] & 0x80) == 0)
                {
                    if (typePresent)
                    {
                        length = (length | (_buffer[i] << shift));
                        return i + 1;
                    }
                    else
                    {
                        typePresent = true;
                        type = (type | (_buffer[i] << shift));
                        shift = 0;
                    }
                }
                else
                {
                    if (typePresent)
                    {
                        length = (length | ((_buffer[i] ^ 0x80) << shift));
                        shift += 7;
                    }
                    else
                    {
                        type = (type | ((_buffer[i] ^ 0x80) << shift));
                        shift += 7;
                    }
                }
            }

            return -1;
        }

        public static Props Props(string host, int port, IActorRef tester) => Akka.Actor.Props.Create(() => new TcpDataClientActor(host, port, tester));
    }
}
