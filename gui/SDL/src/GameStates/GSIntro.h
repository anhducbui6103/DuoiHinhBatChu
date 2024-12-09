#pragma once
#include "GameStateBase.h"
class Sprite2D;
class GSIntro : public GameStateBase
{
public:
	GSIntro();
	~GSIntro();

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
	//Uint32 callback(Uint32 interval, void*param);
	std::shared_ptr<Sprite2D> m_logo;
	float	m_time;
};
