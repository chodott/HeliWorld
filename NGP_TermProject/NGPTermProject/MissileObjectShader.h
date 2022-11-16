
#pragma once
#include"stdafx.h"
#include"Shader.h"
#include"Missileobject.h"
#include"Player.h"


class CMissileObjectsShader : public CObjectsShader
{
public:
	CMissileObjectsShader();
	virtual ~CMissileObjectsShader();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual void ReleaseObjects();
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int pipelinestate);

	virtual void ReleaseUploadBuffers();
	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam, float* fTimeelpased);
	void AnimateObjects(float fTimeElapsed);
	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; };

private:


	//ID3D12Resource* m_pd3dcbGameObjects = NULL;
	//CB_GAMEOBJECT_INFO* m_pcbMappedGameObjects = NULL;

	CCubeMeshTextured* m_pMissileTexturedMesh{ NULL };
	CTexture* m_pMissileTexture{ NULL };
	CMaterial* m_pMissileMaterial{ NULL };
	CPlayer* m_pPlayer{ NULL };
	CMissleObject* pMissleObject = NULL;
	list<CGameObject*>		m_MissileList;




	const double					MaxBulletDistance = 250.f;
	int						m_MissileCount;

	/*CFireParticleShader* m_pFireParticleShader{ NULL };
	CFireParticle* m_pFireParticle{ NULL };

	CExplosionParticleShader* m_pExplosionParticleShader{ NULL };
	CExplosionParticle* m_pExplosionParticle{ NULL };*/

};
