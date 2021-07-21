#include "GameRenderer.h"

#include <windows.h>
#include <cstring>
#include "Valkyrie.h"

void GameRenderer::ReadFromBaseAddress(int baseAddr) {
	__try {
		memcpy(&viewMatrix, (void*)(baseAddr + Offsets::ViewMatrix), 16 * sizeof(float));
		memcpy(&projMatrix, (void*)(baseAddr + Offsets::ProjectionMatrix), 16 * sizeof(float));

		char* addrRenderer = (char*)*(int*)(baseAddr + Offsets::Renderer);
		memcpy(&width, addrRenderer + Offsets::RendererWidth, sizeof(int));
		memcpy(&height, addrRenderer + Offsets::RendererHeight, sizeof(int));

		MultiplyMatrices(viewProjMatrix, viewMatrix, 4, 4, projMatrix, 4, 4);
	}
	__except (1) {}
}

void GameRenderer::MultiplyMatrices(float * out, float * a, int row1, int col1, float * b, int row2, int col2)
{
	int size = row1 * col2;
	for (int i = 0; i < row1; i++) {
		for (int j = 0; j < col2; j++) {
			float sum = 0.f;
			for (int k = 0; k < col1; k++)
				sum = sum + a[i * col1 + k] * b[k * col2 + j];
			out[i * col2 + j] = sum;
		}
	}
}

Vector2 GameRenderer::WorldToScreen(const Vector3& pos) const {

	Vector2 out = { 0.f, 0.f };
	Vector2 screen = { (float)width, (float)height };
	static Vector4 clipCoords;
	clipCoords.x = pos.x * viewProjMatrix[0] + pos.y * viewProjMatrix[4] + pos.z * viewProjMatrix[8] + viewProjMatrix[12];
	clipCoords.y = pos.x * viewProjMatrix[1] + pos.y * viewProjMatrix[5] + pos.z * viewProjMatrix[9] + viewProjMatrix[13];
	clipCoords.z = pos.x * viewProjMatrix[2] + pos.y * viewProjMatrix[6] + pos.z * viewProjMatrix[10] + viewProjMatrix[14];
	clipCoords.w = pos.x * viewProjMatrix[3] + pos.y * viewProjMatrix[7] + pos.z * viewProjMatrix[11] + viewProjMatrix[15];

	if (clipCoords.w < 1.0f)
		clipCoords.w = 1.f;

	Vector3 M;
	M.x = clipCoords.x / clipCoords.w;
	M.y = clipCoords.y / clipCoords.w;
	M.z = clipCoords.z / clipCoords.w;

	out.x += (screen.x / 2.f * M.x) + (M.x + screen.x / 2.f);
	out.y += -(screen.y / 2.f * M.y) + (M.y + screen.y / 2.f);


	return out;
}

Vector2 GameRenderer::WorldToMinimap(const Vector3& pos, const Vector2& wPos, const Vector2& wSize) const {

	Vector2 result = { pos.x / 15000.f, pos.z / 15000.f };
	result.x = wPos.x + result.x * wSize.x;
	result.y = wPos.y + wSize.y - (result.y * wSize.y);

	return result;
}

float GameRenderer::DistanceToMinimap(float dist, const Vector2& wSize) const {

	// This assumes that the minimap is a square !
	return (dist / 15000.f) * wSize.x;
}

bool GameRenderer::IsScreenPointOnScreen(const Vector2& point, float offsetX, float offsetY) const {
	return point.x > -offsetX && point.x < (width + offsetX) && point.y > -offsetY && point.y < (height + offsetY);
}

bool GameRenderer::IsWorldPointOnScreen(const Vector3& point, float offsetX, float offsetY) const {
	return IsScreenPointOnScreen(WorldToScreen(point), offsetX, offsetY);
}

void GameRenderer::AddDrawCommand(std::shared_ptr<DrawCommand> cmd)
{
	drawCommands.push_back(cmd);
}

void GameRenderer::DrawOverlay(LPDIRECT3DDEVICE9 dxDevice)
{
	/// Backup dx state
	IDirect3DStateBlock9* dxStateBlock = NULL;
	if (dxDevice->CreateStateBlock(D3DSBT_ALL, &dxStateBlock) < 0) {
		drawCommands.clear();
		return;
	}

	D3DMATRIX last_world, last_view, last_projection;
	dxDevice->GetTransform(D3DTS_WORLD, &last_world);
	dxDevice->GetTransform(D3DTS_VIEW, &last_view);
	dxDevice->GetTransform(D3DTS_PROJECTION, &last_projection);
	
	/// Write our state
	D3DVIEWPORT9 vp;
	vp.X = vp.Y = 0;
	vp.Width = width;
	vp.Height = height;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	dxDevice->SetViewport(&vp);

	/// Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing, shade mode (for gradient)
	dxDevice->SetPixelShader(NULL);
	dxDevice->SetVertexShader(NULL);
	dxDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	dxDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	dxDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	dxDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	dxDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
	dxDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	dxDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	dxDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dxDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dxDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	dxDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	dxDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	dxDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	dxDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	dxDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	static const D3DMATRIX identityMatrix = { { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } } };

	dxDevice->SetTransform(D3DTS_WORLD, &identityMatrix);
	dxDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&viewMatrix[0]);
	dxDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&projMatrix[0]);

	if (!vertexBuff) {
		if (dxDevice->CreateVertexBuffer(VertexBuffSize * sizeof(Vertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1), D3DPOOL_DEFAULT, &vertexBuff, NULL) < 0) {
			Logger::Error("Failed to create vtx buff");
			drawCommands.clear();
			return;
		}
	}

	Vertex* vtx;
	if (vertexBuff->Lock(0, (UINT)(VertexBuffSize), (void**)&vtx, D3DLOCK_DISCARD) < 0) {
		Logger::Error("Failed to lock vtx buff");
		drawCommands.clear();
		return;
	}

	for (auto cmd : drawCommands) {
		cmd->WriteVertices(vtx);
		vtx += cmd->GetVertexCount();
	}

	vertexBuff->Unlock();
	dxDevice->SetStreamSource(0, vertexBuff, 0, sizeof(Vertex));
	dxDevice->SetFVF((D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1));

	RECT scissor = { 0, 0, width, height };
	dxDevice->SetScissorRect(&scissor);

	PDIRECT3DTEXTURE9 currentTexture = (PDIRECT3DTEXTURE9)1;
	int vertexOffset = 0;
	for (auto cmd : drawCommands) {
		if (currentTexture != cmd->texture) {
			dxDevice->SetTexture(0, cmd->texture);
			currentTexture = cmd->texture;
		}
		
		cmd->Draw(dxDevice, vertexOffset);
		vertexOffset += cmd->GetVertexCount();
	}

	/// Restore dx state
	dxDevice->SetTransform(D3DTS_WORLD, &last_world);
	dxDevice->SetTransform(D3DTS_VIEW, &last_view);
	dxDevice->SetTransform(D3DTS_PROJECTION, &last_projection);

	dxStateBlock->Apply();
	dxStateBlock->Release();

	drawCommands.clear();
}

void GameRenderer::DrawCircleAt(ImDrawList* canvas, const Vector3& worldPos, float radius, int numPoints, ImColor color, float thickness) const {

	if (numPoints >= 200)
		return;
	static Vector2 points[202];
	static Vector2 center;

	float step = 6.2831f / numPoints;
	float theta = 0.f;

	Vector2 screenSpace = WorldToScreen({ worldPos.x, worldPos.y, worldPos.z});
	center.x = screenSpace.x;
	center.y = screenSpace.y;

	screenSpace = WorldToScreen({ worldPos.x + radius * cos(theta), worldPos.y, worldPos.z - radius * sin(theta)});
	points[0].x = screenSpace.x;
	points[0].y = screenSpace.y;

	float thicknessFraction = thickness / 3.0f;
	for (int i = 1; i <= numPoints + 1; i++, theta += step) {
		screenSpace = WorldToScreen({ worldPos.x + radius * cos(theta), worldPos.y, worldPos.z - radius * sin(theta) });

		points[i].x = screenSpace.x;
		points[i].y = screenSpace.y;

		canvas->AddLine((ImVec2&)points[i - 1], (ImVec2&)points[i], color, thickness + thicknessFraction * sin(theta));
	}
}

void GameRenderer::DrawCircleAtFilled(ImDrawList* canvas, const Vector3& worldPos, float radius, int numPoints, ImColor color) const {

	if (numPoints >= 200)
		return;
	static ImVec2 points[200];

	float step = 6.2831f / numPoints;
	float theta = 0.f;
	for (int i = 0; i < numPoints; i++, theta += step) {
		Vector2 screenSpace = WorldToScreen({ worldPos.x + radius * cos(theta), worldPos.y, worldPos.z - radius * sin(theta) });

		points[i].x = screenSpace.x;
		points[i].y = screenSpace.y;
	}

	canvas->AddConvexPolyFilled(points, numPoints, color);
}

void DrawCommandImage::WriteVertices(Vertex * vtx)
{
	vtx->col = color;
	vtx->pos[0] = p1.x;
	vtx->pos[1] = p1.y;
	vtx->pos[2] = p1.z;
	vtx->uv[0] = 0.f;
	vtx->uv[1] = 0.f;

	vtx++;
	vtx->col = color;
	vtx->pos[0] = p2.x;
	vtx->pos[1] = p2.y;
	vtx->pos[2] = p2.z;
	vtx->uv[0] = 1.f;
	vtx->uv[1] = 0.f;

	vtx++;
	vtx->col = color;
	vtx->pos[0] = p4.x;
	vtx->pos[1] = p4.y;
	vtx->pos[2] = p4.z;
	vtx->uv[0] = 0.f;
	vtx->uv[1] = 1.f;

	vtx++;
	vtx->col = color;
	vtx->pos[0] = p4.x;
	vtx->pos[1] = p4.y;
	vtx->pos[2] = p4.z;
	vtx->uv[0] = 0.f;
	vtx->uv[1] = 1.f;

	vtx++;
	vtx->col = color;
	vtx->pos[0] = p3.x;
	vtx->pos[1] = p3.y;
	vtx->pos[2] = p3.z;
	vtx->uv[0] = 1.f;
	vtx->uv[1] = 1.f;

	vtx++;
	vtx->col = color;
	vtx->pos[0] = p2.x;
	vtx->pos[1] = p2.y;
	vtx->pos[2] = p2.z;
	vtx->uv[0] = 1.f;
	vtx->uv[1] = 0.f;
}

int DrawCommandImage::GetVertexCount()
{
	return 6;
}

void DrawCommandImage::Draw(LPDIRECT3DDEVICE9 device, int vtxOffset)
{
	device->DrawPrimitive(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, vtxOffset, 2);
}

void DrawCommandCircle::WriteVertices(Vertex * vtx)
{
	float step = 6.2831f / numPoints;
	float theta = 0.f;

	for (int i = 0; i < numPoints; i++, theta += step) {
		vtx->col = color;
		vtx->pos[0] = pos.x + radius*cos(theta);
		vtx->pos[1] = pos.y;
		vtx->pos[2] = pos.z - radius*sin(theta);
		vtx->uv[0] = 0.f;
		vtx->uv[1] = 0.f;
		vtx++;

		vtx->col = color;
		vtx->pos[0] = pos.x + (radius - thickness) * cos(theta);
		vtx->pos[1] = pos.y;
		vtx->pos[2] = pos.z - (radius - thickness) * sin(theta);
		vtx->uv[0] = 0.f;
		vtx->uv[1] = 0.f;
		vtx++;
	}

	vtx->col = color;
	vtx->pos[0] = pos.x + radius * cos(0.f);
	vtx->pos[1] = pos.y;
	vtx->pos[2] = pos.z - radius * sin(0.f);
	vtx->uv[0] = 0.f;
	vtx->uv[1] = 0.f;
	vtx++;

	vtx->col = color;
	vtx->pos[0] = pos.x + (radius - thickness) * cos(0.f);
	vtx->pos[1] = pos.y;
	vtx->pos[2] = pos.z - (radius - thickness) * sin(0.f);
	vtx->uv[0] = 0.f;
	vtx->uv[1] = 0.f;
}

int DrawCommandCircle::GetVertexCount()
{
	return 2*(numPoints + 1);
}

void DrawCommandCircle::Draw(LPDIRECT3DDEVICE9 device, int vtxOffset)
{
	device->DrawPrimitive(D3DPRIMITIVETYPE::D3DPT_TRIANGLESTRIP, vtxOffset, 2*numPoints);
}

void DrawCommandCircleFilled::WriteVertices(Vertex * vtx)
{
	float step = 6.2831f / numPoints;
	float theta = 0.f;

	vtx->col = D3DCOLOR_ARGB(0, 255, 255, 255);
	vtx->pos[0] = pos.x;
	vtx->pos[1] = pos.y;
	vtx->pos[2] = pos.z;
	vtx->uv[0] = 0.f;
	vtx->uv[1] = 0.f;
	vtx++;

	for (int i = 0; i < numPoints; i++, theta += step) {
		vtx->col = color;
		vtx->pos[0] = pos.x + radius * cos(theta);
		vtx->pos[1] = pos.y;
		vtx->pos[2] = pos.z - radius * sin(theta);
		vtx->uv[0] = 0.f;
		vtx->uv[1] = 0.f;
		vtx++;
	}

	vtx->col = color;
	vtx->pos[0] = pos.x + radius * cos(0.f);
	vtx->pos[1] = pos.y;
	vtx->pos[2] = pos.z - radius * sin(0.f);
	vtx->uv[0] = 0.f;
	vtx->uv[1] = 0.f;
}

int DrawCommandCircleFilled::GetVertexCount()
{
	return numPoints + 2;
}

void DrawCommandCircleFilled::Draw(LPDIRECT3DDEVICE9 device, int vtxOffset)
{
	device->DrawPrimitive(D3DPRIMITIVETYPE::D3DPT_TRIANGLEFAN, vtxOffset, numPoints);
}
