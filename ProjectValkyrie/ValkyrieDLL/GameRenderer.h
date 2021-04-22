#pragma once
#include "MemoryReadable.h"
#include "Vector.h"
#include <D3dx9core.h>
#include <d3d9.h>
#include <vector>

#define COL_TO_D3COL(col) D3DCOLOR_ARGB((int)(col.w*255), (int)(col.x*255), (int)(col.y*255), (int)(col.z*255))

struct Vertex {
	float    pos[3];
	D3DCOLOR col;
	float    uv[2];
};

class DrawCommand {

public:
	virtual void WriteVertices(Vertex* vtx) = 0;
	virtual int  GetVertexCount() = 0;
	virtual void Draw(LPDIRECT3DDEVICE9 device, int vtxOffset) = 0;

public:
	PDIRECT3DTEXTURE9 texture;
};

class DrawCommandCircle : public DrawCommand {
	
public:
	virtual void WriteVertices(Vertex* vtx);
	virtual int  GetVertexCount();
	virtual void Draw(LPDIRECT3DDEVICE9 device, int vtxOffset);

public:
	Vector3  pos;
	int      numPoints = 4;
	float    radius;
	float    thickness;
	D3DCOLOR color;
};

class DrawCommandCircleFilled : public DrawCommand {

public:
	virtual void WriteVertices(Vertex* vtx);
	virtual int  GetVertexCount();
	virtual void Draw(LPDIRECT3DDEVICE9 device, int vtxOffset);

public:
	Vector3  pos;
	int      numPoints = 4;
	float    radius;
	D3DCOLOR color;
};

class DrawCommandImage: public DrawCommand {

public:
	virtual void WriteVertices(Vertex* vtx);
	virtual int  GetVertexCount();
	virtual void Draw(LPDIRECT3DDEVICE9 device, int vtxOffset);

public:
	Vector3  p1, p2, p3, p4;
	D3DCOLOR color;
};

class GameRenderer: MemoryReadable{

public:
	int width;
	int height;

	float viewMatrix[16];
	float projMatrix[16];
	float viewProjMatrix[16];

	void ReadFromBaseAddress(int baseAddr);

	/// Converts world coordinates to screen coordinates
	Vector2  WorldToScreen(const Vector3& pos) const;

	/// Converts world coordinates to minimap coordinates
	Vector2  WorldToMinimap(const Vector3& pos, const Vector2& wPos, const Vector2& wSize) const;

	/// Converts distances in world space to minimap space
	float    DistanceToMinimap(float dist, const Vector2& wSize) const;

	/// Draws a circle at the given coordinate. Coordinates and radius must be in world space 
	void     DrawCircleAt(ImDrawList* canvas, const Vector3& worldPos, float radius, int numPoints, ImColor color, float thickness = 3.f) const;
	void     DrawCircleAtFilled(ImDrawList* canvas, const Vector3& worldPos, float radius, int numPoints, ImColor color) const;

	/// Used to determine if a screen space point is on screen
	bool     IsScreenPointOnScreen(const Vector2& point, float offsetX = 0.f, float offsetY = 0.f) const;

	/// Used to determine if a world space point is on screen
	bool     IsWorldPointOnScreen(const Vector3& point, float offsetX = 0.f, float offsetY = 0.f) const;

	void     AddDrawCommand(std::shared_ptr<DrawCommand> cmd);
	void     DrawOverlay(LPDIRECT3DDEVICE9 dxDevice);

private:
	void     MultiplyMatrices(float *out, float *a, int row1, int col1, float *b, int row2, int col2);

	std::vector<std::shared_ptr<DrawCommand>> drawCommands;
	const static int                          VertexBuffSize = 5000;
	LPDIRECT3DVERTEXBUFFER9                   vertexBuff     = NULL;
};