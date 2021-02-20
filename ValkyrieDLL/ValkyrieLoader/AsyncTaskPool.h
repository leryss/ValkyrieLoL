#pragma once

#include "AsyncTask.h"
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <map>

#include <windows.h>

/// Simple task executioner ive slapped together in 30 mins
class AsyncTaskPool {

public:

	AsyncTaskPool(int numWorkers = 4) {
		for (int i = 0; i < numWorkers; ++i) {
			workerThreads.push_back(std::unique_ptr<std::thread>(new std::thread(&AsyncTaskPool::TaskWorkerLoop, this)));
			workerThreads.back()->detach();
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

	void VisitTasks(std::function<void(std::string taskId, std::shared_ptr<AsyncTask>& task)> visitFunction) {
		for (auto& pair : runningTasks) {
			visitFunction(pair.first, pair.second);
		}
		for (auto& pair : doneTasks) {
			visitFunction(pair.first, pair.second);
		}
	}

	bool IsExecuting(std::string taskId) {
		auto find = runningTasks.find(taskId);
		if (find == runningTasks.end())
			return false;

		return find->second->GetStatus() == ASYNC_RUNNING;
	}

private:
	void TaskWorkerLoop() {

		while (true) {

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

				task->Perform();

				mtxTasks.lock();
				runningTasks.erase(task->taskId);
				doneTasks[task->taskId] = task;
				mtxTasks.unlock();

				if (task->GetStatus() == ASYNC_SUCCEEDED)
					task->onSuccess(task);
			}

			Sleep(10);
		}
	}

	std::vector<std::unique_ptr<std::thread>> workerThreads;
	
	std::mutex                                           mtxTasks;

	std::map<std::string, std::shared_ptr<AsyncTask>>    waitingTasks;
	std::map<std::string, std::shared_ptr<AsyncTask>>    runningTasks;
	std::map<std::string, std::shared_ptr<AsyncTask>>    doneTasks;
};