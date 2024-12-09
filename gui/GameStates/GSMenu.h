#pragma once
#include "GameStateBase.h"
#include "GameObject/Text.h"
#include "GameObject/MouseButton.h"

class MouseButton;
class GSMenu : public GameStateBase
{
private:
    std::shared_ptr<Sprite2D> m_background;
    std::shared_ptr<MouseButton> m_playButton, m_exitButton;
    SDL_Rect m_playRect, m_exitRect;
    std::shared_ptr<Text> m_titleText, m_playText, m_exitText;

public:
    GSMenu();
    ~GSMenu();

    // Implement abstract methods
    void Init() override;
    void Exit() override;
    void Pause() override;
    void Resume() override;
    void HandleEvents() override;
    void HandleKeyEvents(SDL_Event& e) override;
    void HandleTouchEvents(SDL_Event& e) override;
    void HandleMouseMoveEvents(int x, int y) override;
    void Update(float deltaTime) override;
    void Draw(SDL_Renderer* renderer) override;

};
