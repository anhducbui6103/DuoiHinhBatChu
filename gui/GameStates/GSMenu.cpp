#include "GSMenu.h"
#include "ResourceManagers.h"

GSMenu::GSMenu() : GameStateBase(StateType::STATE_MENU), m_playButton(nullptr), m_exitButton(nullptr), m_titleText(nullptr), m_playText(nullptr), m_exitText(nullptr){}

GSMenu::~GSMenu() {}

void GSMenu::Init()
{
    // background
    auto texture = ResourceManagers::GetInstance()->GetTexture("white.png");
    m_background = std::make_shared<Sprite2D>(texture, SDL_FLIP_NONE);
    m_background->SetSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    m_background->Set2DPosition(0, 0);

    // Title
    auto font = ResourceManagers::GetInstance()->GetFont("OpenSans_Condensed-Regular.ttf", 100);
    SDL_Color textColor = { 255, 0, 0, 255 };
    m_titleText = std::make_shared<Text>("Catch Phrase", font, textColor);
    m_titleText->Set2DPosition((SCREEN_WIDTH - m_titleText->GetWidth()) / 2, SCREEN_HEIGHT / 2 - 250);

    // Play Button
    m_playButton = std::make_shared<MouseButton>(texture, SDL_FLIP_NONE);
    m_playButton->SetSize(200, 70);
    m_playButton->Set2DPosition(SCREEN_WIDTH / 2 - m_playButton->GetWidth() / 2, 300);
    m_playButton->SetOnClick([this]() {
        printf("Play button clicked!\n");
        GameStateMachine::GetInstance()->ChangeState(StateType::STATE_WATINGROOM);
        });

    font = ResourceManagers::GetInstance()->GetFont("OpenSans-Regular.ttf", 50);
    textColor = { 0, 0, 0, 255 };
    m_playText = std::make_shared<Text>("PLAY", font, textColor);
    m_playText->Set2DPosition(m_playButton->Get2DPosition().x+(m_playButton->GetWidth() - m_playText->GetWidth()) / 2, m_playButton->Get2DPosition().y + (m_playButton->GetHeight() - m_playText->GetHeight()) / 2);

    m_playRect = {
        static_cast<int>(m_playButton->Get2DPosition().x),
        static_cast<int>(m_playButton->Get2DPosition().y),
        static_cast<int>(m_playButton->GetWidth()),
        static_cast<int>(m_playButton->GetHeight())
    };

    // Exit Button
    m_exitButton = std::make_shared<MouseButton>(texture, SDL_FLIP_NONE);
    m_exitButton->SetSize(200, 70);
    m_exitButton->Set2DPosition(SCREEN_WIDTH / 2 - m_exitButton->GetWidth() / 2, 400);
    m_exitButton->SetOnClick([this]() {
        printf("Exit button clicked!\n");
        SDL_QUIT;
        exit(0);
        });

    m_exitText = std::make_shared<Text>("EXIT", font, textColor);
    m_exitText->Set2DPosition(m_exitButton->Get2DPosition().x + (m_exitButton->GetWidth() - m_exitText->GetWidth()) / 2, m_exitButton->Get2DPosition().y + (m_exitButton->GetHeight() - m_exitText->GetHeight()) / 2);

    m_exitRect = {
        static_cast<int>(m_exitButton->Get2DPosition().x),
        static_cast<int>(m_exitButton->Get2DPosition().y),
        static_cast<int>(m_exitButton->GetWidth()),
        static_cast<int>(m_exitButton->GetHeight())
    };
}

void GSMenu::Exit()
{
    printf("Exiting Menu State...\n");
}

void GSMenu::Pause()
{
}

void GSMenu::Resume()
{
}

void GSMenu::HandleEvents()
{
}

void GSMenu::HandleKeyEvents(SDL_Event& e)
{
}

void GSMenu::HandleTouchEvents(SDL_Event& e)
{
    m_playButton->HandleTouchEvent(&e);
    m_exitButton->HandleTouchEvent(&e);
}

void GSMenu::HandleMouseMoveEvents(int x, int y)
{
}

void GSMenu::Update(float deltaTime)
{
}

void GSMenu::Draw(SDL_Renderer* renderer)
{
    m_background->Draw(renderer);

    // Draw the title text
    m_titleText->Draw(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    // Play Button
    m_playButton->Draw(renderer);
    m_playText->Draw(renderer);
    SDL_RenderDrawRect(renderer, &m_playRect);

    // Exit Button
    m_exitButton->Draw(renderer);
    m_exitText->Draw(renderer);
    SDL_RenderDrawRect(renderer, &m_exitRect);

}
