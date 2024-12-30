#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EditorDevice.h
// 2024.12.31 김승범   - Editor 에 접근하기 위한 메인 디바이스를 정의하고, 별도의 창을 생성할 준비를 하였음. 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../Base/Interface.h"
#include <thread>

class EditorDevice {
public:
	EditorDevice();
	~EditorDevice();

public:
	void Join(); 
private:
	bool InitWindow();
private:
	HINSTANCE mApplicationHandle{ ::GetModuleHandle(nullptr) };
	HWND mEditorDeviceWindow{ nullptr };

	std::thread mEditorDeviceThread{};
};

extern EditorDevice gDevice;