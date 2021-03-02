#pragma once
#include "AsyncUpdater.h"

class AsyncCheatUpdater : public AsyncUpdater {

public:
	AsyncCheatUpdater(ValkyrieLoader& vloader, std::shared_ptr<GetS3ObjectAsync> s3UpdateFile);

	virtual void Perform();

private:
	bool Extract(char* downloaded, int sizeDownloaded);
	void UpdateVersionHash();
	bool CopyDependencies();
	void ReadChangeLog();

};