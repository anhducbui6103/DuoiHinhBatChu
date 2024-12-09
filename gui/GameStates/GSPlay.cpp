#include "GSPlay.h"
#include "ResourceManagers.h" 

#include "MouseButton.h"

#include <cctype>
#include <iostream>

GSPlay::GSPlay() :GameStateBase(StateType::STATE_PLAY), m_answer("______"), currentQuestionIndex(0), m_textAnswer(nullptr), isBellRing(false)
{
	LoadQuestions();
	LoadPlayer();
}

GSPlay::~GSPlay()
{

}

void GSPlay::Init() {
	// background
	auto texture = ResourceManagers::GetInstance()->GetTexture("white.png");
	m_background = std::make_shared<Sprite2D>(texture, SDL_FLIP_NONE);
	m_background->SetSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	m_background->Set2DPosition(0, 0);

	// Picture
	texture = ResourceManagers::GetInstance()->GetTexture("question1.jpg");
	m_ques = std::make_shared<Sprite2D>(texture, SDL_FLIP_NONE);
	m_ques->SetSize(700, 400);
	m_ques->Set2DPosition(50, 50);

	auto font = ResourceManagers::GetInstance()->GetFont("Roboto-Regular.ttf", 50);
	SDL_Color textColor = { 0, 0, 0, 255 };

	// Score Board
	int startX = 900;   // Vị trí X góc trên bên trái
	int startY = 20;   // Vị trí Y góc trên bên trái
	int spacing = 243;  // Khoảng cách giữa các dòng
	

	for (size_t i = 0; i < players.size(); ++i) {
	// Tạo đối tượng Sprite 2D cho avt
	texture = ResourceManagers::GetInstance()->GetTexture(players[i].avtPath);
	auto avt = std::make_shared<Sprite2D>(texture, SDL_FLIP_NONE);
	avt->Set2DPosition(startX, startY + i * spacing);
	avt->SetSize(150, 150);
	playerAvts.push_back(avt);

	// Tạo đối tượng Text cho tên
	auto name = std::make_shared<Text>(players[i].playerName, font, textColor);
	name->Set2DPosition(startX + (avt->GetWidth()-name->GetWidth())/2, startY + i * spacing + avt->GetHeight() + 2);
	playerNames.push_back(name);

	// Tạo đối tượng Text cho điểm
	auto score = std::make_shared<Text>(std::to_string(players[i].playerScore), font, textColor);
	score->Set2DPosition(startX + 200, startY + i * spacing + (avt->GetHeight() - score->GetHeight()) / 2);
	playerScores.push_back(score);
	}

	font = ResourceManagers::GetInstance()->GetFont("SpaceMono-Regular.ttf", 50);
	textColor = { 0, 0, 0, 255 };

	// Answer Input
	m_textAnswer = std::make_shared<Text>(m_answer, font, textColor);
	m_textAnswer->Set2DPosition(150, 600);

	// Send Button
	texture = ResourceManagers::GetInstance()->GetTexture("send.png");
	m_send = std::make_shared<MouseButton>(texture, SDL_FLIP_NONE);
	m_send->Set2DPosition(400, 610);
	m_send->SetSize(50, 50);
	m_send->SetOnClick([this]() {
		printf("Button click");
		isBellRing = true;
		UpdateQuestion();
	});

	texture = ResourceManagers::GetInstance()->GetTexture("yellow_bell.png");
	m_bell = std::make_shared<MouseButton>(texture, SDL_FLIP_NONE);
	m_bell->Set2DPosition(480, 610);
	m_bell->SetSize(50, 50);
	m_bell->SetOnClick([this]() {
		printf("Ring bell");
	});
}

void GSPlay::Exit()
{

}


void GSPlay::Pause()
{

}

void GSPlay::Resume()
{

}

void GSPlay::HandleEvents()
{

}

void GSPlay::HandleKeyEvents(SDL_Event& e)
{
	if (e.type == SDL_TEXTINPUT) {
		// Get input char
		std::string inputChar = e.text.text;

		// Check valid
		const auto& currentQuestion = questions[currentQuestionIndex]; // Lấy câu hỏi hiện tại
		if (inputChar.length() == 1 && m_currentPos < currentQuestion.length) // Kiểm tra vị trí hiện tại < độ dài đáp án
		{
			if (isalpha(inputChar[0])) {
				m_answer[m_currentPos] = toupper(inputChar[0]);  // Replace '_' with the typed character
				m_currentPos++;
			}
		}
	}

	// Backspace
	if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_BACKSPACE && m_currentPos > 0) { // Kiểm tra m_currentPos > 0
	m_currentPos--;  // Move back one position
	m_answer[m_currentPos] = '_';  // Set the character back to '_'
	}
	if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
	UpdateQuestion();
	}


	// Update display text
	m_textAnswer->SetText(m_answer);
}

void GSPlay::HandleTouchEvents(SDL_Event& e)
{
	m_send->HandleTouchEvent(&e);
	m_bell->HandleTouchEvent(&e);
}

void GSPlay::HandleMouseMoveEvents(int x, int y)
{

}

void GSPlay::Update(float deltaTime)
{
}

void GSPlay::Draw(SDL_Renderer* renderer)
{
	m_background->Draw(renderer);
	m_ques->Draw(renderer);
	m_textAnswer->Draw(renderer);
	m_send->Draw(renderer);
	m_bell->Draw(renderer);

	for (const auto& name : playerNames) {
		name->Draw(renderer);
	}
	for (const auto& score : playerScores) {
		score->Draw(renderer);
	}
	for (const auto& avt : playerAvts) {
		avt->Draw(renderer);
	}
}

void GSPlay::UpdateQuestion() {
	// Kiểm tra nếu chưa nhập đủ ký tự
	const auto& currentQuestion = questions[currentQuestionIndex];
	if (m_currentPos < currentQuestion.length) {
		std::cout << "Please complete the answer before proceeding to the next question.\n";
		return; // Thoát hàm nếu chưa nhập đủ ký tự
	}

	// Cập nhật điểm trước khi chuyển sang câu hỏi mới
	UpdateScore();

	// Chuyển sang câu hỏi tiếp theo
	currentQuestionIndex++;

	if (currentQuestionIndex >= questions.size()) {
		currentQuestionIndex = 0;
		// TODO: Logic endgame nếu cần
	}

	// Lấy câu hỏi hiện tại
	const auto& newQuestion = questions[currentQuestionIndex];

	// Cập nhật hình ảnh câu hỏi
	auto texture = ResourceManagers::GetInstance()->GetTexture(newQuestion.imagePath);
	m_ques->SetTexture(texture);

	// Reset đáp án
	m_answer = std::string(newQuestion.length, '_');
	m_currentPos = 0;

	// Cập nhật hiển thị
	m_textAnswer->SetText(m_answer);
}

void GSPlay::UpdateScore() {
	const auto& currentQuestion = questions[currentQuestionIndex];

	// Chuyển cả answer và m_answer về chữ thường để so sánh không phân biệt chữ hoa và chữ thường
	std::string correctAnswer = currentQuestion.answer;
	std::string playerAnswer = m_answer;

	// So sánh câu trả lời của người chơi với câu trả lời đúng
	if (playerAnswer == correctAnswer) {
		players[0].playerScore += 10;
		std::cout << "Correct! Your score: " << players[0].playerScore << std::endl;
	}
	else {
		std::cout << "Incorrect. Your score: " << players[0].playerScore << std::endl;
	}
	playerScores[0]->SetText(std::to_string(players[0].playerScore));
}

void GSPlay::LoadQuestions() {
	questions = {
		{"question1.jpg", "BAOHAM", 6},
		{"question2.jpg", "COLOA", 5},
		{"question3.jpg", "GIAYBAC", 7},
		{"question4.jpg", "BACHTHU", 7},
		{"question5.jpg", "BIOI", 4},
		{"question6.jpg", "GAUNGUA", 7},
		{"question7.jpg", "BAIBAC", 6},
	};
}

void GSPlay::LoadPlayer() {
	players = {
		{"avt.jpg", "Duc 1", 0},
		{"avt.jpg", "Duc 2", 30},
		{"avt.jpg", "Duc 3", 50},
	};
}