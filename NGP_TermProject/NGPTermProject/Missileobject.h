#pragma once
#include "Object.h"
#include "Shader.h"

class CMissleObject : public CGameObject
{
public:
	CMissleObject();
	virtual ~CMissleObject();

	virtual void AnimateObject(float fElapsedTime);

	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	void SetCollision(bool Collision) { m_Collision = Collision; }
	bool GetCollision()	const { return m_Collision; }
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, MissileInfoPacket& prevPacket, MissileInfoPacket& nextPacket, float value);
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, MissileInfoPacket* MissilePacket);


	void SetID(int id) { m_ID = id; }
	int GetID()	const { return m_ID; }

	virtual void Rotate(XMFLOAT3& xmf3RotationAxis, float fAngle);
	void Rotate(float fPitch, float fYaw, float fRoll);
	virtual void RotatePYR(XMFLOAT3& xmf3RotationAxis);
	virtual void Move(XMFLOAT3& vDirection, float fSpeed);

	inline void SetServerPosition(const XMFLOAT3& xmf3Position) { m_xmf3ServerPosition = xmf3Position; }
	inline void SetRealPosition(const XMFLOAT3& xmf3Position) { m_xmf3RealPosition = xmf3Position; }
	XMFLOAT3 GetRealPosition() { return m_xmf3RealPosition; }

	bool bActiveInServer = false;
	bool bLocalMissile = false;

private:
	XMFLOAT3					m_xmf3RealPosition;
	XMFLOAT3					m_xmf3ServerPosition;

	const int movingSpeed = 500.f;
	bool m_Collision = false;
	int m_ID;
	float m_PressTime = 0;
};
