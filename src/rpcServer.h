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

class RpcServer: public TcpServer {
public:
    RpcServer();
    ~RpcServer();
    void init(Oimo::Service* serv);
    int  rpcCall(Net::Connection::sPtr conn, Packle::MsgID msg_id
            , const std::string& dest_service, NetProto::Body& body);
    int  decode(Connection::sPtr conn, google::protobuf::Message* message);
    void handleRead(Packle::sPtr packle);

private:
    Net::Socket m_connected_socket;
};


}  // Net
}  // Oimo