#pragma once
#include "GameStateBase.h"
#include "GameObject/Text.h"

#include <vector>
#include <string>
#include <SDL.h>

struct Question {
	std::string imagePath;
	std::string answer;
	int length;
};

struct Player {
	std::string avtPath;
	std::string playerName;
	int playerScore;
};

class Sprite2D;
class SpriteAnimation;
class MouseButton;
class GSPlay :
	public GameStateBase
{
public:
	GSPlay();
	~GSPlay();
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
	
	void	LoadQuestions();
	void	UpdateQuestion();
	void	UpdateScore();
	void	LoadPlayer();

private:
	std::shared_ptr<Sprite2D>	m_background, m_ques;
	std::shared_ptr<MouseButton> m_bell, m_send;
	std::shared_ptr<Text> m_textAnswer;
	std::string	m_answer;
	int m_currentPos = 0;
	int m_currentQuestionIndex;
	std::vector<Question> questions;
	int currentQuestionIndex;
	bool isBellRing;
	bool isMatLuot;
	std::vector<Player> players;
	std::vector<std::shared_ptr<Text>> m_answerText;  // Danh sách các Text để hiển thị các ký tự
	std::vector<std::shared_ptr<Sprite2D>> playerAvts; // Danh sách lưu avt
	std::vector<std::shared_ptr<Text>> playerNames; // Danh sách lưu tên
	std::vector<std::shared_ptr<Text>> playerScores; // Danh sách lưu điểm
};