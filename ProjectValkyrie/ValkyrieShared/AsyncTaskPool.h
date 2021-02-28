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

	                      ~AsyncTaskPool();
	void                  AddWorkers(int numWorkers);
	void                  DispatchTask(std::string taskId, std::shared_ptr<AsyncTask> task, std::function<void(std::shared_ptr<AsyncTask>)> onSuccess);
	bool                  IsExecuting(std::string taskId);
	void                  ImGuiDraw();
	static AsyncTaskPool* Get();

private:
	     AsyncTaskPool();
	void TaskWorkerLoop();

	static AsyncTaskPool*                                Instance;

	std::vector<std::unique_ptr<std::thread>>            workerThreads;
	
	std::mutex                                           mtxTasks;

	std::map<std::string, std::shared_ptr<AsyncTask>>    waitingTasks;
	std::map<std::string, std::shared_ptr<AsyncTask>>    runningTasks;
	std::map<std::string, std::shared_ptr<AsyncTask>>    doneTasks;

	bool                                                 stopThreads = false;
};