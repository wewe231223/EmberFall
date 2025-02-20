#pragma once 

#define VIRTUAL_MOUSE 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <memory> 
#include <array>
#include <functional>
#include "../External/Include/DirectXTK12/Keyboard.h"
#include "../External/Include/DirectXTK12/Mouse.h"


class GInput {
public:
	GInput();
	~GInput();

	void Initialize(HWND);
	void Update();

	void DisableVirtualMouse();
	void EnableVirtualMouse();
	void UpdateFocus(UINT msg);
	void UpdateWindowCenter();

	DirectX::Keyboard::State& GetKeyboardState();
	DirectX::Mouse::State& GetMouseState();
	DirectX::Keyboard::KeyboardStateTracker& GetKeyboardTracker();
	DirectX::Mouse::ButtonStateTracker& GetMouseTracker();

	void RegisterKeyPressCallBack(DirectX::Keyboard::Keys key, int sign, std::function<void()>&& callback);
	void RegisterKeyDownCallBack(DirectX::Keyboard::Keys key, int sign, std::function<void()>&& callback);
	void RegisterKeyReleaseCallBack(DirectX::Keyboard::Keys key, int sign, std::function<void()>&& callback);

	float GetDeltaMouseX() const;
	float GetDeltaMouseY() const;

	void EraseCallBack(int sign);
private:
	HWND mWindowHandle{ nullptr };

	std::unique_ptr<DirectX::Keyboard> mKeyboard{ std::make_unique<DirectX::Keyboard>() };
	std::unique_ptr<DirectX::Mouse> mMouse{ std::make_unique<DirectX::Mouse>() };

	DirectX::Keyboard::State mKeyboardState{};

	POINT mWindowCenter{ 0,0 };
	DirectX::Mouse::State mMouseState{};
	bool mWindowFocused{ true };
	bool mVirtualMouse{ false };

	DirectX::Keyboard::KeyboardStateTracker mKeyboardTracker{};
	DirectX::Mouse::ButtonStateTracker mMouseTracker{};

	// 키당 콜백이 여러개일 필요가... 
	std::array<std::vector<std::pair<std::function<void()>, int>>, DirectX::Keyboard::Keys::END> mKeyDownCallbacks{};
	std::array<std::vector<std::pair<std::function<void()>, int>>, DirectX::Keyboard::Keys::END> mKeyPressCallbacks{};
	std::array<std::vector<std::pair<std::function<void()>, int>>, DirectX::Keyboard::Keys::END> mKeyReleaseCallbacks{};
};


extern GInput Input;