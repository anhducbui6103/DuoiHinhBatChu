#pragma once
#include "GameStateBase.h"
#include "GameObject/Text.h"
#include "GameObject/MouseButton.h"

enum class InputFocus
{
    NONE,       // Không focus vào trường nào
    USERNAME,   // Focus vào trường username
    PASSWORD,   // Focus vào trường password
    CONFIRMPASS // Focus vào trường confirm password
};

class MouseButton;
class GSSignUp : public GameStateBase
{
private:
    std::shared_ptr<Sprite2D> m_background;
    std::shared_ptr<MouseButton> m_usernameButton, m_passwordButton, m_confirmPasswordButton, m_signupButton;
    SDL_Rect m_usernameRect, m_passwordRect, m_confirmPasswordRect, m_signupRect;
    std::shared_ptr<Text> m_usernameText, m_passwordText, m_confirmPasswordText, m_signUp, m_signupText;
    std::string m_username, m_password, m_passwordDisplay, m_confirmPassword, m_confirmPasswordDisplay;
    InputFocus m_currentFocus;
    bool m_isSignUpValid;

public:
    GSSignUp();
    ~GSSignUp();

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
    bool CheckCredentials(const std::string& username, const std::string& password, const std::string& confirmPassword);
};
