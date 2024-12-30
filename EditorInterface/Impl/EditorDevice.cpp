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
    if (!InitWindow()) {
        ::abort();
    }
}

EditorDevice::~EditorDevice()
{
}

void EditorDevice::Join()
{
}

bool EditorDevice::InitWindow()
{
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
    if (!RegisterClassEx(&wc))
    {
        MessageBox(nullptr, L"Failed to register window class!", L"Error", MB_ICONERROR);
        return false;
    }



    return true;
}


EditorDevice gDevice{};