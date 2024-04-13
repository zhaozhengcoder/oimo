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
#include "src/rpcServer.h"
#include "serviceContextMgr.h"

namespace Oimo {
namespace Net {
    RpcServer::RpcServer() {}
    RpcServer::~RpcServer() {}

    void RpcServer::init(Oimo::Service* serv) {
        TcpServer::init(serv);
        service()->registerFunc((Packle::MsgID)SystemMsgID::DATA,
            std::bind(&RpcServer::handleRead, this, std::placeholders::_1));
    }

    int RpcServer::rpcCall(Net::Connection::sPtr conn, Packle::MsgID msg_id
            , const std::string& dest_service, NetProto::Body& body) {
        NetProto::Rpc rpc;
        auto request = rpc.mutable_request();
        request->mutable_head()->set_dest_name(dest_service);
        request->mutable_head()->set_msg_id(msg_id);
        request->mutable_body()->CopyFrom(body);

        auto context = service()->context()->currentContext();
        auto cb = rpc.mutable_request()->mutable_head()->mutable_callback();
        cb->set_packle_session_id(context->getSession());
        cb->set_packle_source(context->serviceID());

        auto pb_data = pbEncode(rpc);
        int ret = conn->send(pb_data.c_str(), pb_data.size());

        auto self = Oimo::ServiceContext::currentContext();
        auto cor = Oimo::Coroutine::currentCoroutine();
        auto sid = cb->packle_session_id();
        cor->setSid(sid);
        self->suspend(cor);
        Oimo::Coroutine::yieldToSuspend();
        return ret;
    }

    int RpcServer::decode(Connection::sPtr conn, google::protobuf::Message* message) {
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

    void RpcServer::handleRead(Packle::sPtr packle) {
        int fd = packle->fd();
        char *buf = packle->buf();
        size_t len = packle->size();
        auto conn = findConn(fd);
        if (conn == nullptr) {
            LOG_ERROR << "TcpServer::handleRead fd : " << fd;
            return;
        }
        conn->append(buf, len);
    
        NetProto::Rpc rpc_message;
        int ret = decode(conn, &rpc_message);
        if (ret != 0) {
            LOG_ERROR << "decode error ret : " << ret;
            return;
        }
        Packle::sPtr cb_packle = std::make_shared<Packle>(
            (Packle::MsgID)CommonMsgType::RpcCall
        );
        const std::string data = "helloworld";
        auto rpc_cb = rpc_message.response().head().callback();

        cb_packle->setSource(rpc_cb.packle_source());
        cb_packle->setSessionID(rpc_cb.packle_session_id());
        cb_packle->userData = rpc_message;
        ServiceContext::sPtr context = ServiceContextMgr::getContext(service()->context()->name());
        context->setReturnPackleAndSetSource(cb_packle, rpc_cb.packle_source());
    }
}
}