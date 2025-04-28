#pragma once
#include "Object.h"
#include "Shader.h"

class CMissleObject : public CGameObject
{
public:
	CMissleObject();
	virtual ~CMissleObject();

	//virtual void Move(XMFLOAT3& vDirection, float fSpeed);
	//void Rotate(XMFLOAT3& xmf3RotationAxis, float fAngle);

	virtual void AnimateObject(float fElapsedTime);

	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	void SetCollision(bool Collision) { m_Collision = Collision; }
	bool GetCollision()	const { return m_Collision; }
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, MissileInfoPacket* MissilePacket);
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, PlayerInfoPacket* MissilePacket);


	void SetID(int id) { m_ID = id; }
	int GetID()	const { return m_ID; }

	virtual void Rotate(XMFLOAT3& xmf3RotationAxis, float fAngle);
	void Rotate(float fPitch, float fYaw, float fRoll);
	virtual void RotatePYR(XMFLOAT3& xmf3RotationAxis);
	virtual void Move(XMFLOAT3& vDirection, float fSpeed);

	const int movingSpeed = 300.f;
	bool bServerLife = false;
	bool bLocalMissile = false;

private:
	bool m_Collision = false;
	int m_ID;
	float m_PressTime = 0;
	//bool b_Active = false;
};
