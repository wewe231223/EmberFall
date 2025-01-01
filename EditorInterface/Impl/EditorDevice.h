#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EditorDevice.h
// 2024.12.31 김승범   - Editor 에 접근하기 위한 메인 디바이스를 정의하고, 별도의 창을 생성할 준비를 하였음. 
// 2025.01.02 김승범	  - 별도의 쓰레드를 활용해서 하나의 창을 만드는 것에 성공하였음. 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../Base/Interface.h"
#include <thread>

class EditorDevice {
	static constexpr bool AUTOMATIC_CLOSED{ false };
public:
	EditorDevice();
	~EditorDevice();

	EditorDevice(const EditorDevice& other) = delete;
	EditorDevice& operator=(const EditorDevice& other) = delete;

	EditorDevice(EditorDevice&& other) = delete;
	EditorDevice& operator=(EditorDevice&& other) = delete;
public:
	void Test();
private:
	void EditorWindowWorker();
private:
	HINSTANCE mApplicationHandle{ ::GetModuleHandle(nullptr) };
	HWND mEditorDeviceWindow{ nullptr };

	DWORD mMainThreadID{ ::GetCurrentThreadId() };
	DWORD mEditorThreadID{ 0 };
	
	std::thread mEditorDeviceThread{};
};

extern EditorDevice gDevice;