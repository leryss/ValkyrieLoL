#pragma once
#include "AsyncUpdater.h"

class AsyncCheatUpdater : public AsyncUpdater {

public:
	AsyncCheatUpdater(std::shared_ptr<GetS3ObjectAsync> s3UpdateFile);

	virtual void Perform();

private:
	bool Extract(char* downloaded, int sizeDownloaded);
	bool CopyDependencies();

};