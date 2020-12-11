
#include "connection_manager.h"
#include "server_messages.h"

namespace LabNet::network
{
    ConnectionManager::ConnectionManager(log::Logger logger, so_5::mbox_t server_in_box)
        : logger_(logger)
        , server_in_box_(server_in_box)
    {
    }

    void ConnectionManager::Start(std::shared_ptr<Connection> c)
    {
        if (connection_ == nullptr)
        {
            logger_->WriteInfoEntry("accept new connection");

            connection_ = c;
            c->Start();

            so_5::send<LabNet::network::ClientConnected>(server_in_box_, c);
        }
        else
        {
            logger_->WriteInfoEntry("only one connection possible");

            connections_.insert(c);
            c->RefuseConection();
        }
    }

    void ConnectionManager::Stop(std::shared_ptr<Connection> c)
    {
        if (connection_ == c)
        {
            c->Stop();
            connection_ = nullptr;

            so_5::send<LabNet::network::ClientDisconnected>(server_in_box_);
        }
        else
        {
            connections_.erase(c);
        }
    }

    void ConnectionManager::StopAll()
    {
        if (connection_ != nullptr)
        {
            connection_->Stop();
            connection_ = nullptr;
        }

        for (auto c : connections_)
            c->Stop();
        connections_.clear();
    }
}