#pragma once
#include <string>

#include <windows.h>
#include "Paths.h"
#include "miniz/miniz.h"
#include "imgui/imgui.h"

class ValkyrieShared {

public:
	static void SaveCredentials(const char* name, const char* pass) {
		
		HKEY key;
		DWORD disposition;
		LSTATUS status;

		if (ERROR_SUCCESS != (status = RegCreateKeyExA(
			HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Valkyrie",
			NULL,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_WRITE,
			NULL,
			&key,
			&disposition))){

			auto err = std::string("Failed to open valkyrie registry ") + std::to_string(status);
			throw std::exception(err.c_str());
		}

		if (ERROR_SUCCESS != (status = RegSetKeyValueA(key, "", "username", REG_SZ, (const BYTE*)name, strlen(name)))) {
			auto err = std::string("Failed to set valkyrie registry value ") + std::to_string(status);
			throw std::exception(err.c_str());
		}
		if (ERROR_SUCCESS != (status = RegSetKeyValueA(key, "", "password", REG_SZ, (const BYTE*)pass, strlen(pass)))) {
			auto err = std::string("Failed to set valkyrie registry value ") + std::to_string(status);
			throw std::exception(err.c_str());
		}

		RegCloseKey(key);
	}

	static void LoadCredentials(char* name, char* pass, int sizeBuffs) { 
		HKEY key;
		DWORD disposition;
		LSTATUS status;

		if (ERROR_SUCCESS != (status = RegCreateKeyExA(
			HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Valkyrie",
			NULL,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_READ,
			NULL,
			&key,
			&disposition))) {

			auto err = std::string("Failed to open valkyrie registry ") + std::to_string(status);
			throw std::exception(err.c_str());
		}

		DWORD size = sizeBuffs;
		if (ERROR_SUCCESS != (status = RegGetValueA(key, "", "username", RRF_RT_REG_SZ, NULL, name, &size))) {
			if (status == ERROR_FILE_NOT_FOUND)
				return;

			auto err = std::string("Failed to get valkyrie registry value ") + std::to_string(status);
			throw std::exception(err.c_str());
		}

		size = sizeBuffs;
		if (ERROR_SUCCESS != (status = RegGetValueA(key, "", "password", RRF_RT_REG_SZ, NULL, pass, &size))) {
			auto err = std::string("Failed to get valkyrie registry value ") + std::to_string(status);
			throw std::exception(err.c_str());
		}

		RegCloseKey(key);
	}

	static void LoadCredentials(char** name, char** pass) {

		DWORD size = 256;
		*name = new char[size];
		*pass = new char[size];

		LoadCredentials(*name, *pass, size);
	}

	static void ImGuiSetupSizesAndFont() {
		ImGuiStyle* style = &ImGui::GetStyle();
		style->WindowPadding = { 16.f, 11.f };
		style->FramePadding = { 16.f, 5.f };
		style->CellPadding = { 10.f, 2.f };
		style->ItemSpacing = { 8.f, 4.f };
		style->ItemInnerSpacing = { 4.f, 7.f };
		style->IndentSpacing = 20.f;
		style->ScrollbarSize = 20.f;
		style->GrabMinSize = 20.f;

		style->WindowBorderSize = 1.5f;
		style->ChildBorderSize = 1.5f;
		style->PopupBorderSize = 1.5f;
		style->FrameBorderSize = 1.5f;
		style->TabBorderSize = 1.5f;

		style->WindowRounding = 4.f;
		style->ChildRounding = 4.f;
		style->FrameRounding = 6.f;
		style->PopupRounding = 6.f;
		style->ScrollbarRounding = 6.f;
		style->GrabRounding = 6.f;
		style->LogSliderDeadzone = 0.f;
		style->TabRounding = 8.f;

		std::string fontPath = Paths::Root + "\\vfont.ttf";
		if (Paths::FileExists(fontPath)) {
			auto fontAtlas = ImGui::GetIO().Fonts;

			static const ImWchar ranges[] =
			{
				0x0020, 0x00FF, // Basic Latin + Latin Supplement
				0x2000, 0x3000,
				0,
			};
			
			FontConsolas = fontAtlas->AddFontDefault();
			FontValkyrie = fontAtlas->AddFontFromFileTTF(fontPath.c_str(), 13, 0, ranges);
		}
	}

	static void ImGuiStyleValkyrie() {
		ImGuiStyle* style = &ImGui::GetStyle();
		ImVec4* colors = style->Colors;
		colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg]               = ImVec4(0.08f, 0.08f, 0.08f, 0.88f);
		colors[ImGuiCol_ChildBg]                = ImVec4(0.07f, 0.07f, 0.07f, 0.94f);
		colors[ImGuiCol_PopupBg]                = ImVec4(0.09f, 0.09f, 0.09f, 0.88f);
		colors[ImGuiCol_Border]                 = ImVec4(0.72f, 0.72f, 0.72f, 0.50f);
		colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg]                = ImVec4(0.03f, 0.03f, 0.03f, 0.54f);
		colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.16f, 0.16f, 0.16f, 0.40f);
		colors[ImGuiCol_FrameBgActive]          = ImVec4(0.33f, 0.34f, 0.34f, 0.67f);
		colors[ImGuiCol_TitleBg]                = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TitleBgActive]          = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 1.00f, 0.00f, 1.00f);
		colors[ImGuiCol_SliderGrab]             = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
		colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
		colors[ImGuiCol_Button]                 = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_ButtonHovered]          = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
		colors[ImGuiCol_ButtonActive]           = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_Header]                 = ImVec4(0.53f, 0.53f, 0.53f, 0.31f);
		colors[ImGuiCol_HeaderHovered]          = ImVec4(0.81f, 0.81f, 0.81f, 0.31f);
		colors[ImGuiCol_HeaderActive]           = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
		colors[ImGuiCol_Separator]              = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
		colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
		colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
		colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colors[ImGuiCol_Tab]                    = ImVec4(0.00f, 0.00f, 0.00f, 0.86f);
		colors[ImGuiCol_TabHovered]             = ImVec4(0.34f, 0.34f, 0.34f, 0.80f);
		colors[ImGuiCol_TabActive]              = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
		colors[ImGuiCol_TabUnfocused]           = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
		colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}

	public:
		static ImFont* FontConsolas;
		static ImFont* FontValkyrie;
}; 
