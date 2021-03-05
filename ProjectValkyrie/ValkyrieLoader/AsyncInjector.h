#pragma once
#include "AsyncTask.h"
#include <windows.h>

class AsyncInjector : public AsyncTask {


public:
	             AsyncInjector(std::string dllPath, bool oneTimeInjection, int delayMs);
	virtual void Perform();
				 
private:		 
	void         InjectIntoPID(std::string& dllPath, DWORD pid);

private:
	int          delay;
	bool         oneTimeInjection;
	bool         injected  = false;
	HWND         handleInjected = NULL;
	std::string  dllPath;
};