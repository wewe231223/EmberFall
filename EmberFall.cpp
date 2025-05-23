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
#include "Renderer/Core/Console.h"
#include "EmberFall.h"
#include "Renderer/core/Renderer.h"
#include "Game/System/Timer.h"
#include "Game/System/Input.h"
#include "Game/Scene/SceneManager.h"
#include "Utility/NonReplacementSampler.h"
#include "MeshLoader/Loader/MeshLoader.h"
#include "Utility/IntervalTimer.h"

#ifdef _DEBUG
#pragma comment(lib,"out/debug/Renderer.lib")
#pragma comment(lib,"out/debug/Game.lib")
#pragma comment(lib,"out/debug/MeshLoader.lib")
#else 
#pragma comment(lib,"out/release/Renderer.lib")
#pragma comment(lib,"out/release/Game.lib")
#pragma comment(lib,"out/release/MeshLoader.lib")
#endif

#include "Config/Config.h"


#ifdef _DEBUG 
#define MONITER_CPU_GPU_TIME
#endif 

#define MONITER_CPU_GPU_TIME

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
INT_PTR CALLBACK IPDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

SceneManager sceneManager{};

CHAR iPAddr[64]{};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    if (DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, IPDialogProc) == IDOK) {
        gClientCore->Init(); 
        if (not gClientCore->Start(iPAddr, 7777)) {
            CrashExp(true, "Failed to connect");
            return -1; 
        }
    } 
    
    MSG msg{};
    
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_EMBERFALL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }


    Renderer renderer{ hWnd };
    renderer.UploadResource();
	renderer.ResetLoadCommandList();

    Input.Initialize(hWnd);

	sceneManager.Init(
        renderer.GetRenderManager(),
		renderer.GetMainCameraBuffer(), 
        renderer.GetDevice(), 
        renderer.GetLoadCommandList(),
		[&renderer]() { renderer.LoadTextures(); });

    int n = NonReplacementSampler::GetInstance().Sample();
    
    Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::Escape, n, []() {
        PostQuitMessage(0);
    });
    
    Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::F2, n, []() {Input.ToggleVirtualMouse(); });
    Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::F5, n, [&renderer]() { renderer.ToggleFullScreen(); });

    size_t frameCount = 0;
    Time.AddEvent(1s, [&frameCount]() {
        std::string title = "FPS : " + std::to_string(frameCount);
        SetWindowTextA(hWnd, title.c_str());
        frameCount = 0;
        return true;
        });

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EMBERFALL));

#ifdef DEV_MODE
    TextBlock* CPUTime = TextBlockManager::GetInstance().CreateTextBlock(L"", D2D1_RECT_F{ 1000.f, 0.f, 1400.f, 100.f }, StringColor::Black, "NotoSansKR");
    IntervalTimer CPUTimer{};
    TextBlock* GPUTime = TextBlockManager::GetInstance().CreateTextBlock(L"", D2D1_RECT_F{ 1000.f, 30.f, 1400.f, 200.f }, StringColor::Black, "NotoSansKR");
    IntervalTimer GPUTimer{};
#endif 

    // 기본 메시지 루프입니다:
    while (true) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

        }
        else {
#ifdef DEV_MODE
            CPUTimer.Start();
#endif
            Time.AdvanceTime();
            Input.Update();

            sceneManager.Update(renderer.GetDevice(), renderer.GetLoadCommandList());


            renderer.Render();
#ifdef DEV_MODE
            CPUTimer.End();
#endif 
            if (sceneManager.CheckLoaded()) {
                renderer.ExecuteLoadCommandList();
            }
#ifdef DEV_MODE
            GPUTimer.Start();
#endif  
            renderer.ExecuteRender();
#ifdef DEV_MODE
            GPUTimer.End();
            CPUTime->GetText() = std::format(L"CPU Time : {:.2f}us", CPUTimer.Microseconds());
            GPUTime->GetText() = std::format(L"GPU Time : {:.2f}us", GPUTimer.Microseconds());
#endif

            // 게임 루프... 
            frameCount++;
        }
    }

    ::DestroyWindow(hWnd);

    decltype(auto) packet = FbsPacketFactory::PlayerExitCS(gClientCore->GetSessionId());
    gClientCore->Send(packet);

    gClientCore->End();

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
    ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);

    constexpr UINT keyPressedCheckBitMask = 0x60000000;
    constexpr UINT keyPressedAtTime = 0x20000000;

    switch (message) {
    case WM_ADVANCESCENE:
    {
        switch (wParam) {
        case Packets::GameStage::GameStage_LOBBY:
			sceneManager.ChangeSceneTo(SceneType::LOBBY);
			break;
		case Packets::GameStage::GameStage_TERRAIN:
			sceneManager.ChangeSceneTo(SceneType::TERRAIN);
			break;
        default:
            break;
        }
    }
    break;
        /**
        *	**아래의 WM_SYSKEYDOWN 메세지에 등장하는 KeyPressedBitMask, KeyPressedAtTime 값에 대한 설명입니다**
        *
        *	(lParam & 0x60000000) == 0x20000000 부분은 키보드 입력 메시지에서 lParam 값을 검사하여
        *	키가 눌린 상태와 반복된 횟수를 확인하는 것입니다.
        *	KeyPressedCheckBitMask 가 0x60000000, KeyPressedAtTime 이 0x20000000으로 정의되어 있습니다.
        *
        *	lParam 값은 키보드 메시지에서 다양한 정보를 담고 있는 32비트 값으로 구성되어 있습니다.
        *	lParam의 각 비트의 의미를 설명하면 다음과 같습니다:
        *
        *	비트 0-15: 키가 눌려진 반복 횟수
        *	비트 16-23: 키의 스캔 코드
        *	비트 24: 확장 키 여부 (0: 일반 키, 1: 확장 키)
        *	비트 25-28: 예약된 비트
        *	비트 29: 이전 키 상태 (0: 이전에 눌리지 않았음, 1: 이전에 눌림)
        *	비트 30: 키 상태 (0: 키가 눌렸음, 1: 키가 놓였음)
        *	비트 31: 전송된 메시지의 종류 (0: WM_KEYDOWN, 1: WM_KEYUP)
        *	이제 (lParam & 0x60000000) == 0x20000000의 의미를 해석해 보겠습니다:
        *
        *	0x60000000는 이진수로 01100000 00000000 00000000 00000000입니다.
        *	0x20000000는 이진수로 00100000 00000000 00000000 00000000입니다.
        *	이 조건문은 lParam 값의 비트 29와 비트 30을 확인하고 있습니다. 이를 비트별로 분석해보면:
        *
        *	(lParam & 0x60000000)은 lParam 값에서 비트 29와 비트 30을 추출합니다.
        *	비트 29는 키가 이전에 눌렸는지 여부를 나타냅니다.
        *	비트 30은 키가 현재 눌렸는지 여부를 나타냅니다.
        *	== 0x20000000은 비트 29가 0이고 비트 30이 1인지를 확인합니다.
        *
        *	즉, (lParam & 0x60000000) == 0x20000000 조건은 키가 처음 눌렸을 때만 참이 됩니다.
        *	키가 반복되거나 키가 놓였을 때는 이 조건이 거짓이 됩니다.
        *	이 조건을 통해 키가 처음 눌렸을 때 ALT+ENTER 조합을 처리하는 것입니다.
        *
        **/
    case WM_SYSKEYDOWN:
        if (wParam == VK_RETURN && (lParam & keyPressedCheckBitMask) == keyPressedAtTime) {
            // 여기에는 Alt + Enter 과 같은 경우에 대한 처리를 추가하면 됩니다. 

        }
        break;
    case WM_KILLFOCUS:
    case WM_SETFOCUS:
        // Input.UpdateFocus(message);
        break;

        // 아래는 모든 입력을 Keyboard 에게 넘기는 부분입니다. 
    case WM_ACTIVATEAPP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
        DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
        break;
    case WM_MENUCHAR:
        return MAKELRESULT(0, MNC_CLOSE);
        // 아래는 모든 입력을 Mouse 에게 넘기는 부분입니다. 
    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
        DirectX::Mouse::ProcessMessage(message, wParam, lParam);
        break;
    case WM_DESTROY:
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

       int posX = (GetSystemMetrics(SM_CXSCREEN) / 2) - (Config::WINDOW_WIDTH<> / 2 + Config::EDITOR_WINDOW_WIDTH<> / 2);
       int posY = (GetSystemMetrics(SM_CYSCREEN) / 2) - (Config::WINDOW_HEIGHT<> / 2);

       RECT adjustedRect{ 0, 0, Config::WINDOW_WIDTH<long>, Config::WINDOW_HEIGHT<long> };
       ::AdjustWindowRectEx(std::addressof(adjustedRect), style, FALSE, exStyle);

       hWnd = CreateWindowEx(
           exStyle,                                // 확장 스타일
           szWindowClass,                          // 윈도우 클래스 이름
           szTitle,                                // 윈도우 타이틀 
           style,                                  // 윈도우 스타일
           posX, posY,                             // 위치
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

	   int posX = (GetSystemMetrics(SM_CXSCREEN) / 2) - (Config::WINDOW_WIDTH<> / 2 + Config::EDITOR_WINDOW_WIDTH<> / 2);
	   int posY = (GetSystemMetrics(SM_CYSCREEN) / 2) - (Config::WINDOW_HEIGHT<> / 2 );

       hWnd = CreateWindowEx(
           exStyle,                                // 확장 스타일
           szWindowClass,                          // 윈도우 클래스 이름
           szTitle,                                // 윈도우 타이틀 
           style,                                  // 윈도우 스타일
           posX, posY,                             // 위치 
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

   return TRUE;
}


INT_PTR IPDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    constexpr const char* LOCALHOST{ "127.0.0.1" };

    switch (message) {
    case WM_INITDIALOG:
        SetDlgItemTextA(hWnd, IDC_IPADDRESS1, LOCALHOST);  // 기본 IP 표시
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            GetDlgItemTextA(hWnd, IDC_IPADDRESS1, iPAddr, sizeof(iPAddr));
            EndDialog(hWnd, IDOK);
            return TRUE;
        case IDCANCEL:
            EndDialog(hWnd, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}
