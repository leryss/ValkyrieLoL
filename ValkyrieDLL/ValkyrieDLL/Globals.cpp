#include "Globals.h"
#include <fstream>

fs::path FindWorkingDir() {
	fs::path appdata = fs::temp_directory_path().parent_path().parent_path().parent_path();
	appdata /= "Roaming";
	appdata /= "Valkyrie";

	return appdata;
}

fs::path    Globals::WorkingDir  = FindWorkingDir();
