#pragma once

class ServerCore {
public:
    ServerCore();
    ~ServerCore();

public:
    void Start(const std::string& ip, const unsigned short port, size_t workerThreadNum=HARDWARE_CONCURRENCY);
    void End();

private:
    std::shared_ptr<class Listener> mListener;
    //std::unique_ptr<class PacketHandler> mPacketHandler{ nullptr };

    std::vector<std::thread> mWorkerThreads{ };
};