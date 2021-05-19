#pragma once
#include <cmath>
#include "imgui/imgui.h"
#include "Strings.h"

struct Vector2 {
	Vector2() {};
	Vector2(float _x, float _y) {
		x = _x;
		y = _y; 
	}

	float x;
	float y;

	void ImGuiDraw(const char* label){
		ImGui::Text(label);
		ImGui::Button(Strings::Format("x: %.2f y: %.2f", x, y).c_str());
	}

	float length() const {
		return sqrt(x*x + y * y);
	}

	float distance(const Vector2& o) const {
		return sqrt(pow(x - o.x, 2) + pow(y - o.y, 2));
	}

	float l1(const Vector2& o) const {
		return abs(x - o.x) + abs(y - o.y);
	}

	Vector2 vscale(const Vector2& s) const {
		return Vector2(x*s.x, y*s.y);
	}

	Vector2 scale(float s) const {
		return Vector2(x*s, y*s);
	}

	Vector2 normalize() const {
		float l = length();
		if (l < 0.0001f)
			return Vector2(0.f, 0.f);
		return Vector2(x / l, y / l);
	}

	Vector2 add(const Vector2& o) const {
		return Vector2(x + o.x, y + o.y);
	}

	Vector2 sub(const Vector2& o) const {
		return Vector2(x - o.x, y - o.y);
	}

	Vector2 clone() const {
		return Vector2(x, y);
	}
};

struct Vector3 {
	Vector3() {};
	Vector3(float _x, float _y, float _z) {
		x = _x; 
		y = _y;
		z = _z; 
	}

	float x;
	float y;
	float z;

	void ImGuiDraw(const char* label) {
		ImGui::Text(label);
		ImGui::SameLine();
		ImGui::Button(Strings::Format("x: %.2f y: %.2f z: %.2f", x, y, z).c_str());
	}

	float length() const {
		return sqrt(x*x + y*y + z*z);
	}

	float distance(const Vector3& o) const {
		return sqrt(pow(x - o.x, 2) + pow(y - o.y, 2) + pow(z - o.z, 2));
	}

	float l1(const Vector3& o) const {
		return abs(x - o.x) + abs(y - o.y) + abs(z - o.z);
	}

	Vector3 rotate_x(float angle) const {
		return Vector3(
			x,
			y * cos(angle) - z * sin(angle),
			y * sin(angle) + z * cos(angle)
		);
	}

	Vector3 rotate_y(float angle) const {
		return Vector3(
			x * cos(angle) + z * sin(angle),
			y,
			-x * sin(angle) + z * cos(angle)
		);
	}

	Vector3 rotate_z(float angle) const {
		return Vector3(
			x * cos(angle) - y * sin(angle),
			x * sin(angle) + y * cos(angle),
			z
		);
	}	

	Vector3 vscale(const Vector3& s) const {
		return Vector3(x*s.x, y*s.y, z*s.z);
	}
	
	Vector3 scale(float s) const {
		return Vector3(x*s, y*s, z*s);
	}

	Vector3 normalize() const {
		float l = length();
		if (l < 0.0001f)
			return Vector3(0.f, 0.f, 0.f);
		return Vector3(x / l, y / l, z / l);
	}

	Vector3 add(const Vector3& o) const {
		return Vector3(x + o.x, y + o.y, z + o.z);
	}

	Vector3 sub(const Vector3& o) const {
		return Vector3(x - o.x, y - o.y, z - o.z);
	}

	Vector3 clone() const {
		return Vector3(x, y, z);
	}

	float angle(const Vector3& o) const {
		return acos(dot(o) / (length() + o.length()));
	}

	float dot(const Vector3& o) const {
		return (x*o.x + y * o.y + z * o.z);
	}
};

struct Vector4 {
	Vector4() {};
	Vector4(float _x, float _y, float _z, float _w) { 
		x = _x; 
		y = _y; 
		z = _z; 
		w = _w; 
	}

	float x;
	float y;
	float z;
	float w;

	void ImGuiDraw(const char* label) {
		ImGui::Text(label);
		ImGui::Button(Strings::Format("x: %.2f y: %.2f z: %.2f w: %2.f", x, y, z, w).c_str());
	}

	float length() const {
		return sqrt(x*x + y*y + z*z + w*w);
	}

	float distance(const Vector4& o) const {
		return sqrt(pow(x - o.x, 2) + pow(y - o.y, 2) + pow(z - o.z, 2) + pow(w - o.w, 2));
	}

	float l1(const Vector4& o) const {
		return abs(x - o.x) + abs(y - o.y) + abs(z - o.z) + abs(w - o.w);
	}

	Vector4 vscale(const Vector4& s) const {
		return Vector4(x*s.x, y*s.y, z*s.z, w*s.w);
	}

	Vector4 scale(float s) const {
		return Vector4(x*s, y*s, z*s, w*s);
	}

	Vector4 normalize() const {
		float l = length();
		if (l < 0.0001f)
			return Vector4(0.f, 0.f, 0.f, 0.f);
		return Vector4(x / l, y / l, z / l, w / l);
	}

	Vector4 add(const Vector4& o) const {
		return Vector4(x + o.x, y + o.y, z + o.z, w + o.w);
	}

	Vector4 sub(const Vector4& o) const {
		return Vector4(x - o.x, y - o.y, z - o.z, w - o.w);
	}

	Vector4 clone() const {
		return Vector4(x, y, z, w);
	}
};