#include "AsyncUpdater.h"

AsyncUpdater::AsyncUpdater(ValkyrieLoader & vloader, std::shared_ptr<GetS3ObjectAsync> s3UpdateFile)
	:loader(vloader), updateFile(s3UpdateFile)
{
}

void AsyncUpdater::Download()
{
	currentStep = "Downloading data";

	auto& updateFileStream = updateFile->result.GetBody();

	updateFileStream.seekg(0, updateFileStream.end);
	int sizeDownloadFull = (int)updateFileStream.tellg();
	updateFileStream.seekg(0, updateFileStream.beg);

	sizeDownload = 0;
	downloadBuff = new char[sizeDownloadFull];
	while (true) {
		int readBytes = (int)updateFileStream.readsome(downloadBuff + sizeDownload, 10000);
		sizeDownload += readBytes;
		if (readBytes == 0)
			break;
	}
}
