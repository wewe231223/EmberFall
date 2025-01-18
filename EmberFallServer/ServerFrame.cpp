#include "ServerFrame.h"

ServerFrame::ServerFrame() 
    : mServerCore{ std::make_unique<ServerCore>() } { 
    mServerCore->Start(SERVER_PORT);
}

ServerFrame::~ServerFrame() { 
    mServerCore->End();
}

void ServerFrame::GameLoop() {
    while (true) {
        


    }
}
