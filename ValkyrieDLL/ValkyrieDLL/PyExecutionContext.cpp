#include "PyExecutionContext.h"

object PyExecutionContext::GetImGuiInterface()
{
	return object(boost::ref(imgui));
}

void PyExecutionContext::SetGameState(GameState * state)
{
	this->state = state;
}

void PyExecutionContext::SetImGuiOverlay(ImDrawList * overlay)
{
	this->overlay = overlay;
}

void PyExecutionContext::Log(const char * msg)
{
	Logger::Console.Log(msg);
}
