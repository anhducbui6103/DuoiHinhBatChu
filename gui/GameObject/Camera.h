#pragma once
#include "CMath.h"
#include "SDL.h"

#include "Define.h"
#include"BaseObject.h"

class Camera : public CSingleton<Camera>
{
public:
	Camera() { m_ViewBox = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT }; }
	SDL_Rect GetViewBox();
	Vector2 GetPosition();
	void SetTarget(std::shared_ptr<BaseObject> target);
	void Update(float deltaTime);
	void SetLevelDimension(int width, int height);
	int GetLevelWidth();
	int GetLevelHeight();
private:
	std::shared_ptr<BaseObject> m_target;
	Vector2 m_Position;
	SDL_Rect m_ViewBox;
	float m_Smooth = 5.0f;
	int m_levelWidth;
	int m_levelHeight;
};