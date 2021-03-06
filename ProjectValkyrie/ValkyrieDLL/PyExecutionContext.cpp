#include "PyExecutionContext.h"
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

bool PyExecutionContext::CastSpell(GameSpell* spell, const Vector3& targetLocation) {
	
	auto now = steady_clock::now();
	duration<float, std::milli> diff = now - spell->lastCastTimestamp;
	if (diff.count() < 100.f)
		return false;

	/// Check if castable
	if (spell->lvl == 0 || spell->GetRemainingCooldown() > 0.01)
		return false;

	/// Check if something already casting
	if (state->player->isCasting)
		return false;

	Vector2 screenPos = state->renderer.WorldToScreen(targetLocation);
	currentScript->input.IssuePressKeyAt(spell->castKey, [screenPos] { return screenPos; });
	spell->lastCastTimestamp = steady_clock::now();

	return true;
}

void PyExecutionContext::MoveToMouse() {
	if (!state->hud.isChatOpen) {
		if (state->hovered != nullptr && state->hovered->IsEnemyTo(*state->player.get()))
			return;
		currentScript->input.IssueClick(CT_RIGHT_CLICK);
	}
}

void PyExecutionContext::MoveToLocation(const Vector3 & location)
{
	if (!state->hud.isChatOpen) {
		if (state->hovered != nullptr && state->hovered->IsEnemyTo(*state->player.get()))
			return;

		Vector2 screenPos = state->renderer.WorldToScreen(location);
		currentScript->input.IssueClickAt(CT_RIGHT_CLICK, [screenPos] { return screenPos; });
	}
}

void PyExecutionContext::AttackUnit(const GameUnit & unit)
{
	if (!state->hud.isChatOpen)
		currentScript->input.IssueClickUnit(CT_RIGHT_CLICK, unit);
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

object PyExecutionContext::GetImGuiInterface()
{
	return object(boost::ref(imgui));
}

object PyExecutionContext::GetChampions()
{
	return object(boost::ref(*champs));
}

object PyExecutionContext::GetMissiles()
{
	return object(boost::ref(*missiles));
}

object PyExecutionContext::GetJungle()
{
	return object(boost::ref(*jungle));
}

object PyExecutionContext::GetMinions()
{
	return object(boost::ref(*minions));
}

object PyExecutionContext::GetTurrets()
{
	return object(boost::ref(*turrets));
}

object PyExecutionContext::GetOthers()
{
	return object(boost::ref(*others));
}

object PyExecutionContext::GetConfig()
{
	return object(boost::ref(currentScript->config));
}

void PyExecutionContext::SetScript(Script * script)
{
	this->currentScript = script;
}

void PyExecutionContext::SetGameState(GameState * state)
{
	this->state = state;

	pillPosition = state->renderer.WorldToScreen(state->player->pos);

	ping     = state->ping;
	time     = state->time;
	hovered  = object(ptr(state->hovered.get()));
	player   = object(ptr(state->player.get()));

	champs   = MakePyList(state->champions);
	minions  = MakePyList(state->minions);
	turrets  = MakePyList(state->turrets);
	jungle   = MakePyList(state->jungle);
	missiles = MakePyList(state->missiles);
	others   = MakePyList(state->others);

	collisionEngine.Update(*state);
}

void PyExecutionContext::SetImGuiOverlay(ImDrawList * overlay)
{
	this->overlay = overlay;
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
	static ImVec2 zero = ImVec2(0.f, 0.f);
	static ImVec2 one = ImVec2(1.f, 1.f);

	std::string imageName(img);

	overlay->AddImage(GameData::GetImage(imageName), ImVec2(start.x - size.x / 2.f, start.y - size.y / 2.f), ImVec2(start.x + size.x / 2.f, start.y + size.y / 2.f), zero, one, ImColor(color));
}

void PyExecutionContext::DrawImageRounded(const char * img, const Vector2 & start, const Vector2 & size, const ImVec4 & color, float rounding)
{
	static ImVec2 zero = ImVec2(0.f, 0.f);
	static ImVec2 one = ImVec2(1.f, 1.f);

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

