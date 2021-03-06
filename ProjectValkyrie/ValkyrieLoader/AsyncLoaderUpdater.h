#pragma once
#include "AsyncUpdater.h"

class AsyncLoaderUpdater: public AsyncUpdater {
public:
	AsyncLoaderUpdater(std::shared_ptr<GetS3ObjectAsync> s3UpdateFile);

	virtual void Perform();
};