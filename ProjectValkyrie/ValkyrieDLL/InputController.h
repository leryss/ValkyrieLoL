#pragma once
#include "HKey.h"
#include "Vector.h"
#include "FakeMouse.h"
#include "GameObject.h"
#include "GameUnit.h"
#include "DirectInputHook.h"

#include <queue>
#include <chrono>
#include <windows.h>

using namespace std::chrono;

enum IoStepId {
	IO_PRESS_KEY,
	IO_PRESS_KEY_AT,
	IO_CLICK,
	IO_CLICK_AT,
	IO_CLICK_UNIT,
	IO_HOLD_KEY,
	IO_UNHOLD_KEY,
	IO_UNHOLD_KEY_AT,
	IO_DELAY
};

enum ClickType {
	CT_LEFT_CLICK,
	CT_RIGHT_CLICK
};

class IoStep {
	
public:
	void         Start() {};
	virtual bool Update() = 0;
};

class IoStepBatch {
public:
	     IoStepBatch(std::initializer_list<std::shared_ptr<IoStep>> initSteps, int controlId);
	void Start();
	bool Update();
	bool operator==(const IoStepBatch& other);

public:
	int                                 controlId;
	
private:
	std::shared_ptr<IoStep>             currentStep;
	std::queue<std::shared_ptr<IoStep>> steps;
};

/// Input controller. One instance of this utility is intended for each entity because of functions like WasPressed that are limited to one caller only.
class InputController {

public:

	/// Checks if the key is held down
	bool    IsDown(HKey key);

	/// Checks if key was pressed in the last `lastMillis` milliseconds. This can have only one caller
	bool    WasPressed(HKey key, float lastMillis = 250.f);

	/// Gets cursor mouse cursor position
	Vector2 GetMouseCursor();

	/// Sets the mouse cursor position
	void    SetMouseCursor(const Vector2& position);

	/// The user of this InputController can queue up async input operations for performance purposes but the user also has to update these operations manually by calling this function.
	void UpdateIssuedOperations();

	/// Issues a io operation to press a specific key
	void IssuePressKey(HKey key);

	/// Issues a io operation to press a specific key at a specific screen location (mouse position will be reverted after key is pressed)
	void IssuePressKeyAt(HKey key, std::function<Vector2()> posGetter);

	/// Issues a mouse click operation
	void IssueClick(ClickType type, HKey withKeyHold = NO_KEY);

	/// Issues a mouse click operation at a screen position
	void IssueClickAt(ClickType type, std::function<Vector2()> posGetter, HKey withKeyHold = NO_KEY);

	/// Issues a click operation on a game unit
	void IssueClickUnit(ClickType type, const GameUnit& unit);
	
	/// Issues a hold down key operation
	void IssueHoldKey(HKey key);

	/// Issues a unhold key operation
	void IssueUnholdKey(HKey key);

	/// Issues a unhold key operation at a position
	void IssueUnholdKeyAt(HKey key, std::function<Vector2()> posGetter);

	/// Issues a delay of N milliseconds
	void IssueDelay(float millis);

	/// Imgui utility to display a key selector
	static int ImGuiKeySelect(const char* label, int key);

	/// Maps hardware keys to virtual keys
	int GetVirtualKey(HKey key);

private:
	void PushIoBatch(std::shared_ptr<IoStepBatch> batch);

public:
	static const float                ScreenWidth;
	static const float                ScreenHeight;
	static const float                WidthRatio;
	static const float                HeightRatio;

private:
	std::shared_ptr<IoStepBatch>             ioCurrent;
	std::deque<std::shared_ptr<IoStepBatch>> ioQueue;

	duration<float, std::milli>       timeDiff;
	high_resolution_clock::time_point nowTime;
	high_resolution_clock::time_point lastTimePressed[300] = { high_resolution_clock::now() };
	bool                              pressed[300] = { 0 };
};

class IoDelay : public IoStep {

public:
	IoDelay(float delayMillis) {
		this->delayMillis = delayMillis;
	}

	void Start() {
		startTime = high_resolution_clock::now();
	}

	bool Update() {
		timeDiff = high_resolution_clock::now() - startTime;
		if (timeDiff.count() < delayMillis)
			return false;
		return true;
	}

	float                             delayMillis;
	duration<float, std::milli>       timeDiff;
	high_resolution_clock::time_point startTime;
};

class IoPressKey : public IoStep {

public:
	IoPressKey(HKey key) {
		this->key = key;
	}

	bool Update() {
		DirectInputHook::QueueKey(key, true);
		return true;
	}

	HKey key;
};

class IoReleaseKey : public IoStep {

public:
	IoReleaseKey(HKey key) {
		this->key = key;
	}

	bool Update() {
		DirectInputHook::QueueKey(key, false);
		return true;
	}

	HKey key;
};

class IoSpoofMouse : public IoStep {

public:
	IoSpoofMouse(Vector2 pos) {
		FakeMouse::FakePositionGetter = [pos] { return pos; };
	}

	IoSpoofMouse(std::function<Vector2()>& getter) {
		FakeMouse::FakePositionGetter = getter;
	}

	bool Update() {
		FakeMouse::Enabled = true;
		return true;
	}
};

class IoUnspoofMouse : public IoStep {

public:
	bool Update() {
		FakeMouse::Enabled = false;
		return true;
	}
};

class IoPressMouse : public IoStep {

public:
	IoPressMouse(ClickType type, std::function<bool()> condition = nullptr) {
		this->type = type;
		this->condition = condition;
	}

	bool Update() {
		if (condition && !condition())
			return true;

		INPUT input = { 0 };
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = (type == ClickType::CT_LEFT_CLICK ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN);
		SendInput(1, &input, sizeof(INPUT));

		return true;
	}

	ClickType type;
	std::function<bool()> condition;
};

class IoReleaseMouse : public IoStep {

public:
	IoReleaseMouse(ClickType type, std::function<bool()> condition = nullptr) {
		this->type = type;
		this->condition = condition;
	}

	bool Update() {
		if (condition && !condition())
			return true;

		INPUT input = { 0 };
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = (type == ClickType::CT_LEFT_CLICK ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP);
		SendInput(1, &input, sizeof(INPUT));

		return true;
	}

	ClickType type;
	std::function<bool()> condition;
};

class IoBlockInput : public IoStep {

public:

	bool Update() {
		BlockInput(TRUE);
		return true;
	}
};

class IoUnBlockInput : public IoStep {

public:

	bool Update() {
		BlockInput(FALSE);
		return true;
	}
};