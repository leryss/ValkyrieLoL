#include "UserPanel.h"
#include "AsyncInjector.h"
#include "AsyncCheatUpdater.h"
#include "Paths.h"

#include <fstream>

UserPanel::UserPanel()
{
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
				std::shared_ptr<AsyncTask>((AsyncTask*)new AsyncCheatUpdater(loader, std::static_pointer_cast<GetS3ObjectAsync>(response))),
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

		this->loader = &loader;
		if (ImGui::BeginTabBar("UserPanelTabBar")) {

			if (ImGui::BeginTabItem("Homepage")) {

				DrawHome();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Scripts")) {

				DrawScriptRepo();
				ImGui::EndTabItem();

			}

			if (ImGui::BeginTabItem("Changelog")) {

				ImGui::Text(changeLog.c_str());
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
		ImGui::End();
	}
}

void UserPanel::DrawHome()
{
	ImGui::Text("Welcome %s !", loader->loggedUser.name.c_str());

	float days = (loader->loggedUser.expiry - duration_cast<seconds>(system_clock::now().time_since_epoch()).count()) / Constants::ONE_DAY_SECS;
	float hours = (days - int(days)) * 24.f;

	ImGui::TextColored((days < 5.f ? Color::YELLOW : Color::GREEN), "Your subscription will expire in %d days %d hours", int(days), int(hours));

	if (ImGui::Button("Force Update")) {
		loader->cheatVersionHash = "";
		performUpdate = true;
		updateComplete = false;
	}

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
	if (loadScriptRepo) {
		scriptRepo.LoadRemoteEntries(loader->identity);
		scriptRepo.LoadLocalEntries(Paths::ScriptsIndex);
		loadScriptRepo = false;
	}

	if (scriptRepo.localsUnsaved)
		scriptRepo.SaveLocalEntries(Paths::ScriptsIndex);

	scriptRepo.Draw();
}
