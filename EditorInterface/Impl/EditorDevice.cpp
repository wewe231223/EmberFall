#include "pch.h"
#include "EditorDevice.h"

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
    mEditorDeviceThread = std::thread{ [this]() {EditorWindowWorker(); } };
}

EditorDevice::~EditorDevice()
{
    if (mEditorDeviceThread.joinable()) {
        if (AUTOMATIC_CLOSED) {
            PostThreadMessage(mEditorThreadID, WM_QUIT, 0, 0);
        }
        mEditorDeviceThread.join();
    }
}

void EditorDevice::Test()
{
   
}

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

	mEditorDeviceWindow = CreateWindowW(wc.lpszClassName, L"EditorInterface", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, mApplicationHandle, nullptr);
   
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
        
	}

    // 이 함수가 종료되면, 메인 창도 꺼지도록 메세지를 보냄 
	::PostThreadMessage(mMainThreadID, WM_QUIT, 0, 0);
}


EditorDevice gDevice{};