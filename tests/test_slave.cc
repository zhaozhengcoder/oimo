#include <src/log.h>
#include <src/application.h>
#include <src/singleton.h>
#include <src/logThread.h>
#include <src/coroutine.h>
#include <src/tcpServer.h>
#include <src/rpcClientStub.h>
#include <src/socket.h>
#include <src/utils.h>
#include "src/protobuf/rpc.pb.h"

using namespace Oimo;

class PongService : public Service {
public:
    void init(Packle::sPtr packle) {
        registerFunc((Packle::MsgID)CommonMsgType::RpcCall,
            std::bind(&PongService::pong, this, std::placeholders::_1));
    }
private:
    void pong(Packle::sPtr packle) {
        auto rpc_message = std::any_cast<NetProto::Rpc>(packle->userData);
        LOG_DEBUG << rpc_message.ShortDebugString();

        NetProto::Rpc rpc;
        auto rsp = rpc.mutable_response();
        rsp->mutable_head()->CopyFrom(rpc_message.request().head());
        rsp->mutable_body()->mutable_pong_biz_data()->set_rsp_pong("helloworld");
        packle->userData = rpc;
        setReturnPackle(packle);
    }
};

int main() {
    Logger::setLogLevel(LogLevel::DEBUG);
    auto& app = APP::instance();
    app.init();
    app.newService<RpcClientStub>("rpcStub");
    app.newService<PongService>("pong");
    return app.run();
}