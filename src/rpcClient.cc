#include <cassert>
#include <sys/socket.h>
#include <memory>
#include <google/protobuf/message.h>
#include "utils.h"
#include "address.h"
#include "service.h"
#include "log.h"
#include "singleton.h"
#include "sysMsg.h"
#include "coroutine.h"
#include "ctrlMsg.h"
#include "socketContext.h"
#include "socketServer.h"
#include "tcpServer.h"
#include "src/protobuf/rpc.pb.h"
#include "src/rpcClient.h"

namespace Oimo {
namespace Net {
    RpcClient::RpcClient() {
        m_connected_fd = -1;
    }

    RpcClient::~RpcClient() {}

    void RpcClient::init(Oimo::Service* serv) {
        TcpServer::init(serv);
        service()->registerFunc((Packle::MsgID)SystemMsgID::DATA,
            std::bind(&RpcClient::handleRead, this, std::placeholders::_1));
    }

    int RpcClient::connectToSvr(const std::string& ip, uint16_t port) {
        Address address(ip, port);
        if (!m_connected_socket.connect(&address)) {
            LOG_ERROR << "Failed to connect socket";
            return -1;
        }

        int fd = m_connected_socket.fd();
        m_connected_fd = fd;
        Connection::sPtr conn = std::make_shared<Connection>(fd, address.ipForNet(), port, this);
        assert(!hasFd(fd));
        addConn(fd, conn);
        SocketContext* ctx = GSocketServer::instance().getSocketContext(fd);
        assert(!ctx->isValid());

        ctx->reset(fd, service()->id());
        auto& sock = ctx->sock();
        sock.setNonBlocking(true);
        sock.setNoDelay(true);
        ctx->setSockType(SocketType::ACCEPT);
        ctx->enableRead();
        return 0;
    }

    int RpcClient::sendToSvr(const std::string &data) {
        auto conn = findConn(m_connected_fd);
        if (conn == nullptr) {
            LOG_ERROR << "TcpServer::handleRead: unknown fd";
            return -1;
        }
        conn->send(data.c_str(), data.size());
        return 0;
    }

    int RpcClient::sendToSvr(google::protobuf::Message& message) {
        auto data = pbEncode(message);
        sendToSvr(data);
        return 0;
    }

    int RpcClient::decode(Connection::sPtr conn, google::protobuf::Message* message) {
        const int MAX_DECODE_LEN = 1024 * 10;
        auto data = std::shared_ptr<char>(new char[MAX_DECODE_LEN]);

        uint32_t header_len = 4;
        auto recv_len = conn->recvN(data.get(), header_len);
        assert(recv_len == header_len);

        uint32_t msg_len = ntohl(*(uint32_t*)data.get());
        recv_len = conn->recvN(data.get(), msg_len);
    
        bool ret = message->ParseFromArray(data.get(), msg_len);
        if (!ret) {return -1;}
        return 0;
    }

    void RpcClient::handleRead(Packle::sPtr packle) {
        int fd = packle->fd();
        char *buf = packle->buf();
        size_t len = packle->size();
        auto conn = findConn(m_connected_fd);
        if (conn == nullptr) {
            LOG_ERROR << "TcpServer::handleRead: connected : " 
                      << m_connected_fd << ", fd : " << fd;
            return;
        }
        conn->append(buf, len);
        // todo : 这里可以message指针 然后move一下 应该性能更好 
        NetProto::Rpc rpc_message;
        decode(conn, &rpc_message);
        Packle::sPtr cb_packle = std::make_shared<Packle>(
            (Packle::MsgID)CommonMsgType::RpcCall
        );
        cb_packle->userData = rpc_message;
        service()->call(service()->context()->name(), cb_packle);
    }
}
}