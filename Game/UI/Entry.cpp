#include "pch.h"
#include "Entry.h"


void Entry::Init(Canvas& cancvas, UINT frame, UINT carrot, float LTx, float LTy, float width, float height, size_t max) {
	mCanvas = &cancvas;
	mFrame = cancvas.CreateCanvasObject(); 
	mFrame.ChangeImage(frame);
	mFrame.GetRect() = { LTx, LTy, width, height };

	mCarrot = cancvas.CreateCanvasObject();
	mCarrot.ChangeImage(carrot); 
	mCarrot.GetRect() = { LTx + width * 0.05f , LTy + height * 0.1f, 5.f, height * 0.8f };
	mCarrot.SetActive(false);

	mText = TextBlockManager::GetInstance().CreateTextBlock(L"", D2D1_RECT_F{ LTx + width * 0.05f, LTy + height * 0.1f, LTx + width * 0.95f, LTy + height * 0.8f }, StringColor::Black, "NotoSansKR_Big");

	mInputActive = false;
}

void Entry::SetActiveState(bool state) {
	mFrame.SetActive(state);
	mCarrot.SetActive(state);
	mText->SetActiveState(state);
	mInputActive = state;
}

void Entry::Update() {
	if (not mFrame.GetActive()) {
		return; 
	}

	RECT clientRect{ mCanvas->GetClientRect() };
	const float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
	const float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);

	constexpr float renderWidth{ Config::WINDOW_WIDTH<float> };
	constexpr float renderHeight{ Config::WINDOW_HEIGHT<float> };

	const float mouseX = Input.GetMouseState().x * renderWidth / clientWidth;
	const float mouseY = Input.GetMouseState().y * renderHeight / clientHeight;

	auto& rect = mFrame.GetRect();

	// 마우스 커서가 안쪽일 때
	if (Entry::Inside(rect.LTx, rect.LTx + rect.width, mouseX) && Entry::Inside(rect.LTy, rect.LTy + rect.height, mouseY)) {
		// 왼쪽 마우스를 눌렀으면 
		if (Input.GetMouseTracker().leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED) {
			// 캐럿과 입력 활성화 
			mInputActive = true;
			mCarrot.SetActive(true);
		}

	}
	// 마우스 커서가 바깥쪽일 때, 
	else {
		// 어느 마우스라도 눌렀다면 
		if (Input.GetMouseTracker().leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED or 
			Input.GetMouseTracker().rightButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED) {
			// 캐럿과 입력 비활성화
			mInputActive = false;
			mCarrot.SetActive(false);
		}
	}

	if (mInputActive) {
		mCarrotCount--;

		if (mCarrotCount == 0) {
			if (mCarrot.GetActive()) {
				mCarrot.SetActive(false);
				mCarrotCount = 100; 
			}
			else {
				mCarrot.SetActive(true);
				mCarrotCount = 100;
			}
		}
	}
	else {
		mCarrot.SetActive(false);
		mCarrotCount = 100; 
	}

	if (mInputActive) {
		if (Input.GetKeyboardTracker().IsKeyPressed(DirectX::Keyboard::Keys::Back)) {
			if (not mText->GetText().empty()) {
				mText->GetText().pop_back();
			}
		}

		auto newInput = ConvertUtf8ToWstring(GetPressedText().c_str());

		
		std::wstring& currentText = mText->GetText();
		size_t remaining = maxLength > currentText.length() ? maxLength - currentText.length() : 0;
		if (newInput.length() > remaining) {
			newInput = newInput.substr(0, remaining);
		}

		mText->GetText() += newInput;

		mText->UpdateLayout();

		DWRITE_HIT_TEST_METRICS metrics;
		float x = 0.0f;
		float y = 0.0f;
		mText->GetTextLayout()->HitTestTextPosition(static_cast<UINT32>(mText->GetText().length()), FALSE, &x, &y, &metrics);

		mCarrot.GetRect() = {
			mFrame.GetRect().LTx + x + mFrame.GetRect().width * 0.05f,
			mFrame.GetRect().LTy + mFrame.GetRect().height * 0.1f,
			5.f,
			mFrame.GetRect().height * 0.8f
		};
	}



	mFrame.Update(); 
	mCarrot.Update();
}

std::string Entry::GetText() {
	return ConvertWstringToUtf8(mText->GetText());
}

CanvasRect& Entry::GetRect() {
	return mFrame.GetRect(); 
}

bool Entry::Inside(float min, float max, float value) const {
	return ((value > min) and (value < max));
}

char Entry::GetPrintableChar(DirectX::Keyboard::Keys key, bool shift) const {
	switch (key) {
	case DirectX::Keyboard::Keys::D0: return shift ? ')' : '0';
	case DirectX::Keyboard::Keys::D1: return shift ? '!' : '1';
	case DirectX::Keyboard::Keys::D2: return shift ? '@' : '2';
	case DirectX::Keyboard::Keys::D3: return shift ? '#' : '3';
	case DirectX::Keyboard::Keys::D4: return shift ? '$' : '4';
	case DirectX::Keyboard::Keys::D5: return shift ? '%' : '5';
	case DirectX::Keyboard::Keys::D6: return shift ? '^' : '6';
	case DirectX::Keyboard::Keys::D7: return shift ? '&' : '7';
	case DirectX::Keyboard::Keys::D8: return shift ? '*' : '8';
	case DirectX::Keyboard::Keys::D9: return shift ? '(' : '9';

	case DirectX::Keyboard::Keys::A: return shift ? 'A' : 'a';
	case DirectX::Keyboard::Keys::B: return shift ? 'B' : 'b';
	case DirectX::Keyboard::Keys::C: return shift ? 'C' : 'c';
	case DirectX::Keyboard::Keys::D: return shift ? 'D' : 'd';
	case DirectX::Keyboard::Keys::E: return shift ? 'E' : 'e';
	case DirectX::Keyboard::Keys::F: return shift ? 'F' : 'f';
	case DirectX::Keyboard::Keys::G: return shift ? 'G' : 'g';
	case DirectX::Keyboard::Keys::H: return shift ? 'H' : 'h';
	case DirectX::Keyboard::Keys::I: return shift ? 'I' : 'i';
	case DirectX::Keyboard::Keys::J: return shift ? 'J' : 'j';
	case DirectX::Keyboard::Keys::K: return shift ? 'K' : 'k';
	case DirectX::Keyboard::Keys::L: return shift ? 'L' : 'l';
	case DirectX::Keyboard::Keys::M: return shift ? 'M' : 'm';
	case DirectX::Keyboard::Keys::N: return shift ? 'N' : 'n';
	case DirectX::Keyboard::Keys::O: return shift ? 'O' : 'o';
	case DirectX::Keyboard::Keys::P: return shift ? 'P' : 'p';
	case DirectX::Keyboard::Keys::Q: return shift ? 'Q' : 'q';
	case DirectX::Keyboard::Keys::R: return shift ? 'R' : 'r';
	case DirectX::Keyboard::Keys::S: return shift ? 'S' : 's';
	case DirectX::Keyboard::Keys::T: return shift ? 'T' : 't';
	case DirectX::Keyboard::Keys::U: return shift ? 'U' : 'u';
	case DirectX::Keyboard::Keys::V: return shift ? 'V' : 'v';
	case DirectX::Keyboard::Keys::W: return shift ? 'W' : 'w';
	case DirectX::Keyboard::Keys::X: return shift ? 'X' : 'x';
	case DirectX::Keyboard::Keys::Y: return shift ? 'Y' : 'y';
	case DirectX::Keyboard::Keys::Z: return shift ? 'Z' : 'z';

	case DirectX::Keyboard::Keys::Space: return ' ';
	case DirectX::Keyboard::Keys::OemSemicolon: return shift ? ':' : ';';
	case DirectX::Keyboard::Keys::OemPlus: return shift ? '+' : '=';
	case DirectX::Keyboard::Keys::OemComma: return shift ? '<' : ',';
	case DirectX::Keyboard::Keys::OemMinus: return shift ? '_' : '-';
	case DirectX::Keyboard::Keys::OemPeriod: return shift ? '>' : '.';
	case DirectX::Keyboard::Keys::OemQuestion: return shift ? '?' : '/';
	case DirectX::Keyboard::Keys::OemTilde: return shift ? '~' : '`';
	case DirectX::Keyboard::Keys::OemOpenBrackets: return shift ? '{' : '[';
	case DirectX::Keyboard::Keys::OemPipe: return shift ? '|' : '\\';
	case DirectX::Keyboard::Keys::OemCloseBrackets: return shift ? '}' : ']';
	case DirectX::Keyboard::Keys::OemQuotes: return shift ? '\"' : '\'';
	case DirectX::Keyboard::Keys::OemBackslash: return shift ? '|' : '\\';

	default:
		return '\0'; 
	}
}

std::string Entry::GetPressedText() {
	const auto& tracker = Input.GetKeyboardTracker();
	bool shift = Input.GetKeyboardState().IsKeyDown(DirectX::Keyboard::Keys::LeftShift) || Input.GetKeyboardState().IsKeyDown(DirectX::Keyboard::Keys::RightShift);

	std::string result;

	for (int i = 0; i <= (int)DirectX::Keyboard::Keys::END; ++i) {
		DirectX::Keyboard::Keys key = static_cast<DirectX::Keyboard::Keys>(i);
		if (tracker.IsKeyPressed(key)) {
			char c = GetPrintableChar(key, shift);
			if (c != '\0') {
				result += c;
			}
		}
	}

	return result;
}