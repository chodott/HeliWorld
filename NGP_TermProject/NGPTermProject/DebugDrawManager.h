#pragma once
#include <vector>
#include "DebugboxShader.h"
class DebugDrawManager
{
private:
	class CDebugboxShader* m_pDebugboxShader;
	int count = 0;

public:
	static DebugDrawManager& Get();

	void AddDebugCube(XMFLOAT3& position, XMFLOAT3& rotation, XMFLOAT4 color);

	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int npipelinestate);

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

	
};

