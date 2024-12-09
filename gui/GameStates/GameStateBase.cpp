#include "GameStateBase.h"
#include "GSIntro.h"
#include "GSMenu.h"
#include "GSLogin.h"
#include "GSSignUp.h"
#include "GSPlay.h"
#include "GSWatingRoom.h"

GameStateBase::GameStateBase(StateType stateType) : m_stateType(stateType)
{}

std::shared_ptr<GameStateBase> GameStateBase::CreateState(StateType stt)
{
	std::shared_ptr<GameStateBase> gs = nullptr;
	switch (stt)
	{
	case StateType::STATE_INVALID:
		break;
	case StateType::STATE_INTRO:
		gs = std::make_shared<GSIntro>();
		//GSINTRO;
		break;
	case StateType::STATE_MENU:
		gs = std::make_shared<GSMenu>();
		//GSMENU
		break;
	case StateType::STATE_LOGIN:
		gs = std::make_shared<GSLogin>();
		//GSLOGIN
		break;
	case StateType::STATE_SIGNUP:
		gs = std::make_shared<GSSignUp>();
		//GSSIGNUP
		break;
	case StateType::STATE_PLAY:
		gs = std::make_shared<GSPlay>();
		//GSPLAY
		break;
	case StateType::STATE_WATINGROOM:
		gs = std::make_shared<GSWatingRoom>();
		//GSWatingRoom
		break;
	default:
		break;
	}
	return gs;
}

StateType GameStateBase::GetGameStateType()
{
	return m_stateType;
}
