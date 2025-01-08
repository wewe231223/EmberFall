////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EmberFall.cpp
// 2024.12.23 김승범   - 프로젝트를 생성하고 이 파일을 작성함. 기본 코드 중 필요없는 부분을 제거함 
//                      프로젝트가 out 폴더의 debug/release 에 출력하도록 프로젝트 설정을 변경함 
// 2025.01.03 김승범   - EditorDevice 를 InitInstance 함수에서 메인 윈도우를 만들고 난 후, 초기화 하도록 변경함
//                      
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "framework.h"
#include "EmberFall.h"
#include "EditorInterface/Impl/EditorDevice.h"

#pragma comment(lib,"out/debug/EditorInterface.lib")
#include "Config/Config.h"

#define MAX_LOADSTRING 100
// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
HWND hWnd;
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_EMBERFALL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EMBERFALL));

    MSG msg{};
    
    // 기본 메시지 루프입니다:
    while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
		}
        // 게임 루프... 
    }

	::DestroyWindow(hWnd);
	gDevice.WaitForTerminate();

    return (int) msg.wParam;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EMBERFALL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.


   if constexpr (Config::WINDOWED) {
       DWORD style = WS_OVERLAPPEDWINDOW;
       DWORD exStyle = WS_EX_OVERLAPPEDWINDOW;

       RECT adjustedRect{ 0, 0, Config::WINDOW_WIDTH<long>, Config::WINDOW_HEIGHT<long> };
       ::AdjustWindowRectEx(std::addressof(adjustedRect), style, FALSE, exStyle);

       hWnd = CreateWindowEx(
           exStyle,                                // 확장 스타일
           szWindowClass,                          // 윈도우 클래스 이름
           szTitle,                                // 윈도우 타이틀 
           style,                                  // 윈도우 스타일
           CW_USEDEFAULT, CW_USEDEFAULT,           // 위치 
           adjustedRect.right - adjustedRect.left,
           adjustedRect.bottom - adjustedRect.top, // 크기
           nullptr,                                // 부모 윈도우
           nullptr,                                // 메뉴
           hInstance,                              // 인스턴스 핸들
           nullptr                                 // 추가 매개변수
       );
   }
   else {
       DWORD style = WS_POPUP;
       DWORD exStyle = NULL;

       hWnd = CreateWindowEx(
           exStyle,                                // 확장 스타일
           szWindowClass,                          // 윈도우 클래스 이름
           szTitle,                                // 윈도우 타이틀 
           style,                                  // 윈도우 스타일
           CW_USEDEFAULT, CW_USEDEFAULT,           // 위치 
           Config::WINDOW_WIDTH<long>, Config::WINDOW_HEIGHT<long>, // 크기
           nullptr,                                // 부모 윈도우
           nullptr,                                // 메뉴
           hInstance,                              // 인스턴스 핸들
           nullptr                                 // 추가 매개변수
       );
   }
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   gDevice.Initialize(hWnd);

   return TRUE;
}

