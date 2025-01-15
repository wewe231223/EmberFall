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
    std::vector<std::thread> mWorkerThreads{ };
};