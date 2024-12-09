#include "GSWatingRoom.h"
#include "ResourceManagers.h"
#include "Define.h"
#include "Sprite2D.h"
#define INTRO_TIME 2.0f

GSWatingRoom::GSWatingRoom() : GameStateBase(StateType::STATE_INTRO), m_time(0.0f)
{
}


GSWatingRoom::~GSWatingRoom()
{
}

void GSWatingRoom::Init()
{
}

void GSWatingRoom::Exit()
{
}


void GSWatingRoom::Pause()
{
}

void GSWatingRoom::Resume()
{

}


void GSWatingRoom::HandleEvents()
{
}

void GSWatingRoom::HandleKeyEvents(SDL_Event& e)
{
}

void GSWatingRoom::HandleTouchEvents(SDL_Event& e)
{
}

void GSWatingRoom::HandleMouseMoveEvents(int x, int y)
{
}

void GSWatingRoom::Update(float deltaTime)
{
	m_time += deltaTime;
	if (m_time > INTRO_TIME)
	{
		GameStateMachine::GetInstance()->ChangeState(StateType::STATE_PLAY);
		m_time = 0;
	}
}

void GSWatingRoom::Draw(SDL_Renderer* renderer)
{
}
