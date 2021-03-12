#pragma once
#include "Logger.h"
#include "GameState.h"
#include "PyImGui.h"
#include "ConfigSet.h"
#include "CollisionEngine.h"
#include "ObjectQuery.h"

#include "imgui/imgui.h"
#include <boost/python.hpp>

using namespace boost::python;

class Script;
class PyExecutionContext {

public:
	ObjectQuery     queryEngine;

	CollisionEngine collisionEngine;
	PyImGui         imgui;
	GameState*      state;
	ImDrawList*     overlay;
	Script*         currentScript;

public:
	Vector2 pillPosition;

	float   ping;
	float   time;
	object  hovered;
	object  player;
	object  queryEnginePy;
	object  selfPy;

public:
	PyExecutionContext();

	object  GetImGuiInterface();
	object  GetChampions();
	object  GetMissiles();
	object  GetJungle();
	object  GetMinions();
	object  GetTurrets();
	object  GetOthers();
	object  GetConfig();

	void    SetScript(Script* script);
	void    SetGameState(GameState* state);
	void    SetImGuiOverlay(ImDrawList* overlay);

	/* Exposed methods */
	bool    IsKeyDown(int key);
	bool    WasKeyPressed(int key);
	void    MoveToMouse();
	void    MoveToLocation(const Vector3& location);
	void    MoveMouse(const Vector3& worldLocation);
	void    AttackUnit(const GameUnit& unit);

	list    GetCollisionsForUnit(const GameUnit& unit);
	list    GetCollisionsForCast(const SpellCast& cast);
	object  GetSpellInfo(const char* label);
	bool    CastSpell(GameSpell* spell, const Vector3& targetLocation);

	void    LogInfo(const char* msg);
	void    LogWarning(const char* msg);
	void    LogError(const char* msg);
	Vector2 World2Screen(const Vector3& world);
	Vector2 World2Minimap(const Vector3& world);
	float   DistanceOnMinimap(float dist);
	BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(IsScreenPointOnScreenOverloads, IsScreenPointOnScreen, 1, 3);
	bool    IsScreenPointOnScreen(const Vector2& point, float offsetX = 0.f, float offsetY = 0.f);
	BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(IsWorldPointOnScreenOverloads, IsWorldPointOnScreen, 1, 3);
	bool    IsWorldPointOnScreen(const Vector3& point, float offsetX = 0.f, float offsetY = 0.f);
	bool    IsInFountain(const GameObject& obj);

	void    DrawRectWorld(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4, float thickness, const ImVec4& color);
	void    DrawTriangleWorld(const Vector3& p1, const Vector3& p2, const Vector3& p3, float thickness, const ImVec4& color);
	void    DrawTriangleWorldFilled(const Vector3& p1, const Vector3& p2, const Vector3& p3, const ImVec4& color);
	void    DrawCircle(const Vector2& center, float radius, int numPoints, float thickness, const ImVec4& color);
	void    DrawCircleFilled(const Vector2& center, float radius, int numPoints, const ImVec4& color);
	void    DrawCircleWorld(const Vector3& center, float radius, int numPoints, float thickness, const ImVec4& color);
	void    DrawCircleWorldFilled(const Vector3& center, float radius, int numPoints, const ImVec4& color);
	void    DrawLine(const Vector2& start, const Vector2& end, float thickness, const ImVec4& color);
	void    DrawLineWorld(const Vector3 & start, const Vector3 & end, float thickness, const ImVec4 & color);
	void    DrawImage(const char* img, const Vector2& start, const Vector2& end, const ImVec4& color);
	void    DrawImageRounded(const char* img, const Vector2& start, const Vector2& end, const ImVec4& color, float rounding);
	void    DrawTxt(const Vector2& pos, const char* text, const ImVec4& color);
	BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(DrawRectOverloads, DrawRect, 3, 5);
	void    DrawRect(const Vector2& start, const Vector2& size, const ImVec4& color, float rounding = 0, float thickness = 1.0);
	BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(DrawRectFilledOverloads, DrawRectFilled, 3, 4);
	void    DrawRectFilled(const Vector2& start, const Vector2& size, const ImVec4& color, float rounding = 0);
	void    Pill(const char* text, const ImVec4& colText, const ImVec4& colRect);

private:
	object  Query(QueryKey key);
};