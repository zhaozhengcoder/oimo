# 练手项目（更新到service模块）

高性能游戏服务器，也可作为web服务器，主要技术要点：

1. **日志模块**：参考muoduo库，实现异步可回滚日志系统，日志前后端分离，临界区小且线程安全；
2. **配置模块**：采用yaml格式进行配置，约定优于配置；
3. **线程模块**；
4. **协程模块**：参考腾讯libco库，通过嵌入汇编切换寄存器上下文；
5. **服务模块**：Actor实例，可通过消息队列进行消息收发和服务间同步rpc调用（由于采用协程实现，所以底层是异步的不会阻塞线程，只是对业务层透明）；