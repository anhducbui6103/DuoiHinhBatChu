#pragma once
#include "GameStateBase.h"
class Sprite2D;
class GSWatingRoom : public GameStateBase
{
public:
	GSWatingRoom();
	~GSWatingRoom();

	void	Init() override;
	void	Exit() override;

	void	Pause() override;
	void	Resume() override;

	void	HandleEvents() override;
	void	HandleKeyEvents(SDL_Event& e) override;
	void	HandleTouchEvents(SDL_Event& e) override;
	void	HandleMouseMoveEvents(int x, int y) override;
	void	Update(float deltaTime) override;
	void	Draw(SDL_Renderer* renderer) override;

private:
	float	m_time;

};
