#include <src/log.h>
#include <src/logThread.h>
#include <src/coroutine.h>
#include <src/tcpServer.h>
#include <src/socket.h>
#include <src/utils.h>
#include <src/rpcClientStub.h>
#include <src/sysMsg.h>
#include "src/protobuf/rpc.pb.h"

using namespace Oimo;

static const int port = 9527;
static const std::string ip = "0.0.0.0";

void RpcClientStub::init(Packle::sPtr packle) {
    registerFunc((Packle::MsgID)CommonMsgType::RpcCall,
        std::bind(&RpcClientStub::rpcDispatch, this, std::placeholders::_1));
    m_serv.init(this);
    m_serv.connectToSvr(ip, port);
    LOG_INFO << "rpc client connect to " << ip << ":" << port;
}

void RpcClientStub::rpcDispatch(Packle::sPtr packle) {
    auto message = std::any_cast<NetProto::Rpc>(packle->userData);
    packle->setType((Packle::MsgID)message.request().head().msg_id());
    auto dst_service = message.request().head().dest_name();
    call(dst_service, packle);

    auto resp = responsePackle();
    auto response = std::any_cast<NetProto::Rpc>(resp->userData);
    int ret = m_serv.sendToSvr(response);
    if (ret != 0) {
        LOG_ERROR << "ret : " << ret;
    }
    return;
}