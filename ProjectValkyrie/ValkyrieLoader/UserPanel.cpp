#include "UserPanel.h"
#include "AsyncInjector.h"
#include "AsyncUpdater.h"

#include <fstream>

UserPanel::UserPanel()
{
	remoteScripts.local = &localScripts;
	localScripts.remote = &remoteScripts;
}

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
	if (ImGui::Begin("Valkyrie")) {

		localScripts.PerformQueued();
		remoteScripts.PerformQueued();
		if (localScripts.dirty)
			localScripts.SaveToFile(Paths::ScriptsIndex.c_str());

		this->loader = &loader;
		ImGui::BeginTabBar("UserPanelTabs");

		if (ImGui::BeginTabItem("Homepage")) {

			DrawHome();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Script Manager")) {
			
			DrawScriptManager();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Script Repository")) {

			DrawScriptRepo();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Changes")) {
			
			ImGui::Text(changeLog.c_str());
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

	/// Inject stuff
	ImGui::Separator();
	ImGui::Checkbox("Auto inject", &autoInject);
	if ((injectorTask == nullptr || injectorTask->GetStatus() != ASYNC_RUNNING) && updateComplete) {
		if (autoInject) {
			injectorTask = std::shared_ptr<AsyncInjector>(new AsyncInjector(Paths::Payload, false));
			taskPool->DispatchTask(
				trackIdInjector,
				injectorTask,
				[](std::shared_ptr<AsyncTask> response) {}
			);
		}
		else if (ImGui::Button("Inject")) {
			injectorTask = std::shared_ptr<AsyncInjector>(new AsyncInjector(Paths::Payload, true));
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
	LoadScriptIndicesIfNecessary();
	remoteScripts.Draw();
}

void UserPanel::DrawScriptManager()
{
	LoadScriptIndicesIfNecessary();
	localScripts.Draw();
}

void UserPanel::LoadScriptIndicesIfNecessary()
{
	if (loadRemoteScripts) {
		remoteScripts.Load(loader->identity);
		loadRemoteScripts = false;
	}

	if (loadLocalScripts) {
		localScripts.LoadFromFile(Paths::ScriptsIndex.c_str());
		loadLocalScripts = false;
	}
}
