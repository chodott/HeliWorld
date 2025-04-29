#pragma once
#pragma warning(disable : 26495)

#include "stdafx.h"
#include "SCPacket.h"

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20


class GameObject {
public:
	GameObject();
	XMFLOAT4X4 m_xmf4x4World;
	BoundingOrientedBox m_xmOOBB;
	XMFLOAT3 m_xmf3MovingDirection;

	XMFLOAT3					m_xmf3Position;
	XMFLOAT3					m_xmf3Right;
	XMFLOAT3					m_xmf3Up;
	XMFLOAT3					m_xmf3Look;


	void RecalculateLook() { m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)); }
	void RecalculateRight() { m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)); }


	float           			m_fPitch = 0.f;
	float           			m_fYaw = 0.f;
	float           			m_fRoll = 0.f;

	float						m_fOldxPos = 0.f;
	float						m_fOldyPos = 0.f;
	float						m_fOldzPos = 0.f;

	float						m_fxPos = 0.f;
	float						m_fyPos = 0.f;
	float						m_fzPos = 0.f;


	bool						m_bActive = false;//active
	bool						shouldDeactivated = false;
	void ShouldDeactive() { shouldDeactivated = true; }
	void Deactivate() { m_bActive = false; shouldDeactivated = false; }

	inline XMFLOAT3 GetCurPos() { return XMFLOAT3(m_fxPos, m_fyPos, m_fzPos); }
	inline XMFLOAT3 GetCurRot() { return XMFLOAT3(m_fPitch, m_fYaw, m_fRoll); }

	void Rotate(float Pitch, float Yaw, float Roll);
	void SetPosition(float x, float y, float z);
	void SetActive(bool active) { m_bActive = active; }
	bool IsActive() { return m_bActive; }


	void MoveOOBB(XMFLOAT3& position) { m_xmOOBB.Center = position; }
	void InitOOBB() { m_xmOOBB = BoundingOrientedBox{ XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(10.f, 10.f, 10.f), XMFLOAT4(0.f, 0.f, 0.f, 1.f) }; }
	void InitOOBB(XMFLOAT3& xmCenter, XMFLOAT3& xmExtents, XMFLOAT4& xmOrientation) { m_xmOOBB = BoundingOrientedBox(xmCenter, xmExtents, xmOrientation); }

	BoundingOrientedBox GetBoundingBox() { return m_xmOOBB; }

};

class CMissileObject;

class CPlayer : public GameObject
{
public:
	CPlayer();
	~CPlayer();

	float m_fFriction;
	const int maxHp = 100;
	int m_nHp = maxHp;

	const int maxMissileNum = 8;
	CMissileObject* m_pMissiles[8];

	void Move(const XMFLOAT3& xmf3Shift);
	void Rotate(float x, float y, float z);
	void LaunchMissile(int16_t missileNum);
	void UpdateMissiles(float elapsedTime);
	void Update(float elapsedTime, int connectedClients);
	void Reset(int playerNum);

	const float movingSpeed = 200.f;

	PlayerKeyPacket keyPacket;

	XMFLOAT3 initialPos[4]{ {100,400,100},{900, 400, 900},{900.0f, 400.0f, 100.0f},{100.0f, 400.0f, 900.0f} };

	XMFLOAT3 initialRot[4]{ {0,90,0},{0,90,0},{0,90,0},{0,90,0} };

private:
	// Key bindings
	unsigned char option0 = 0x01;	// 0000 0001 
	unsigned char option1 = 0x02;	// 0000 0010
	unsigned char option2 = 0x04;	// 0000 0100
	unsigned char option3 = 0x08;	// 0000 1000
	unsigned char option4 = 0x10;	// 0001 0000
	unsigned char option5 = 0x20;	// 0010 0000
	unsigned char option6 = 0x40;	// 0100 0000
	unsigned char option7 = 0x80;	// 1000 0000

	const float maxPitch = 89.f;
	const float maxRoll = 20.f;
	const float missileLifeSpan = 6.f;

};

class CMissileObject : public GameObject
{
public:
	int m_playerNumber = 0;
	int damage = 10;
	const float movingSpeed = 300.f;
	bool bMustKill = false;

	float m_fLifeSpan = 6.f;
	
	void Move(float elapsedTime);
	void Reset();
};

class CItemObject : public GameObject
{
public:
	int healAmount = 0;
};

