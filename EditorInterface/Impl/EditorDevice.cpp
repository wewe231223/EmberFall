#include "pch.h"
#include "EditorDevice.h"
#include "../Config/Config.h"
#include "EditorRenderer.h"

// Procedure 
LRESULT CALLBACK EditorDeviceProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

// DirectXImpl
struct EditorDevice::DirectXImpl {
    EditorRenderer mRenderer{};
};


EditorDevice::EditorDevice() {
}

EditorDevice::~EditorDevice() {

}

EditorDevice::operator bool() const {
    return mEditorDeviceWindow != nullptr;
}

void EditorDevice::Initialize(HWND mainWindowHandle) {
    mMainWindow = mainWindowHandle;
    mEditorDeviceThread = std::thread{ [this]() {EditorWindowWorker(); } };
}

void EditorDevice::WaitForTerminate() {
    if (mEditorDeviceThread.joinable()) {
        if (AUTOMATIC_CLOSED) {
            PostThreadMessage(mEditorThreadID, WM_QUIT, 0, 0);
        }
        mEditorDeviceThread.join();
    }

    mDirectXImpl.release();
}

// 여기서부터는 Device Thread 에서 작동하는 함수들이다. 
void EditorDevice::EditorWindowWorker() {
    mEditorThreadID = ::GetCurrentThreadId();

    // 창 클래스 설정
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = EditorDeviceProc;
    wc.hInstance = mApplicationHandle;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"EditorInterface";

    // 창 클래스 등록
    if (not RegisterClassEx(&wc))
    {
        MessageBox(nullptr, L"Editor Window 의 초기화에 실패했습니다.", L"Error", MB_ICONERROR);
        return;
    }

    // 창 스타일: 테두리 없음, 타이틀 바 없음
    DWORD style = WS_EX_OVERLAPPEDWINDOW; // WS_POPUP은 테두리와 타이틀 바가 없는 창을 만듭니다.
    DWORD exStyle = WS_EX_APPWINDOW; // 추가 스타일 설정 (필요에 따라 변경 가능)

	mEditorDeviceWindow = CreateWindowEx(
        exStyle,                                // 확장 스타일
        L"EditorInterface",                     // 윈도우 클래스 이름
        L"EditorInterface",                     // 윈도우 타이틀 (보이지 않음)
        WS_OVERLAPPEDWINDOW,                    // 윈도우 스타일
        CW_USEDEFAULT, CW_USEDEFAULT,           // 위치 
        Config::EDITOR_WINDOW_WIDTH<>, Config::EDITOR_WINDOW_HEIGHT<>, // 크기
        nullptr,                                // 부모 윈도우
        nullptr,                                // 메뉴
        mApplicationHandle,                     // 인스턴스 핸들
        nullptr                                 // 추가 매개변수
    );
   
	if (nullptr == mEditorDeviceWindow)
	{
        MessageBox(nullptr, L"Editor Window 의 초기화에 실패했습니다.", L"Error", MB_ICONERROR);
        return;
	}

	::ShowWindow(mEditorDeviceWindow, SW_SHOWDEFAULT);
	::UpdateWindow(mEditorDeviceWindow);

    mDirectXImpl = std::make_unique<DirectXImpl>();
    mDirectXImpl->mRenderer.Initialize(mEditorDeviceWindow);



    MSG msg{};

	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
        // 게임 루프... 
        EditorDevice::Update();
        EditorDevice::Render();
	}

    // 이 함수가 종료되면, 메인 창도 꺼지도록 메세지를 보냄 
	::PostThreadMessage(mMainThreadID, WM_QUIT, 0, 0);

    mDirectXImpl.reset();
}

void EditorDevice::Update() {
	static bool flag = true;
    if (flag) {
	    std::unique_lock lock{ mEditorDeviceMutex };
        auto res = ::GetWindowRect(mMainWindow, std::addressof(mMainWindowRect));
        if (res) {
            ::SetWindowPos(mEditorDeviceWindow, NULL, mMainWindowRect.right, mMainWindowRect.top, 0, 0, SWP_NOSIZE);
            ::SetWindowPos(mMainWindow, mEditorDeviceWindow, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
        }
        else {
			::SetForegroundWindow(mEditorDeviceWindow);
            flag = false;
        }
    }


}

void EditorDevice::Render() {
	mDirectXImpl->mRenderer.Render();
}


EditorDevice gDevice{};