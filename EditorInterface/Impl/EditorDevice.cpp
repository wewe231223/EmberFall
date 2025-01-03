#include "pch.h"
#include "EditorDevice.h"
#include "../Config/Config.h"

LRESULT CALLBACK EditorDeviceProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

EditorDevice::EditorDevice()
{
}

EditorDevice::~EditorDevice()
{
}

EditorDevice::operator bool() const
{
    return mEditorDeviceWindow != nullptr;
}

void EditorDevice::Initialize(HWND mainWindowHandle)
{
    mMainWindow = mainWindowHandle;
    mEditorDeviceThread = std::thread{ [this]() {EditorWindowWorker(); } };
}

void EditorDevice::WaitForTerminate()
{
    if (mEditorDeviceThread.joinable()) {
        if (AUTOMATIC_CLOSED) {
            PostThreadMessage(mEditorThreadID, WM_QUIT, 0, 0);
        }
        mEditorDeviceThread.join();
    }
}

// 여기서부터는 Device Thread 에서 작동하는 함수들이다. 
void EditorDevice::EditorWindowWorker()
{
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
        Config::EDITOR_WINDOW_WIDTH<int>, Config::EDITOR_WINDOW_HEIGHT<int>, // 크기
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
}

// TODO ::  이 함수는 메인 윈도우 핸들에 대한 Data Race 가능성을 고려하지 않고, 내가 하고자 하는 기능만 구현하였음. 
//          이 코드 조각에서 Data Race 를 제거하면서도, lock 을 최대한 줄일 수 있는 방법은?  
//          ( 여기서 내가 추측하는 Data Race 는 현재 GetWindowRect 의 리턴을 통해 현재 창이 유효한지 간접적으로 검사 하고 있다. 
//          하지만 GetWindowRect 이후에 창이 파괴되는 경우, MainWindowHandle 에 대한 일관성이 없어진다. - 두 쓰레드가 ( 메인 쓰레드, Editor 쓰레드 ) 동시에 메인 윈도우 핸들에 접근해서 발생하는 일 
void EditorDevice::Update()
{
	static bool flag = true;
    
	std::unique_lock lock{ mEditorDeviceMutex };
    if (flag) {
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
    lock.unlock();

}

void EditorDevice::Render()
{
}


EditorDevice gDevice{};