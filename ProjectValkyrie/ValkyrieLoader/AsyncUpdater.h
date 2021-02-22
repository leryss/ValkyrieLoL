#pragma once
#include "AsyncTask.h"
#include "ValkyrieLoader.h"

class AsyncUpdater : public AsyncTask {

public:

	AsyncUpdater(ValkyrieLoader& vloader, std::shared_ptr<GetS3ObjectAsync> s3UpdateFile);

	ValkyrieLoader&                   loader;
	std::shared_ptr<GetS3ObjectAsync> updateFile;

	virtual void Perform();

private:
	const std::string PATH_DLL_DEPENDENCIES = "\\dependencies";

	char* Download(int& size);
	bool Extract(char* downloaded, int sizeDownloaded);
	void UpdateVersionHash();
	bool CopyDependencies();
	void ReadChangeLog();

};