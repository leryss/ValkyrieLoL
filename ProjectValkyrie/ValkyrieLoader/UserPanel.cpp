#include "UserPanel.h"
#include "AsyncInjector.h"
#include "AsyncUpdater.h"

#include <fstream>

void UserPanel::Draw(ValkyrieLoader& loader)
{
	if (performUpdate && !taskPool->IsExecuting(trackIdCheckVersion)) {
		taskPool->DispatchTask(
			trackIdCheckVersion,
			api->GetCheatS3Object("valkyrie-releases-eu-north-1", loader.loggedUser.level == USER_LEVEL_TESTER ? "latest-beta.zip" : "latest.zip"),

			[this, &loader](std::shared_ptr<AsyncTask> response) {
			taskPool->DispatchTask(
				trackIdUpdate,
				std::shared_ptr<AsyncTask>((AsyncTask*)new AsyncUpdater(loader, std::static_pointer_cast<GetS3ObjectAsync>(response))),
				[this, &loader](std::shared_ptr<AsyncTask> response) {

					updateComplete = true;
			}
			);
		}
		);
		performUpdate = false;
	}

	/// Greeting
	if (ImGui::Begin("Valkyrie", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		
		this->loader = &loader;
		ImGui::BeginTabBar("UserPanelTabs");

		if (ImGui::BeginTabItem("Homepage")) {

			DrawHome();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Script Market")) {

			DrawScriptRepo();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
		ImGui::End();
	}
}

void UserPanel::DrawHome()
{
	ImGui::Text("Welcome %s !", loader->loggedUser.name.c_str());

	float days = (loader->loggedUser.expiry - duration_cast<seconds>(system_clock::now().time_since_epoch()).count()) / Constants::ONE_DAY_SECS;
	float hours = (days - int(days)) * 24.f;

	ImGui::TextColored((days < 5.f ? Color::YELLOW : Color::GREEN), "Your subscription will expire in %d days %d hours", int(days), int(hours));

	/// Change log
	if (changeLog.size() > 0) {
		ImGui::Separator();
		ImGui::TextColored(Color::PURPLE, "** Change Log **");
		ImGui::BeginChildFrame(10000, ImVec2(400.f, 200.f), ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::Text(changeLog.c_str());
		ImGui::EndChildFrame();
	}

	/// Inject stuff
	ImGui::Separator();
	ImGui::Checkbox("Auto inject", &autoInject);
	if ((injectorTask == nullptr || injectorTask->GetStatus() != ASYNC_RUNNING) && updateComplete) {
		if (autoInject) {
			injectorTask = std::shared_ptr<AsyncInjector>(new AsyncInjector(loader->GetDllPath(), false));
			taskPool->DispatchTask(
				trackIdInjector,
				injectorTask,
				[](std::shared_ptr<AsyncTask> response) {}
			);
		}
		else if (ImGui::Button("Inject")) {
			injectorTask = std::shared_ptr<AsyncInjector>(new AsyncInjector(loader->GetDllPath(), true));
			taskPool->DispatchTask(
				trackIdInjector,
				injectorTask,
				[](std::shared_ptr<AsyncTask> response) {}
			);
		}
	}

	if (!autoInject && injectorTask != nullptr && injectorTask->GetStatus() == ASYNC_RUNNING)
		injectorTask->shouldStop = true;
}

void UserPanel::DrawScriptRepo()
{
	if (retrieveScripts) {
		
		taskPool->DispatchTask(
			trackIdGetScripts,
			api->GetScriptList(loader->identity),
			[this](std::shared_ptr<AsyncTask> response) {
				scripts = ((ScriptListAsync*)response.get())->scripts;
			}
		);
		retrieveScripts = false;
	}

	if (taskPool->IsExecuting(trackIdGetScripts)) {
		ImGui::TextColored(Color::YELLOW, "Retrieving scripts...");
		return;
	}

	ImGui::BeginTable("Scripts", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable);
	ImGui::TableSetupColumn("id");
	ImGui::TableSetupColumn("name");
	ImGui::TableSetupColumn("author");
	ImGui::TableSetupColumn("champion");
	ImGui::TableSetupColumn("description");
	ImGui::TableSetupColumn("actions");
	ImGui::TableHeadersRow();

	for (size_t i = 0; i < scripts.size(); ++i) {
		
		auto& script = scripts[i];

		ImGui::PushStyleColor(ImGuiCol_TableRowBg, Color::DARK_GREEN);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s.py", script.id.c_str());

		ImGui::TableSetColumnIndex(1);
		ImGui::Text(script.name.c_str());

		ImGui::TableSetColumnIndex(2);
		ImGui::TextColored((script.author == "TeamValkyrie" ? Color::PURPLE : Color::WHITE), script.author.c_str());

		ImGui::TableSetColumnIndex(3);
		ImGui::Text(script.champion.c_str());

		ImGui::TableSetColumnIndex(4);
		ImGui::Text(script.description.c_str());

		ImGui::PopStyleColor();
	}

	ImGui::EndTable();
}
