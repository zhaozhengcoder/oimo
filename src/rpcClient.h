#pragma once

#include <functional>
#include <string>
#include <map>
#include <functional>
#include <src/tcpServer.h>
#include <google/protobuf/message.h>
#include <src/socket.h>
#include <src/connection.h>
#include "src/protobuf/rpc.pb.h"

namespace Oimo {
class Service;
namespace Net {

class RpcClient: public TcpServer {
public:
    RpcClient();
    ~RpcClient();
    void init(Oimo::Service* serv);
    int connectToSvr(const std::string& ip, uint16_t port);
    int sendToSvr(const std::string &data);
    int sendToSvr(google::protobuf::Message& message);
    int decode(Connection::sPtr conn, google::protobuf::Message* message);
    void handleRead(Packle::sPtr packle);

private:
    int m_connected_fd {-1};
    Net::Socket m_connected_socket;
};


}  // Net
}  // Oimo