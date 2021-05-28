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

enum OffsetExtractLocation {
	AddressInPattern,
	AddressIsPatternLocation,
	AddressInPatternPlusLocation
};

/// Byte like signature for a game memory offset
class OffsetSignature {

public:

	     /// name: Name of the offset
	     /// pattern: Byte like pattern of the offset ex: "3B 9C ? ? A9"
	     /// extractIndex: The position in the pattern at which the offset resides
	     /// offsetInAddress: Set this True if you want the offset to be calculated from the address where the pattern was found and not from the pattern itself
	     OffsetSignature(const char* name, const char* pattern, int extractIndex, OffsetExtractLocation offsetLocation = AddressInPattern, bool subtractModuleAddress = true);
	void Scan(int startAddr, int size);
	
	const char* name         = "";
	const char* pattern      = "";
	ScanStatus  status       = SCAN_NOT_STARTED;
	int         offset       = 0;
	int         extractIndex = 0;
	bool subtractModuleAddress;
	OffsetExtractLocation offsetLocation;

private:
	std::vector<int> values;

	std::vector<char> bytes;
	std::vector<bool> mask;
};

/// Utility to scan the game code for game struct/function memory offsets by pattern matching
class OffsetScanner {

public:
	static void ImGuiDraw();
	static void Scan();

private:
	static char                         CodeDump[2048];
	static bool                         Scanning;
	static std::vector<OffsetSignature> signatures;
};