#pragma once
#include "AsyncTask.h"
#include <windows.h>

class AsyncInjector : public AsyncTask {


public:
	             AsyncInjector(std::string dllPath);
	virtual void Perform();
				 
private:		 
	void         InjectIntoPID(std::string& dllPath, DWORD pid);

private:
	bool         injected  = false;
	HWND         handleInjected = NULL;
	std::string  dllPath;
};