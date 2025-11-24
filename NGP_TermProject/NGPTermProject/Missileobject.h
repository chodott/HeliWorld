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
	virtual void Animate(const MissileInfoPacket& prevPacket, const MissileInfoPacket& nextPacket, const float fTimeElapsed, const float value);


	void SetID(int id) { m_ID = id; }
	int GetID()	const { return m_ID; }

	void SetNetID(int id) { m_netID = id; }
	int GetNetID()	const { return m_netID; }

	inline void SetLocal(bool isLocal) { m_bLocalMissile = isLocal; }
	inline bool GetLocal() { return m_bLocalMissile; }


	virtual void Rotate(XMFLOAT3& xmf3RotationAxis, float fAngle);
	void Rotate(float fPitch, float fYaw, float fRoll);
	virtual void RotatePYR(XMFLOAT3& xmf3RotationAxis);
	virtual void Move(const XMFLOAT3& vDirection, float fSpeed);

	inline void SetServerPosition(const XMFLOAT3& xmf3Position) { m_xmf3ServerPosition = xmf3Position; }
	inline void SetPredictPosition(const XMFLOAT3& xmf3Position) { m_xmf3PredictPosition = xmf3Position; }
	XMFLOAT3 GetPredictPosition() { return m_xmf3PredictPosition; }
	inline void SetLaunched(bool isLaunched) { m_launched = isLaunched; }
	inline bool GetLaunched() { return m_launched; }
	void ApplyServerResult(bool active);
	void ApplyVisualSmoothing(const XMFLOAT3& launchPosition, float fTimeElapsed);



private:
	XMFLOAT3					m_xmf3PredictPosition;
	XMFLOAT3					m_xmf3ServerPosition;

	const int movingSpeed = 500.f;
	bool m_Collision = false;
	bool m_bActiveInServer = false;
	bool m_bLocalMissile = false;
	bool m_launched = false;
	int m_ID;
	float m_PressTime = 0;
	uint64_t m_netID = 0;
};
