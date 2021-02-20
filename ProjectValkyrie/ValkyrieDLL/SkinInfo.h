#pragma once
#include <vector>
#include "Vector.h"
#include "imgui/imgui.h"

class SkinChroma {

public:
	int     id;
	ImVec4  color;
};

class SkinInfo {

public:
	int                     id;
	std::string             name;
	std::vector<SkinChroma> chromas;
};