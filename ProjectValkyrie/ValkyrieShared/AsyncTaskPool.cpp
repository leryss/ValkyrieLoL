#include "AsyncTaskPool.h"

AsyncTaskPool* AsyncTaskPool::Instance = nullptr;

AsyncTaskPool::~AsyncTaskPool()
{
	stopThreads = true;
	for (auto& thread : workerThreads)
		thread->join();
}

void AsyncTaskPool::AddWorkers(int numWorkers)
{
	for (int i = 0; i < numWorkers; ++i) {
		workerThreads.push_back(std::unique_ptr<std::thread>(new std::thread(&AsyncTaskPool::TaskWorkerLoop, this)));
	}
}

void AsyncTaskPool::DispatchTask(std::string taskId, std::shared_ptr<AsyncTask> task, std::function<void(std::shared_ptr<AsyncTask>)> onSuccess)
{
	task->onSuccess = onSuccess;
	task->SetStatus(ASYNC_NOT_STARTED);
	task->taskId = taskId;

	mtxTasks.lock();
	doneTasks.erase(taskId);
	waitingTasks[task->taskId] = task;
	mtxTasks.unlock();
}

bool AsyncTaskPool::IsExecuting(std::string taskId)
{
	auto find = runningTasks.find(taskId);
	if (find == runningTasks.end())
		return false;

	return find->second->GetStatus() == ASYNC_RUNNING;
}

bool AsyncTaskPool::IsWaiting(std::string taskId)
{
	auto find = waitingTasks.find(taskId);
	if (find == waitingTasks.end())
		return false;
	
	return true;
}

void AsyncTaskPool::ImGuiDraw()
{

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

AsyncTaskPool * AsyncTaskPool::Get()
{
	if (Instance == nullptr)
		Instance = new AsyncTaskPool();
	return Instance;
}

AsyncTaskPool::AsyncTaskPool()
{
}

void AsyncTaskPool::TaskWorkerLoop()
{

	while (!stopThreads) {

		
		std::shared_ptr<AsyncTask> task = nullptr;

		mtxTasks.lock();
		/// Find first task that is not started
		for (auto& pair : waitingTasks) {
			if (pair.second->GetStatus() == ASYNC_NOT_STARTED) {
				task = pair.second;
				break;
			}
		}

		/// Start waiting task
		if (task != nullptr) {
			waitingTasks.erase(task->taskId);
			runningTasks[task->taskId] = task;
			task->SetStatus(ASYNC_RUNNING);
		}
		mtxTasks.unlock();

		if (task != nullptr) {
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
