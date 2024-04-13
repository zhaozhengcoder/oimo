#include <vector>
#include <src/log.h>
#include <src/application.h>
#include <src/singleton.h>
#include <src/logThread.h>
#include <src/coroutine.h>
#include <src/tcpServer.h>
#include <src/rpcServer.h>
#include <src/utils.h>
#include "src/protobuf/rpc.pb.h"

using namespace Oimo;

const int port = 9527;
const std::string remote_dest_service = "pong";

class PingService: public Service {
public:
    void init(Packle::sPtr packle) {
        m_serv.init(this);
        m_serv.createFd("0.0.0.0", port);
        m_serv.start(
            std::bind(&PingService::onConnect, this, std::placeholders::_1)
        );
        LOG_INFO << "Master listen port "<< port << ", init done";
        
        auto id = addTimer(0, 5000, [this](){
            this->pingRequest();
        });
        LOG_INFO << "add timer done, timer id : "<< id;
    }

private:
    void onConnect(Net::Connection::sPtr conn) {
        conn->start();
        auto addr = conn->addr();
        LOG_INFO << "new connection from " << addr.ipAsString() << ":" << addr.portAsString();
        m_login.push_back(getCurrentTime() + "-" + addr.ipAsString()+ "-" + addr.portAsString());
    }

    void pingRequest() {
        auto conns = m_serv.getConns();
        assert(conns != nullptr);
        static int seq = 0;
        for (const auto & pair : *conns) {
            NetProto::Body body;
            std::string data = "hello_" + std::to_string(seq++);
            body.mutable_ping_biz_data()->set_req_ping(data);
            
            auto msg_id = (Packle::MsgID)CommonMsgType::RpcCall;
            m_serv.rpcCall(pair.second, msg_id, remote_dest_service, body);

            auto resp = responsePackle();
            auto rsp_message = std::any_cast<NetProto::Rpc>(resp->userData);
            if (resp == nullptr) {assert(0); return;}
            LOG_DEBUG << "recv rsp : " << rsp_message.ShortDebugString();
        } 
    }

private:
    Net::RpcServer m_serv;
    std::vector<std::string> m_login;
};

int main() {
    Logger::setLogLevel(LogLevel::DEBUG);
    auto& app = APP::instance();
    app.init();
    app.newService<PingService>("master");
    return app.run();
}