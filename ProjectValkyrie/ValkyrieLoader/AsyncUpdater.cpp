#include "AsyncUpdater.h"
#include "miniz/miniz.h"
#include <fstream>

AsyncUpdater::AsyncUpdater(ValkyrieLoader & vloader, std::shared_ptr<GetS3ObjectAsync> s3UpdateFile)
	:loader(vloader), updateFile(s3UpdateFile)
{}

void AsyncUpdater::Perform()
{
	auto& etag = updateFile->result.GetETag();
	if (etag.compare(loader.versionHash.c_str()) != 0) {

		int   sizeDownloaded;
		char* downloadBuff = Download(sizeDownloaded);

		if (!Extract(downloadBuff, sizeDownloaded))
			return;
		if (!CopyDependencies())
			return;

		delete[] downloadBuff;
	}
	
	UpdateVersionHash();
	ReadChangeLog();

	SetStatus(ASYNC_SUCCEEDED);
}

char* AsyncUpdater::Download(int& size)
{
	currentStep = "Downloading data";

	auto& updateFileStream = updateFile->result.GetBody();

	updateFileStream.seekg(0, updateFileStream.end);
	int sizeDownloadFull = (int)updateFileStream.tellg();
	updateFileStream.seekg(0, updateFileStream.beg);

	size = 0;
	char* downloadBuff = new char[sizeDownloadFull];
	while (true) {
		int readBytes = (int)updateFileStream.readsome(downloadBuff + size, 10000);
		size += readBytes;
		if (readBytes == 0)
			break;
	}

	return downloadBuff;
}

bool AsyncUpdater::Extract(char* downloaded, int sizeDownloaded)
{
	currentStep = "Extracting data";

	mz_zip_archive archive;
	memset(&archive, 0, sizeof(archive));
	if (!mz_zip_reader_init_mem(&archive, downloaded, sizeDownloaded, 0)) {
		SetError("Failed to open update archive");
		return false;
	}

	int numFilesToExtract = mz_zip_reader_get_num_files(&archive);
	for (int i = 0; i < numFilesToExtract; ++i) {
		mz_zip_archive_file_stat fileStat;
		if (!mz_zip_reader_file_stat(&archive, i, &fileStat)) {
			auto err = std::string("Failed to get archive file info for file num ") + std::to_string(i);
			SetError(err.c_str());
			return false;
		}

		std::string path = loader.valkyrieFolder + "\\" + fileStat.m_filename;
		if (fileStat.m_is_directory) {
			CreateDirectoryA(path.c_str(), NULL);
		}
		else if (!mz_zip_reader_extract_file_to_file(&archive, fileStat.m_filename, path.c_str(), 0)) {
			auto err = std::string("Failed to unarchive file ") + fileStat.m_filename;
			SetError(err.c_str());
			return false;
		}
	}

	mz_zip_reader_end(&archive);

	return true;
}

void AsyncUpdater::UpdateVersionHash()
{
	loader.versionHash = updateFile->result.GetETag().c_str();
	std::ofstream versionFile(loader.valkyrieFolder + "\\version");
	versionFile.write(loader.versionHash.c_str(), loader.versionHash.size());
}

bool AsyncUpdater::CopyDependencies()
{
	currentStep = "Copying dependencies";

	/// Get windows directory
	char windirBuff[1024];
	if (0 == GetWindowsDirectoryA(windirBuff, 1024)) {
		SetError("Couldn't get windows dir");
		return false;
	}
	std::string winDir(windirBuff);

	/// Copy all dlls from dependencies dir to windows dir so they can be automatically loaded when our dll injects
	WIN32_FIND_DATAA findData;
	HANDLE hFind;
	std::string folderPath = loader.valkyrieFolder + PATH_DLL_DEPENDENCIES;
	hFind = FindFirstFileA((folderPath + "\\*.dll").c_str(), &findData);
	do {
		if (hFind != INVALID_HANDLE_VALUE) {

			std::string copyFrom = folderPath + "\\" + findData.cFileName;
			std::string copyTo   = winDir + "\\" + findData.cFileName;
			//printf("Copying %s to %s\n", copyFrom.c_str(), copyTo.c_str());
			CopyFileA(copyFrom.c_str(), copyTo.c_str(), FALSE);
		}
	} while (FindNextFileA(hFind, &findData));

	return true;
}

void AsyncUpdater::ReadChangeLog()
{
	std::ifstream changeLogFile(loader.valkyrieFolder + "\\changelog.txt");
	loader.changeLog = std::string((std::istreambuf_iterator<char>(changeLogFile)), std::istreambuf_iterator<char>());
}
