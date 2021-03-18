#include "ExtendSubscriptionPanel.h"
#include "ValkyrieLoader.h"

void ExtendSubscriptionPanel::Draw(ValkyrieLoader & loader)
{
	ImGui::Begin("Extend Subscription");

	ImGui::InputText("Account Name", name, Constants::INPUT_TEXT_SIZE);
	ImGui::InputText("Subscription Code", code, Constants::INPUT_TEXT_SIZE);

	if (ImGui::Button("Extend")) {
		taskPool->DispatchTask(
			std::string("Extend Subscription"),
			api->ExtendSubscription(name, code),
			[this, &loader](std::shared_ptr<AsyncTask> response) {
			
				loader.loggedUser.expiry = std::atof(((StringResultAsync*)response.get())->result.c_str());
				loader.ChangeToPreviousPanel();
			}
		);
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel")) {
		loader.ChangeToPreviousPanel();
	}

	ImGui::End();
}
