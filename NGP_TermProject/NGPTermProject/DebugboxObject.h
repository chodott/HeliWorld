#pragma once
#include"Object.h"
#include"Shader.h"
class CDebugboxObject: public CGameObject
{
public:
	CDebugboxObject();
	virtual ~CDebugboxObject();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	XMFLOAT4 debugColor;
};

