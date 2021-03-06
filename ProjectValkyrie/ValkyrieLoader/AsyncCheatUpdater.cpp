#include "AsyncCheatUpdater.h"
#include "miniz/miniz.h"
#include "Paths.h"
#include <fstream>

AsyncCheatUpdater::AsyncCheatUpdater(std::shared_ptr<GetS3ObjectAsync> s3UpdateFile)
	:AsyncUpdater(s3UpdateFile)
{
}

void AsyncCheatUpdater::Perform()
{
	Download();

	if (!Extract(downloadBuff, sizeDownload))
		return;
	if (!CopyDependencies())
		return;

	delete[] downloadBuff;

	SetStatus(ASYNC_SUCCEEDED);
}

bool AsyncCheatUpdater::Extract(char* downloaded, int sizeDownloaded)
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

		std::string path = Paths::Root + "\\" + fileStat.m_filename;
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

bool AsyncCheatUpdater::CopyDependencies()
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
	std::string folderPath = Paths::Dependencies;
	hFind = FindFirstFileA((folderPath + "\\*.dll").c_str(), &findData);
	do {
		if (hFind != INVALID_HANDLE_VALUE) {

			std::string copyFrom = folderPath + "\\" + findData.cFileName;
			std::string copyTo   = winDir + "\\" + findData.cFileName;
			CopyFileA(copyFrom.c_str(), copyTo.c_str(), FALSE);
		}
	} while (FindNextFileA(hFind, &findData));

	return true;
}
