#pragma once

#include <src/service.h>
#include <src/rpcClient.h>

namespace Oimo {

class RpcClientStub: public Service {
public:
    void init(Packle::sPtr packle);
    void rpcDispatch(Packle::sPtr packle);
private:
    Net::RpcClient m_serv;
};

}