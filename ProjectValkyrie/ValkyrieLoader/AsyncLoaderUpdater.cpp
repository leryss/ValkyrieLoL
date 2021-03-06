#include "AsyncLoaderUpdater.h"
#include <fstream>

AsyncLoaderUpdater::AsyncLoaderUpdater(std::shared_ptr<GetS3ObjectAsync> s3UpdateFile)
	: AsyncUpdater(s3UpdateFile)
{
}

void AsyncLoaderUpdater::Perform()
{
	Download();

	std::ofstream out("new", std::ofstream::out | std::ofstream::binary);
	out.write(downloadBuff, sizeDownload);
	out.flush();

	char szExeFileName[1024];
	GetModuleFileNameA(NULL, szExeFileName, 1023);

	std::string cmd;
	cmd.append("cmd /k \"echo Updating valkyrie loader & ");
	cmd.append("timeout \t 2 & ");
	cmd.append("taskkill /f /pid ");
	cmd.append(std::to_string(GetCurrentProcessId()));
	cmd.append(" & timeout \t 2 & ");
	cmd.append("copy new \"");
	cmd.append(szExeFileName);
	cmd.append("\" & echo Valkyrie loader updated \"");

	WinExec(cmd.c_str(), true);

	SetStatus(ASYNC_SUCCEEDED);
}
