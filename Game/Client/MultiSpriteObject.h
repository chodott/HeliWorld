#pragma once
#include"Object.h"
//#include"Shader.h"

class CMultiSpriteObjects : public CGameObject
{
public:
	CMultiSpriteObjects();
	virtual ~CMultiSpriteObjects();

	float m_fSpeed = 0.1f;
	float m_fTime = 0.0f;


	virtual void Animate(float fTimeElapsed);
};
