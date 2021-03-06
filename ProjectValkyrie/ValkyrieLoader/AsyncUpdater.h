#pragma once
#include "ValkyrieLoader.h"

class AsyncUpdater : public AsyncTask {

public:
	AsyncUpdater(std::shared_ptr<GetS3ObjectAsync> s3UpdateFile);

	std::shared_ptr<GetS3ObjectAsync> updateFile;

	virtual void Perform() = 0;

protected:
	int   sizeDownload;
	char* downloadBuff;
	void  Download();
};