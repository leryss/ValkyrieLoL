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

		auto onGetObj = [this, &loader](std::shared_ptr<AsyncTask> response) {
			auto s3Obj = std::static_pointer_cast<GetS3ObjectAsync>(response);

			taskPool->DispatchTask(
				trackIdUpdate,
				std::shared_ptr<AsyncTask>((AsyncTask*)new AsyncCheatUpdater(s3Obj)),
				[this, &loader, s3Obj](std::shared_ptr<AsyncTask> response) {
					ReadChangeLog();
					loader.cheatVersionHash = s3Obj->result.GetETag().c_str();
					updateComplete = true;
				}
			); 
		};

		auto onGetHead = [this, onGetObj, &loader](std::shared_ptr<AsyncTask> response) {
			auto s3Head = std::static_pointer_cast<GetS3ObjectHeadResultAsync>(response);
			if (s3Head->result.GetETag() == loader.cheatVersionHash.c_str()) {
				ReadChangeLog();
				updateComplete = true;
			}
			else {
				taskPool->DispatchTask(
					trackIdCheckVersion,
					api->GetCheatS3Object(Constants::S3_BUCKET, loader.loggedUser.level == USER_LEVEL_TESTER ? Constants::S3_DATA_BETA_KEY : Constants::S3_DATA_KEY),
					onGetObj
				);
			}
		};

		taskPool->DispatchTask(
			trackIdCheckVersion,
			api->GetS3ObjectHead(Constants::S3_BUCKET, loader.loggedUser.level == USER_LEVEL_TESTER ? Constants::S3_DATA_BETA_KEY : Constants::S3_DATA_KEY),
			onGetHead
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
	ImGui::SameLine();
	if (ImGui::Button("Extend Subscription")) {
		strcpy_s(loader->extendSubPanel.name, loader->identity.name.c_str());
		loader->ChangePanel(&loader->extendSubPanel);
	}

	/// Inject stuff
	ImGui::Separator();
	ImGui::Checkbox("Auto Login", &loader->loginPanel.autoLogin);
	ImGui::Checkbox("Auto inject", &autoInject);
	if ((injectorTask == nullptr || injectorTask->GetStatus() != ASYNC_RUNNING) && updateComplete) {
		if (autoInject) {
			injectorTask = std::shared_ptr<AsyncInjector>(new AsyncInjector(Paths::Payload, false, 10000));
			taskPool->DispatchTask(
				trackIdInjector,
				injectorTask,
				[](std::shared_ptr<AsyncTask> response) {}
			);
		}
		else if (ImGui::Button("Inject")) {
			injectorTask = std::shared_ptr<AsyncInjector>(new AsyncInjector(Paths::Payload, true, 1000));
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

void UserPanel::ReadChangeLog()
{
	std::ifstream changeLogFile(Paths::ChangeLog);
	changeLog = std::string((std::istreambuf_iterator<char>(changeLogFile)), std::istreambuf_iterator<char>());
}
