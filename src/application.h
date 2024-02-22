#pragma once

#include "service.h"
#include "sysMsg.h"
#include "log.h"
#include "spinLock.h"
#include "singleton.h"
#include <map>

namespace Oimo {
    class Application {
    public:
        using ServiceID = uint32_t;
        using ServiceMap = std::map<ServiceID, Service::sPtr>;
        ~Application();
        void init();
        int run();
        void stop();
        template <typename T>
        ServiceID newService(const std::string& name, std::string args = "") {
            static_assert(std::is_base_of<Service, T>::value, "T must be derived from Service");
            ServiceContext::sPtr context = ServiceContext::createContext(name);
            std::shared_ptr<T> service = std::make_shared<T>();
            service->setContext(context);
            registerService(context->serviceID(), service);
            service->registerFunc((Packle::MsgID)SystemMsgID::INIT, std::bind(&T::init, service, std::placeholders::_1));
            Packle::sPtr packle = std::make_shared<Packle>((Packle::MsgID)SystemMsgID::INIT);
            packle->userData = args;
            ServiceContext::send(context, packle);
            LOG_INFO << "Service: " << name << " created";
            return service->id();
        }
        Service::sPtr getService(ServiceID id) const;
        ServiceMap serviceMap() const;
        void removeService(ServiceID id);
    private:
        void registerService(ServiceID id, Service::sPtr service);
        ServiceMap services;
        mutable SpinLock lock;
    };

    using APP = Singleton<Application>;
}