#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EditorDevice.h
// 2024.12.31 김승범		- Editor 에 접근하기 위한 메인 디바이스를 정의하고, 별도의 창을 생성할 준비를 하였음. 
// 2025.01.02 김승범		- 별도의 쓰레드를 활용해서 하나의 창을 만드는 것에 성공하였음. 
// 2025.01.03 김승범		- Editor 의 윈도우가 Main 윈도우를 따라가도록 만드는 과정에서, 메인 윈도우를 받는 별도의 
//						  초기화 과정을 추가하였음. 이를 통해 Editor 의 업데이트 흐름에서 Main 윈도우의 위치를 
//						  따라가는 기능을 구현하였음. 하지만 Main 윈도우가 일관됨이 보장되지 않는다는 문제가 존재함. 	
//						  이를 SetForegroundWindow 함수를 사용하여, 메인 창이 닫혔을 때에, 한번만 창이 우선권을 받도록 
//						  구현하였음. 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef win32_lean_and_mean
#define	win32_lean_and_mean
#endif
#include <Windows.h>
#include <thread>
#include <mutex>
#include "../Base/Interface.h"

class EditorDevice {
	static constexpr bool AUTOMATIC_CLOSED				{ false };
public:
	EditorDevice();
	~EditorDevice();

	EditorDevice(const EditorDevice& other)				= delete;
	EditorDevice& operator=(const EditorDevice& other)	= delete;

	EditorDevice(EditorDevice&& other)					= delete;
	EditorDevice& operator=(EditorDevice&& other)		= delete;
public:
	operator bool() const;
public:
	void Initialize(HWND mainWindowHandle);

	void WaitForTerminate();
private:
	void EditorWindowWorker();

	void Update();
	void Render();
private:
	HINSTANCE			mApplicationHandle{ ::GetModuleHandle(nullptr) };

	HWND				mMainWindow{ nullptr };
	HWND				mEditorDeviceWindow{ nullptr };

	DWORD				mMainThreadID{ ::GetCurrentThreadId() };
	DWORD				mEditorThreadID{ 0 };
	
	RECT				mMainWindowRect{};

	std::thread			mEditorDeviceThread{};

	std::mutex 			mEditorDeviceMutex{};

	struct DirectXImpl;
	std::unique_ptr<DirectXImpl> mDirectXImpl{ nullptr };
};

extern EditorDevice gDevice;

														