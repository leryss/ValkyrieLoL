#pragma once

#include "imgui/imgui.h"
#include "AsyncTask.h"
#include "Color.h"

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <map>

#include <windows.h>

/// Simple task executioner ive slapped together in 30 mins
class AsyncTaskPool {

public:

	~AsyncTaskPool() {
		stopThreads = true;
		for (auto& thread : workerThreads)
			thread->join();
	}

	void AddWorkers(int numWorkers) {
		for (int i = 0; i < numWorkers; ++i) {
			workerThreads.push_back(std::unique_ptr<std::thread>(new std::thread(&AsyncTaskPool::TaskWorkerLoop, this)));
		}
	}

	void DispatchTask(std::string taskId, std::shared_ptr<AsyncTask> task, std::function<void(std::shared_ptr<AsyncTask>)> onSuccess) {
		task->onSuccess = onSuccess;
		task->SetStatus(ASYNC_NOT_STARTED);
		task->taskId = taskId;

		mtxTasks.lock();
		doneTasks.erase(taskId);
;		waitingTasks[task->taskId] = task;
		mtxTasks.unlock();
	}

	bool IsExecuting(std::string taskId) {
		auto find = runningTasks.find(taskId);
		if (find == runningTasks.end())
			return false;

		return find->second->GetStatus() == ASYNC_RUNNING;
	}

	void ImGuiDraw() {

		if (runningTasks.empty() && doneTasks.empty())
			return;

		ImGui::SetNextWindowPos(ImVec2(10, 10));
		ImGui::Begin("Tasks", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

		ImGui::TextColored(Color::YELLOW, "Valkyrie");

		for (auto& pair : runningTasks) {
			const char* operationName = pair.first.c_str();
			auto& task = pair.second;

			ImGui::Separator();
			ImGui::TextColored(Color::PURPLE, operationName);
			ImGui::TextColored(Color::GREEN, "%s", task->currentStep.c_str());
			if (task->percentDone >= 0.f) {
				ImGui::ProgressBar(task->percentDone);
			}
		}

		for (auto& pair : doneTasks) {
			const char* operationName = pair.first.c_str();
			auto& task = pair.second;

			if (task->GetStatus() == ASYNC_FAILED) {
				ImGui::Separator();
				ImGui::TextColored(Color::PURPLE, operationName);
				ImGui::TextColored(Color::RED, "%s failed: %s", task->currentStep.c_str(), task->error.c_str());
			}
		}

		ImGui::End();
	}

private:
	void TaskWorkerLoop() {

		while (!stopThreads) {

			/// Find first task that is not started
			std::shared_ptr<AsyncTask> task = nullptr;

			mtxTasks.lock();
			for (auto& pair : waitingTasks) {
				if (pair.second->GetStatus() == ASYNC_NOT_STARTED) {
					task = pair.second;
					task->SetStatus(ASYNC_RUNNING);
					break;
				}
			}
			mtxTasks.unlock();
				
			if (task != nullptr) {
				mtxTasks.lock();
				waitingTasks.erase(task->taskId);
				runningTasks[task->taskId] = task;
				mtxTasks.unlock();

				try {
					task->Perform();
				}
				catch (std::exception& exc) {
					task->SetError(exc.what());
					break;
				}

				mtxTasks.lock();
				runningTasks.erase(task->taskId);
				doneTasks[task->taskId] = task;
				mtxTasks.unlock();

				if (task->GetStatus() == ASYNC_SUCCEEDED) {
					try {
						task->onSuccess(task);
					}
					catch (std::exception& exc) {
						task->SetError(exc.what());
						break;
					}
				}
			}

			Sleep(10);
		}
	}

	std::vector<std::unique_ptr<std::thread>>            workerThreads;
	
	std::mutex                                           mtxTasks;

	std::map<std::string, std::shared_ptr<AsyncTask>>    waitingTasks;
	std::map<std::string, std::shared_ptr<AsyncTask>>    runningTasks;
	std::map<std::string, std::shared_ptr<AsyncTask>>    doneTasks;

	bool                                                 stopThreads = false;
};