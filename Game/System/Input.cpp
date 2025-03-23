#include "pch.h"
#include "Input.h"
#include <numeric>
#include "../Config/Config.h"


std::array<unsigned char, DirectX::Keyboard::Keys::END>  keys{};
GInput::GInput()
{
	std::iota(keys.begin(), keys.end(), 0);
}

GInput::~GInput()
{
#ifdef VIRTUAL_MOUSE
	ShowCursor(true);
#endif // VIRTUAL_MOUSE
}

void GInput::Initialize(HWND hWnd)
{
	mWindowHandle = hWnd;

	GInput::EnableVirtualMouse();

}

void GInput::Update()
{
	mMouseState = mMouse->GetState();
#ifdef VIRTUAL_MOUSE
	if (mWindowFocused and mVirtualMouse) {
		SetCursorPos(mWindowCenter.x, mWindowCenter.y);
	}
#endif // VIRTUAL_MOUSE

	mKeyboardState = mKeyboard->GetState();
	mMouseState = mMouse->GetState();

	mKeyboardTracker.Update(mKeyboardState);
	mMouseTracker.Update(mMouseState);

	for (auto& key : keys) {
		if (mKeyboardState.IsKeyDown(static_cast<DirectX::Keyboard::Keys>(key))) {
			if (mKeyboardTracker.IsKeyPressed(static_cast<DirectX::Keyboard::Keys>(key))) {
				for (auto& [callback, sign] : mKeyDownCallbacks[key]) {
					std::invoke(callback);
				}
			}
			else {
				for (auto& [callback, sign] : mKeyPressCallbacks[key]) {
					std::invoke(callback);
				}
			}
		}
		
		
		if (mKeyboardTracker.IsKeyReleased(static_cast<DirectX::Keyboard::Keys>(key))) {
			for (auto& [callback, sign] : mKeyReleaseCallbacks[key]) {
				std::invoke(callback);
			}
		}
	}


}

void GInput::ToggleVirtualMouse() {
#ifdef VIRTUAL_MOUSE
	if (mVirtualMouse) {
		DisableVirtualMouse();
	}
	else {
		EnableVirtualMouse();
	}
#endif
}

void GInput::DisableVirtualMouse()
{
#ifdef VIRTUAL_MOUSE
	if (mVirtualMouse) {
		ShowCursor(true);
		mVirtualMouse = false;
	}
#endif // VIRTUAL_MOUSE
}

void GInput::EnableVirtualMouse()
{
#ifdef VIRTUAL_MOUSE
	if (not mVirtualMouse) {
		ShowCursor(false);
		mVirtualMouse = true;
		GInput::UpdateWindowCenter();
	}
#endif // VIRTUAL_MOUSE
}



void GInput::UpdateFocus(UINT msg)
{
	if (msg == WM_KILLFOCUS) {
		mWindowFocused = false;
	}
	else {
		mWindowFocused = true;
	}
}

void GInput::UpdateWindowCenter()
{
	mWindowCenter = { 0,0 };
	ClientToScreen(mWindowHandle, &mWindowCenter);

	mWindowCenter.x += Config::WINDOW_WIDTH<int> / 2;
	mWindowCenter.y += Config::WINDOW_HEIGHT<int> / 2;

}

DirectX::Keyboard::State& GInput::GetKeyboardState()
{
	return std::ref(mKeyboardState);
}

DirectX::Mouse::State& GInput::GetMouseState()
{
	return std::ref(mMouseState);
}

DirectX::Keyboard::KeyboardStateTracker& GInput::GetKeyboardTracker()
{
	return std::ref(mKeyboardTracker);
}

DirectX::Mouse::ButtonStateTracker& GInput::GetMouseTracker()
{
	return std::ref(mMouseTracker);
}

void GInput::RegisterKeyPressCallBack(DirectX::Keyboard::Keys key, int sign, std::function<void()>&& callback)
{
	mKeyPressCallbacks[key].emplace_back(callback, sign);

}

void GInput::RegisterKeyDownCallBack(DirectX::Keyboard::Keys key, int sign, std::function<void()>&& callback)
{
	mKeyDownCallbacks[key].emplace_back(callback, sign);
}

void GInput::RegisterKeyReleaseCallBack(DirectX::Keyboard::Keys key, int sign, std::function<void()>&& callback)
{
	mKeyReleaseCallbacks[key].emplace_back(callback, sign);
}

float GInput::GetDeltaMouseX() const
{
#ifdef VIRTUAL_MOUSE
	if ((not mWindowFocused) or (not mVirtualMouse)) {
		return 0.f;
	}
	return static_cast<float>(mMouseState.x - (Config::WINDOW_WIDTH<int> / 2));
#else
	return static_cast<float>(0);
#endif // VIRTUAL_MOUSE
}

float GInput::GetDeltaMouseY() const
{
#ifdef VIRTUAL_MOUSE
	if ((not mWindowFocused) or (not mVirtualMouse)) {
		return 0.f;
	}
	return static_cast<float>(mMouseState.y - (Config::WINDOW_HEIGHT<int> / 2));
#else
	return static_cast<float>(0);
#endif // VIRTUAL_MOUSE
}

void GInput::EraseCallBack(int sign)
{
	for (auto& callbacks : mKeyPressCallbacks) {
		std::erase_if(callbacks, [sign](auto& callback) {return callback.second == sign; });
	}

	for (auto& callbacks : mKeyDownCallbacks) {
		std::erase_if(callbacks, [sign](auto& callback) {return callback.second == sign; });
	}

	for (auto& callbacks : mKeyReleaseCallbacks) {
		std::erase_if(callbacks, [sign](auto& callback) {return callback.second == sign; });
	}
}



GInput Input{};
