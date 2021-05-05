#pragma once
#include "Logger.h"
#include "GameState.h"
#include "PyImGui.h"
#include "ConfigSet.h"
#include "CollisionEngine.h"
#include "ObjectQuery.h"
#include "Raycast.h"
#include "GameKeybind.h"

#include "imgui/imgui.h"
#include <boost/python.hpp>

using namespace boost::python;

class Script;

/// Python script execution context provided to scripts when they are executed. Documentation for methods can be found in PyStructs.h
class PyExecutionContext {

public:
	GameKeybind     keybinds;
	ObjectQuery     queryEngine;

	CollisionEngine collisionEngine;
	PyImGui         imgui;
	GameState*      state;
	ImDrawList*     overlay;
	Script*         currentScript;

public:
	Vector2 pillPosition;

	bool    everythingLoaded;
	float   ping;
	float   time;
	object  focused;
	object  hovered;
	object  player;
	object  queryEnginePy;
	object  gameHud;
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

	Vector2 GetMousePosition();

	void    SetScript(Script* script);
	void    SetGameState(GameState* state);
	void    SetImGuiOverlay(ImDrawList* overlay);

	/* Exposed methods */
	void    SetKeyActive(HKey key, bool enabled);
	bool    IsKeyDown(int key);
	bool    WasKeyPressed(int key);
	void    MoveToMouse();
	void    MoveToLocation(const Vector3& location);
	void    MoveMouse(const Vector3& worldLocation);
	void    AttackUnit(const GameUnit& unit);

	void    PingNormal(const Vector3& position);
	void    PingWarn(const Vector3& position);
	void    PingDanger(const Vector3& position);
	void    PingVision(const Vector3& position);
	void    PingMia(const Vector3& position);
	void    PingOmw(const Vector3& position);
	void    PingAssist(const Vector3& position);

	object  Raycast(const Vector3& begin, const Vector3& dir, float length, float halfWidth, RaycastLayer layers);
	bool    IsWallAt(const Vector3& pos);
	list    GetCollisionsForUnit(const GameUnit& unit);
	list    GetCollisionsForCast(const SpellCast& cast);
	object  GetSpellInfo(const char* label);
	bool    StartChannel(GameSpell* spell);
	bool    EndChannel(GameSpell* spell, const Vector3* targetLocation);
	bool    CastSpell(GameSpell* spell, const Vector3* targetLocation);
	object  PredictCastPoint(const GameUnit& caster, const GameUnit& target, const GameSpell* info);
	object  GetObjectWithNetworkId(int netId);

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
	bool    IsUnderTower(const GameUnit& obj);

	void    DrawHpBarDamageIndicator(const GameChampion& champ, float dmg, ImVec4 color);
	void    DrawRectWorld(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4, float thickness, const ImVec4& color);
	void    DrawTriangleWorld(const Vector3& p1, const Vector3& p2, const Vector3& p3, float thickness, const ImVec4& color);
	void    DrawTriangleWorldFilled(const Vector3& p1, const Vector3& p2, const Vector3& p3, const ImVec4& color);
	void    DrawCircle(const Vector2& center, float radius, int numPoints, float thickness, const ImVec4& color);
	void    DrawCircleFilled(const Vector2& center, float radius, int numPoints, const ImVec4& color);
	void    DrawCircleWorld(const Vector3& center, float radius, int numPoints, float thickness, const ImVec4& color);
	void    DrawCircleWorldFilled(const Vector3& center, float radius, int numPoints, const ImVec4& color);
	void    DrawLine(const Vector2& start, const Vector2& end, float thickness, const ImVec4& color);
	void    DrawLineWorld(const Vector3 & start, const Vector3 & end, float thickness, const ImVec4 & color);
	void    DrawImage(const char* img, const Vector2& start, const Vector2& size, const ImVec4& color);
	void    DrawImageUVs(const char* img, const Vector2& start, const Vector2& size, const Vector2& uv1, const Vector2& uv2, const ImVec4& color);
	void    DrawImageRounded(const char* img, const Vector2& start, const Vector2& size, const ImVec4& color, float rounding);
	void    DrawImageWorld(const char * img, const Vector3 & pos, const Vector2 & size, const ImVec4 & color);
	void    DrawImageWorldPoints(const char * img, const Vector3 & p1, const Vector3 & p2, const Vector3 & p3, const Vector3 & p4, const ImVec4 & color);
	void    DrawTxt(const Vector2& pos, const char* text, const ImVec4& color);
	BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(DrawRectOverloads, DrawRect, 3, 5);
	void    DrawRect(const Vector2& start, const Vector2& size, const ImVec4& color, float rounding = 0, float thickness = 1.0);
	BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(DrawRectFilledOverloads, DrawRectFilled, 3, 4);
	void    DrawRectFilled(const Vector2& start, const Vector2& size, const ImVec4& color, float rounding = 0);
	void    Pill(const char* text, const ImVec4& colText, const ImVec4& colRect);

private:
	object  Query(QueryKey key);
	void    PressKeyAt(HKey key, const Vector3& location);
};