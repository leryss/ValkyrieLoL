#include "InputController.h"
#include <windows.h>
#include "imgui/imgui.h"

const float InputController::ScreenHeight = (float)::GetSystemMetrics(SM_CYSCREEN) - 1;
const float InputController::ScreenWidth  = (float)::GetSystemMetrics(SM_CXSCREEN) - 1;

const float InputController::HeightRatio = 65535.0f / ScreenHeight;
const float InputController::WidthRatio  = 65535.0f / ScreenWidth;

bool InputController::IsDown(HKey key)
{
	int virtualKey = MapVirtualKeyA(key, MAPVK_VSC_TO_VK);
	if (virtualKey == 0)
		return false;

	return GetAsyncKeyState(virtualKey);
}

bool InputController::WasPressed(HKey key, float lastMillis)
{
	int virtualKey = MapVirtualKeyA(key, MAPVK_VSC_TO_VK);
	if (virtualKey == 0)
		return false;

	nowTime = high_resolution_clock::now();
	timeDiff = nowTime - lastTimePressed[virtualKey];
	if (pressed[virtualKey]) {

		if (timeDiff.count() > 200)
			pressed[virtualKey] = false;
		return false;
	}

	bool keyDown = GetAsyncKeyState(virtualKey) & 0x8000;
	if (keyDown) {
		lastTimePressed[virtualKey] = high_resolution_clock::now();
		pressed[virtualKey] = true;
		return true;
	}

	return false;
}

Vector2 InputController::GetMouseCursor()
{
	POINT pos;
	GetCursorPos(&pos);
	return { (float)pos.x, (float)pos.y };
}

void InputController::UpdateIssuedOperations()
{
	if (ioCurrent == nullptr) {
		if (ioQueue.size() == 0)
			return;
		ioCurrent = ioQueue.front();
		ioCurrent->Start();
		ioQueue.pop();
	}

	if (ioCurrent->Update()) {
		delete ioCurrent;
		ioCurrent = nullptr;
	}
}

void InputController::IssuePressKey(HKey key)
{
	ioQueue.push(new IoPressKey(key));
	ioQueue.push(new IoDelay(10.f));
	ioQueue.push(new IoReleaseKey(key));
}

void InputController::IssueClick(ClickType type)
{
	ioQueue.push(new IoPressMouse(type));
	ioQueue.push(new IoDelay(10.f));
	ioQueue.push(new IoReleaseMouse(type));
}

void InputController::IssueClickAt(ClickType type, std::function<Vector2()> posGetter)
{
	ioQueue.push(new IoSpoofMouse(posGetter));
	IssueClick(type);
	ioQueue.push(new IoUnspoofMouse());
}

void DrawButton(HKey key, HKey& clickedBtn, bool& wasClicked) {
	if (ImGui::Button(HKeyNames[key])) {
		clickedBtn = key;
		wasClicked = true;
	}
	ImGui::SameLine();
}

int InputController::ImGuiKeySelect(const char* label, int key)
{
	ImGui::PushID(label);
	ImGui::BeginGroup();
	if (ImGui::Button(HKeyNames[key])) {
		ImGui::OpenPopup("Keys");
	}
	ImGui::SameLine();
	ImGui::Text(label);
	ImGui::EndGroup();


	if (ImGui::BeginPopup("Keys")) {

		HKey clickedBtn = HKey::NO_KEY;
		bool wasClicked = false;

		ImGui::BeginGroup();
		DrawButton(HKey::ESC, clickedBtn, wasClicked);
		DrawButton(HKey::F1, clickedBtn, wasClicked);
		DrawButton(HKey::F2, clickedBtn, wasClicked);
		DrawButton(HKey::F3, clickedBtn, wasClicked);
		DrawButton(HKey::F4, clickedBtn, wasClicked);
		DrawButton(HKey::F6, clickedBtn, wasClicked);
		DrawButton(HKey::F6, clickedBtn, wasClicked);
		DrawButton(HKey::F7, clickedBtn, wasClicked);
		DrawButton(HKey::F8, clickedBtn, wasClicked);
		DrawButton(HKey::F9, clickedBtn, wasClicked);
		DrawButton(HKey::F10, clickedBtn, wasClicked);
		ImGui::EndGroup();

		ImGui::BeginGroup();
		DrawButton(HKey::TILDE, clickedBtn, wasClicked);
		DrawButton(HKey::N_1, clickedBtn, wasClicked);
		DrawButton(HKey::N_2, clickedBtn, wasClicked);
		DrawButton(HKey::N_3, clickedBtn, wasClicked);
		DrawButton(HKey::N_4, clickedBtn, wasClicked);
		DrawButton(HKey::N_5, clickedBtn, wasClicked);
		DrawButton(HKey::N_6, clickedBtn, wasClicked);
		DrawButton(HKey::N_7, clickedBtn, wasClicked);
		DrawButton(HKey::N_8, clickedBtn, wasClicked);
		DrawButton(HKey::N_9, clickedBtn, wasClicked);
		DrawButton(HKey::N_0, clickedBtn, wasClicked);
		DrawButton(HKey::MINUS, clickedBtn, wasClicked);
		DrawButton(HKey::EQUAL, clickedBtn, wasClicked);
		DrawButton(HKey::BS, clickedBtn, wasClicked);
		ImGui::EndGroup();

		ImGui::BeginGroup();
		DrawButton(HKey::Tab, clickedBtn, wasClicked);
		DrawButton(HKey::Q, clickedBtn, wasClicked);
		DrawButton(HKey::W, clickedBtn, wasClicked);
		DrawButton(HKey::E, clickedBtn, wasClicked);
		DrawButton(HKey::R, clickedBtn, wasClicked);
		DrawButton(HKey::T, clickedBtn, wasClicked);
		DrawButton(HKey::Y, clickedBtn, wasClicked);
		DrawButton(HKey::U, clickedBtn, wasClicked);
		DrawButton(HKey::I, clickedBtn, wasClicked);
		DrawButton(HKey::O, clickedBtn, wasClicked);
		DrawButton(HKey::P, clickedBtn, wasClicked);
		DrawButton(HKey::LBRACKET, clickedBtn, wasClicked);
		DrawButton(HKey::RBRACKET, clickedBtn, wasClicked);
		DrawButton(HKey::ENTER, clickedBtn, wasClicked);
		ImGui::EndGroup();

		ImGui::BeginGroup();
		DrawButton(HKey::CAPS, clickedBtn, wasClicked);
		DrawButton(HKey::A, clickedBtn, wasClicked);
		DrawButton(HKey::S, clickedBtn, wasClicked);
		DrawButton(HKey::D, clickedBtn, wasClicked);
		DrawButton(HKey::F, clickedBtn, wasClicked);
		DrawButton(HKey::G, clickedBtn, wasClicked);
		DrawButton(HKey::H, clickedBtn, wasClicked);
		DrawButton(HKey::J, clickedBtn, wasClicked);
		DrawButton(HKey::K, clickedBtn, wasClicked);
		DrawButton(HKey::L, clickedBtn, wasClicked);
		DrawButton(HKey::SEMICOLON, clickedBtn, wasClicked);
		DrawButton(HKey::SINGLE_QUOTE, clickedBtn, wasClicked);
		DrawButton(HKey::BACKSLASH, clickedBtn, wasClicked);
		DrawButton(HKey::NO_KEY, clickedBtn, wasClicked);
		ImGui::EndGroup();

		ImGui::BeginGroup();
		DrawButton(HKey::LSHIFT, clickedBtn, wasClicked);
		DrawButton(HKey::Z, clickedBtn, wasClicked);
		DrawButton(HKey::X, clickedBtn, wasClicked);
		DrawButton(HKey::C, clickedBtn, wasClicked);
		DrawButton(HKey::V, clickedBtn, wasClicked);
		DrawButton(HKey::B, clickedBtn, wasClicked);
		DrawButton(HKey::N, clickedBtn, wasClicked);
		DrawButton(HKey::M, clickedBtn, wasClicked);
		DrawButton(HKey::COMMA, clickedBtn, wasClicked);
		DrawButton(HKey::DOT, clickedBtn, wasClicked);
		DrawButton(HKey::FRONTSLASH, clickedBtn, wasClicked);
		DrawButton(HKey::RSHIFT, clickedBtn, wasClicked);
		ImGui::EndGroup();

		ImGui::BeginGroup();
		DrawButton(HKey::CTRL, clickedBtn, wasClicked);
		DrawButton(HKey::ALT, clickedBtn, wasClicked);
		DrawButton(HKey::SPACE, clickedBtn, wasClicked);
		DrawButton(HKey::HOME, clickedBtn, wasClicked);
		DrawButton(HKey::INSERT, clickedBtn, wasClicked);
		DrawButton(HKey::END, clickedBtn, wasClicked);
		DrawButton(HKey::PAGE_DOWN, clickedBtn, wasClicked);
		DrawButton(HKey::PAGE_UP, clickedBtn, wasClicked);
		ImGui::EndGroup();

		if (wasClicked) {
			key = clickedBtn;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::PopID();
	return key;
}
