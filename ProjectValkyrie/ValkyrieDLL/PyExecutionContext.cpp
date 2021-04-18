#include "PyExecutionContext.h"
#include "Debug.h"
#include "GameKeybind.h"
#include "Script.h"

template <class T>
std::shared_ptr<list> MakePyList(std::vector<std::shared_ptr<T>>& cList) {

	std::shared_ptr<list> pyList = std::shared_ptr<list>(new list());

	for (auto& v : cList) {
		pyList->append(ptr(v.get()));
	}

	return pyList;
}

bool PyExecutionContext::IsKeyDown(int key)
{
	if (state->hud.isChatOpen)
		return false;

	return currentScript->input.IsDown((HKey)key);
}

bool PyExecutionContext::WasKeyPressed(int key)
{
	if (state->hud.isChatOpen)
		return false;

	return currentScript->input.WasPressed((HKey)key);
}

void PyExecutionContext::PingDanger(const Vector3 & position)
{
	if (!state->hud.isChatOpen) {
		PressKeyAt(GameKeybind::PingDanger, position);
	}
}

void PyExecutionContext::PingVision(const Vector3 & position)
{
	if (!state->hud.isChatOpen) {
		PressKeyAt(GameKeybind::PingVision, position);
	}
}

void PyExecutionContext::PingMia(const Vector3 & position)
{
	if (!state->hud.isChatOpen) {
		PressKeyAt(GameKeybind::PingMia, position);
	}
}

void PyExecutionContext::PingOmw(const Vector3 & position)
{
	if (!state->hud.isChatOpen) {
		PressKeyAt(GameKeybind::PingOmw, position);
	}
}

void PyExecutionContext::PingAssist(const Vector3 & position)
{
	if (!state->hud.isChatOpen) {
		PressKeyAt(GameKeybind::PingAssist, position);
	}
}

object PyExecutionContext::Raycast(const Vector3 & begin, const Vector3 & dir, float length, float halfWidth, RaycastLayer layers)
{
	std::shared_ptr<RaycastResult> result = Raycast::Cast(state, begin, dir, length, halfWidth, layers);
	if(result != nullptr)
		return object(*result);
	return object();
}

bool PyExecutionContext::IsWallAt(const Vector3 & pos)
{
	return GameData::IsWallAt(pos);
}

list PyExecutionContext::GetCollisionsForUnit(const GameUnit & unit)
{
	return collisionEngine.GetCollisionsForUnit(&unit);
}

list PyExecutionContext::GetCollisionsForCast(const SpellCast & cast)
{
	return collisionEngine.GetCollisionsForCast(&cast);
}

object PyExecutionContext::GetSpellInfo(const char* label) {
	std::string labelStr(label);
	return object(ptr(GameData::GetSpell(labelStr)));
}

bool PyExecutionContext::StartChannel(GameSpell * spell)
{
	if (state->hud.WasChatOpenMillisAgo(100))
		return false;

	/// Check if castable
	if (spell->lvl == 0 || spell->GetRemainingCooldown() > 0.01)
		return false;

	currentScript->input.IssueHoldKey(spell->castKey);
}

bool PyExecutionContext::EndChannel(GameSpell * spell, const Vector3 * targetLocation)
{
	return CastSpell(spell, targetLocation);
}

bool PyExecutionContext::CastSpell(GameSpell* spell, const Vector3* targetLocation) {
	
	DBG_INFO("Casting spell %s", spell->name.c_str())
	if (state->hud.WasChatOpenMillisAgo(100))
		return false;

	auto now = steady_clock::now();
	duration<float, std::milli> diff = now - spell->lastCastTimestamp;
	if (diff.count() < 100.f)
		return false;

	/// Check if castable
	if (!state->player->CanCast(spell))
		return false;

	if (targetLocation != nullptr) {
		Vector2 screenPos = state->renderer.WorldToScreen(*targetLocation);
		currentScript->input.IssuePressKeyAt(spell->castKey, [screenPos] { return screenPos; });
	}
	else
		currentScript->input.IssuePressKey(spell->castKey);
	
	spell->lastCastTimestamp = steady_clock::now();

	return true;
}


object PyExecutionContext::PredictCastPoint(const GameUnit & caster, const GameUnit& target, const GameSpell * info)
{
	if (info->staticData == nullptr) {
		return object();
	}

	Vector3 point;

	if (!collisionEngine.PredictPointForCollision(caster, target, *info->staticData, point))
		return object();

	return object(point);
}

void PyExecutionContext::MoveToMouse() {
	if (state->hud.WasChatOpenMillisAgo(100))
		return;

	currentScript->input.IssueHoldKey(GameKeybind::TargetChampionsOnly);
	currentScript->input.IssueClick(CT_RIGHT_CLICK);
	currentScript->input.IssueUnholdKey(GameKeybind::TargetChampionsOnly);
}

void PyExecutionContext::MoveToLocation(const Vector3 & location)
{
	if (state->hud.WasChatOpenMillisAgo(100))
		return;
	
	Vector2 screenPos = state->renderer.WorldToScreen(location);
	currentScript->input.IssueHoldKey(GameKeybind::TargetChampionsOnly);
	currentScript->input.IssueClickAt(CT_RIGHT_CLICK, [screenPos] { return screenPos; });
	currentScript->input.IssueUnholdKey(GameKeybind::TargetChampionsOnly);
}

void PyExecutionContext::MoveMouse(const Vector3 & worldLocation)
{
	currentScript->input.SetMouseCursor(World2Screen(worldLocation));
}

void PyExecutionContext::AttackUnit(const GameUnit & unit)
{
	if (state->hud.WasChatOpenMillisAgo(100))
		return;
	currentScript->input.IssueClickUnit(CT_RIGHT_CLICK, unit);
}

void PyExecutionContext::PingNormal(const Vector3 & position)
{
	if (!state->hud.isChatOpen) {
		Vector2 screenPos = state->renderer.WorldToScreen(position);
		currentScript->input.IssuePressKey(GameKeybind::PingNormal);
		currentScript->input.IssueClickAt(CT_LEFT_CLICK, [screenPos] { return screenPos; });
	}
}

void PyExecutionContext::PingWarn(const Vector3 & position)
{
	if (!state->hud.isChatOpen) {
		Vector2 screenPos = state->renderer.WorldToScreen(position);
		currentScript->input.IssuePressKey(GameKeybind::PingWarn);
		currentScript->input.IssueClickAt(CT_LEFT_CLICK, [screenPos] { return screenPos; });
	}
}

void PyExecutionContext::LogInfo(const char * msg)
{
	Logger::Info("[%s] %s", currentScript->info->id.c_str(), (msg != NULL ? msg : "NULL"));
}

void PyExecutionContext::LogWarning(const char * msg)
{
	Logger::Warn("[%s] %s", currentScript->info->id.c_str(), (msg != NULL ? msg : "NULL"));
}

void PyExecutionContext::LogError(const char * msg)
{
	Logger::Error("[%s] %s", currentScript->info->id.c_str(), (msg != NULL ? msg : "NULL"));
}

Vector2 PyExecutionContext::World2Screen(const Vector3 & world)
{
	return state->renderer.WorldToScreen(world);
}

Vector2 PyExecutionContext::World2Minimap(const Vector3 & world)
{
	return state->renderer.WorldToMinimap(world, state->hud.minimapPosition, state->hud.minimapSize);
}

float PyExecutionContext::DistanceOnMinimap(float dist)
{
	return state->renderer.DistanceToMinimap(dist, state->hud.minimapSize);
}

bool PyExecutionContext::IsScreenPointOnScreen(const Vector2 & point, float offsetX, float offsetY)
{
	return state->renderer.IsScreenPointOnScreen(point, offsetX, offsetY);
}

bool PyExecutionContext::IsWorldPointOnScreen(const Vector3 & point, float offsetX, float offsetY)
{
	return state->renderer.IsWorldPointOnScreen(point, offsetX, offsetY);
}

bool PyExecutionContext::IsInFountain(const GameObject & obj)
{
	for (auto& turret : state->turrets) {
		if (turret->IsAllyTo(obj) && turret->health > 9000.f && turret->pos.distance(obj.pos) < turret->staticData->baseAttackRange)
			return true;
	}
	return false;
}

bool PyExecutionContext::IsUnderTower(const GameUnit & obj)
{
	for (auto& turret : state->turrets) {
		if (turret->IsAllyTo(obj) && (turret->pos.distance(obj.pos) - obj.staticData->gameplayRadius) < turret->staticData->baseAttackRange)
			return true;
	}
	return false;
}

PyExecutionContext::PyExecutionContext()
{
}

object PyExecutionContext::GetImGuiInterface()
{
	return object(boost::ref(imgui));
}

object PyExecutionContext::GetChampions()
{
	return Query(QKEY_CHAMP);
}

object PyExecutionContext::GetMissiles()
{
	return Query(QKEY_MISSILE);
}

object PyExecutionContext::GetJungle()
{
	return Query(QKEY_JUNGLE);
}

object PyExecutionContext::GetMinions()
{
	return Query(QKEY_MINION);
}

object PyExecutionContext::GetTurrets()
{
	return Query(QKEY_TURRET);
}

object PyExecutionContext::GetOthers()
{
	return Query(QKEY_OTHERS);
}

object PyExecutionContext::GetConfig()
{
	return object(boost::ref(currentScript->config));
}

Vector2 PyExecutionContext::GetMousePosition()
{
	return currentScript->input.GetMouseCursor();
}

void PyExecutionContext::SetScript(Script * script)
{
	this->currentScript = script;
}

void PyExecutionContext::SetGameState(GameState * state)
{
	DBG_INFO("PyExecutionContext::SetGameState")
	this->state = state;

	pillPosition = state->renderer.WorldToScreen(state->player->pos);

	ping          = state->ping;
	time          = state->time;
	hovered       = object(ptr(state->hovered.get()));
	player        = object(ptr(state->player.get()));
	queryEnginePy = object(ptr(&queryEngine));
	selfPy        = object(ptr(this));
	queryEngine.Update(state);
	collisionEngine.Update(*state);
}

void PyExecutionContext::SetImGuiOverlay(ImDrawList * overlay)
{
	this->overlay = overlay;
}

void PyExecutionContext::DrawHpBarDamageIndicator(const GameChampion & champ, float dmg, ImVec4 color)
{
	Vector2 hpbar_pos = champ.GetHpBarPosition();
	Vector2 pos_start = hpbar_pos.add(Vector2(24.0, -18.0));
	Vector2 pos_end = pos_start.add(Vector2((champ.health / champ.maxHealth) * 105.0, 0.0));
	Vector2 pos_start2 = pos_end.add(Vector2(-105.0 * dmg / champ.maxHealth, 0.0));
	if (pos_start2.x < pos_start.x)
		pos_start2.x = pos_start.x;
	DrawLine(pos_start2, pos_end, 12.0, color);
}

void PyExecutionContext::DrawRectWorld(const Vector3 & p1, const Vector3 & p2, const Vector3 & p3, const Vector3 & p4, float thickness, const ImVec4 & color)
{
	static Vector2 points[4];
	points[0] = state->renderer.WorldToScreen(p1);
	points[1] = state->renderer.WorldToScreen(p2);
	points[2] = state->renderer.WorldToScreen(p3);
	points[3] = state->renderer.WorldToScreen(p4);

	overlay->AddPolyline((ImVec2*)points, 4, ImColor(color), true, thickness);
}

void PyExecutionContext::DrawTriangleWorld(const Vector3 & p1, const Vector3 & p2, const Vector3 & p3, float thickness, const ImVec4 & color)
{
	overlay->AddTriangle(
		(ImVec2&)state->renderer.WorldToScreen(p1),
		(ImVec2&)state->renderer.WorldToScreen(p2),
		(ImVec2&)state->renderer.WorldToScreen(p3), ImColor(color));
}

void PyExecutionContext::DrawTriangleWorldFilled(const Vector3 & p1, const Vector3 & p2, const Vector3 & p3, const ImVec4 & color)
{
	overlay->AddTriangleFilled(
		(ImVec2&)state->renderer.WorldToScreen(p1),
		(ImVec2&)state->renderer.WorldToScreen(p2),
		(ImVec2&)state->renderer.WorldToScreen(p3), ImColor(color));
}

void PyExecutionContext::DrawCircle(const Vector2 & center, float radius, int numPoints, float thickness, const ImVec4 & color)
{
	overlay->AddCircle(ImVec2(center.x, center.y), radius, ImColor(color), numPoints, thickness);
}

void PyExecutionContext::DrawCircleFilled(const Vector2 & center, float radius, int numPoints, const ImVec4 & color)
{
	overlay->AddCircleFilled(ImVec2(center.x, center.y), radius, ImColor(color), numPoints);
}

void PyExecutionContext::DrawCircleWorld(const Vector3 & center, float radius, int numPoints, float thickness, const ImVec4 & color)
{
	state->renderer.DrawCircleAt(overlay, center, radius, numPoints, ImColor(color), thickness);
}

void PyExecutionContext::DrawCircleWorldFilled(const Vector3 & center, float radius, int numPoints, const ImVec4 & color)
{
	state->renderer.DrawCircleAtFilled(overlay, center, radius, numPoints, ImColor(color));
}

void PyExecutionContext::DrawLine(const Vector2 & start, const Vector2 & end, float thickness, const ImVec4 & color)
{
	overlay->AddLine((const ImVec2&)start, (const ImVec2&)end, ImColor(color), thickness);
}

void PyExecutionContext::DrawLineWorld(const Vector3 & start, const Vector3 & end, float thickness, const ImVec4 & color)
{
	overlay->AddLine(
		(ImVec2&)state->renderer.WorldToScreen(start), 
		(ImVec2&)state->renderer.WorldToScreen(end), ImColor(color), thickness);
}

void PyExecutionContext::DrawImage(const char * img, const Vector2 & start, const Vector2 & size, const ImVec4 & color)
{
	static const ImVec2 zero = ImVec2(0.f, 0.f);
	static const ImVec2 one = ImVec2(1.f, 1.f);

	std::string imageName(img);

	overlay->AddImage(GameData::GetImage(imageName), ImVec2(start.x - size.x / 2.f, start.y - size.y / 2.f), ImVec2(start.x + size.x / 2.f, start.y + size.y / 2.f), zero, one, ImColor(color));
}

/*void PyExecutionContext::DrawImageWorld(const char * img, const Vector3& origin, const Vector2& size, const Vector2& uv1, const Vector2& uv2, const Vector2& uv3, const Vector2& uv4, const ImVec4& color) {
	
	//static const ImVec2 uv1 = ImVec2(0.f, 0.f);
	//static const ImVec2 uv2 = ImVec2(1.f, 0.f);
	//static const ImVec2 uv3 = ImVec2(1.f, 1.f);
	//static const ImVec2 uv4 = ImVec2(0.f, 1.f);

	std::string imageName(img);

	Vector2 p1 = state->renderer.WorldToScreen(origin.add(Vector3(-size.x, 0.f, size.y)));
	Vector2 p2 = state->renderer.WorldToScreen(origin.add(Vector3(size.x, 0.f, size.y)));
	Vector2 p3 = state->renderer.WorldToScreen(origin.add(Vector3(size.x, 0.f, -size.y)));
	Vector2 p4 = state->renderer.WorldToScreen(origin.add(Vector3(-size.x, 0.f, -size.y)));

	Vector2 v[4] = { p1, p2, p3, p4 };
	overlay->AddPolyline((ImVec2*)v, 4, ImColor(Color::WHITE), true, 2.f);

	overlay->AddImageQuad(
		GameData::GetImage(imageName), 
		(ImVec2&)p1, (ImVec2&)p2, (ImVec2&)p3, (ImVec2&)p4, 
		(ImVec2&)uv1, (ImVec2&)uv2, (ImVec2&)uv3, (ImVec2&)uv4,
		ImColor(color));

}*/

void PyExecutionContext::DrawImageRounded(const char * img, const Vector2 & start, const Vector2 & size, const ImVec4 & color, float rounding)
{
	static const ImVec2 zero = ImVec2(0.f, 0.f);
	static const ImVec2 one = ImVec2(1.f, 1.f);

	std::string imageName(img);
	overlay->AddImageRounded(GameData::GetImage(imageName), ImVec2(start.x - size.x/2.f, start.y - size.y/2.f), ImVec2(start.x + size.x/2.f, start.y + size.y/2.f), zero, one, ImColor(color), rounding);
}

void PyExecutionContext::DrawTxt(const Vector2 & pos, const char * text, const ImVec4 & color)
{
	ImVec2 size = ImGui::CalcTextSize(text);
	overlay->AddText(ImVec2(pos.x - size.x/2.f, pos.y - size.y/2.f), ImColor(color), text);
}

void PyExecutionContext::DrawRect(const Vector2& start, const Vector2& size, const ImVec4 & color, float rounding, float thickness)
{
	overlay->AddRect((ImVec2&)start, (ImVec2&)Vector2(start.x + size.x, start.y + size.y), ImColor(color), rounding, 15, thickness);
}

void PyExecutionContext::DrawRectFilled(const Vector2& start, const Vector2& size, const ImVec4 & color, float rounding)
{
	overlay->AddRectFilled((ImVec2&)start, (ImVec2&)Vector2(start.x + size.x, start.y + size.y), ImColor(color), rounding);
}

void PyExecutionContext::Pill(const char * text, const ImVec4 & colText, const ImVec4 & colRect)
{
	ImVec2 size = ImGui::CalcTextSize(text);
	size.x += 10.f;
	size.y += 6.f;
	pillPosition.y += size.y + 4.f;

	overlay->AddRectFilled((ImVec2&)pillPosition, ImVec2(pillPosition.x + size.x, pillPosition.y + size.y), ImColor(colRect), 10.f);
	DrawTxt(Vector2(pillPosition.x + size.x / 2.f, pillPosition.y + size.y / 2.f), text, colText);
}

object PyExecutionContext::Query(QueryKey key)
{
	queryEngine.NewQuery(key);
	return queryEnginePy;
}

void PyExecutionContext::PressKeyAt(HKey key, const Vector3 & location)
{
	Vector2 screenPos = state->renderer.WorldToScreen(location);
	currentScript->input.IssuePressKeyAt(key, [screenPos] { return screenPos; });
}

