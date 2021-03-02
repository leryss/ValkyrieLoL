#include "AsyncLoaderUpdater.h"
#include <fstream>

AsyncLoaderUpdater::AsyncLoaderUpdater(ValkyrieLoader & vloader, std::shared_ptr<GetS3ObjectAsync> s3UpdateFile)
	: AsyncUpdater(vloader, s3UpdateFile)
{
}

void AsyncLoaderUpdater::Perform()
{
	auto& etag = updateFile->result.GetETag();
	if (etag.compare(loader.loaderVersionHash.c_str()) != 0) {

		Download();

		std::ofstream out("new.exe", std::ofstream::out | std::ofstream::binary);
		out.write(downloadBuff, sizeDownload);
		out.flush();

		loader.loaderVersionHash = etag.c_str();
		loader.SaveConfigs();

		char szExeFileName[1024];
		GetModuleFileNameA(NULL, szExeFileName, 1023);

		std::string cmd;
		cmd.append("cmd /k \"echo Updating valkyrie loader & ");
		cmd.append("timeout \t 2 & ");
		cmd.append("taskkill /f /pid ");
		cmd.append(std::to_string(GetCurrentProcessId()));
		cmd.append(" & timeout \t 2 & ");
		cmd.append("copy new.exe \"");
		cmd.append(szExeFileName);
		cmd.append("\" & echo Valkyrie loader updated \"");

		WinExec(cmd.c_str(), true);
	}

	SetStatus(ASYNC_SUCCEEDED);
}
