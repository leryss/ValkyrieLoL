#pragma once
#include "imgui/imgui.h"
#include <vector>
#include <thread>

enum ScanStatus {
	SCAN_NOT_STARTED,
	SCAN_IN_PROGRESS,
	SCAN_FOUND,
	SCAN_NOT_FOUND
};

class OffsetSignature {

public:

	     OffsetSignature(const char* name, const char* pattern, int extractIndex);
	void Scan(int startAddr, int size);
	
	const char* name         = "";
	const char* pattern      = "";
	ScanStatus  status       = SCAN_NOT_STARTED;
	int         offset       = 0;
	int         extractIndex = 0;

private:
	std::vector<int> values;

	std::vector<char> bytes;
	std::vector<bool> mask;
};

class OffsetScanner {

public:
	static void ImGuiDraw();
	static void Scan();

private:
	static char                         CodeDump[2048];
	static bool                         Scanning;
	static std::vector<OffsetSignature> signatures;
};