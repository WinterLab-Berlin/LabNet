#include "server.h"
#include <signal.h>
#include <memory>
#include <utility>
#include <boost/lexical_cast.hpp>

namespace LabNet::network
{

    Server::Server(log::Logger logger, ConnectionManager& connection_manager, ushort port)
        : logger_(logger)
        , io_context_(1)
        , signals_(io_context_)
        , acceptor_(io_context_)
        , connection_manager_(connection_manager)
    {
        // Register to handle the signals that indicate when the server should exit.
        signals_.add(SIGINT);
        signals_.add(SIGTERM);
#if defined(SIGQUIT)
        signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)

        DoAwaitStop();

        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);

        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen();

        logger_->WriteInfoEntry(std::string("server is now running on: ") + boost::lexical_cast<std::string>(endpoint));

        DoAccept();
    }

    void Server::Run()
    {
        io_context_.run();
    }

    void Server::DoAccept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
                // Check whether the server was stopped by a signal before this
                // completion handler had a chance to run.
                if (!acceptor_.is_open())
                {
                    return;
                }

                if (!ec)
                {
                    logger_->WriteInfoEntry(std::string("new connection from ") + boost::lexical_cast<std::string>(socket.remote_endpoint()));

                    socket.set_option(boost::asio::socket_base::keep_alive(true));
                    socket.set_option(boost::asio::socket_base::linger(true, 2));

                    boost::asio::ip::tcp::no_delay option(true);
                    socket.set_option(option);

                    connection_manager_.Start(std::make_shared<Connection>(
                        logger_, std::move(socket), connection_manager_, connection_manager_.GetServerInBox()));
                }

                DoAccept();
            });
    }

    void Server::DoAwaitStop()
    {
        signals_.async_wait(
            [this](boost::system::error_code /*ec*/, int /*signo*/) {
                acceptor_.close();
                connection_manager_.StopAll();

                logger_->WriteInfoEntry("stop server");
            });
    }
}