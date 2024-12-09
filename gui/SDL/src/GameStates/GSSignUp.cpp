#include "GSSignUp.h"
#include "ResourceManagers.h"

GSSignUp::GSSignUp() : GameStateBase(StateType::STATE_LOGIN), m_usernameButton(nullptr), m_passwordButton(nullptr), m_signUp(nullptr), m_usernameText(nullptr), m_passwordText(nullptr), m_username(""), m_password("") {}

GSSignUp::~GSSignUp() {}

void GSSignUp::Init()
{
    // background
    auto texture = ResourceManagers::GetInstance()->GetTexture("white.png");
    m_background = std::make_shared<Sprite2D>(texture, SDL_FLIP_NONE);
    m_background->SetSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    m_background->Set2DPosition(0, 0);

    // Title
    auto font = ResourceManagers::GetInstance()->GetFont("OpenSans_Condensed-Regular.ttf", 80);
    SDL_Color textColor = { 0, 0, 0, 255 };
    m_signUp = std::make_shared<Text>("Sign up", font, textColor);
    m_signUp->Set2DPosition(400, 100);

    // Login Button
    m_signupButton = std::make_shared<MouseButton>(texture, SDL_FLIP_NONE);
    m_signupButton->SetSize(200, 70);
    m_signupButton->Set2DPosition(400, 570);
    m_signupButton->SetOnClick([this]() {
        m_isSignUpValid = CheckCredentials(m_username, m_password, m_confirmPassword); // Kiểm tra tài khoản

        if (m_isSignUpValid) {
            printf("Sign up successful!\n");
            GameStateMachine::GetInstance()->ChangeState(StateType::STATE_LOGIN);
            // Thực hiện các hành động khi login thành công
        }
        else {
            printf("Invalid.\n");
            // Xử lý khi tài khoản không hợp lệ
        }
        });

    font = ResourceManagers::GetInstance()->GetFont("OpenSans-Regular.ttf", 50);
    textColor = { 0, 0, 0, 255 };
    m_signupText = std::make_shared<Text>("Sign up", font, textColor);
    m_signupText->Set2DPosition(m_signupButton->Get2DPosition().x + (m_signupButton->GetWidth() - m_signupText->GetWidth()) / 2, m_signupButton->Get2DPosition().y + (m_signupButton->GetHeight() - m_signupText->GetHeight()) / 2);

    m_signupRect = {
        static_cast<int>(m_signupButton->Get2DPosition().x),
        static_cast<int>(m_signupButton->Get2DPosition().y),
        static_cast<int>(m_signupButton->GetWidth()),
        static_cast<int>(m_signupButton->GetHeight())
    };

    // UserName Input
    m_usernameButton = std::make_shared<MouseButton>(texture, SDL_FLIP_NONE);
    m_usernameButton->SetSize(500, 70);
    m_usernameButton->Set2DPosition(400, 230);
    m_usernameButton->SetOnClick([this]() {
        printf("Username button clicked!\n");
        m_username = "";
        m_usernameText->SetText(m_username);
        m_currentFocus = InputFocus::USERNAME;
        });
    textColor = { 128, 128, 128, 255 };
    m_usernameText = std::make_shared<Text>("username", font, textColor);
    m_usernameText->Set2DPosition(420, m_usernameButton->Get2DPosition().y + (m_usernameButton->GetHeight() - m_usernameText->GetHeight()) / 2);

    m_usernameRect = {
        static_cast<int>(m_usernameButton->Get2DPosition().x),
        static_cast<int>(m_usernameButton->Get2DPosition().y),
        static_cast<int>(m_usernameButton->GetWidth()),
        static_cast<int>(m_usernameButton->GetHeight())
    };

    // Password Input
    m_passwordButton = std::make_shared<MouseButton>(texture, SDL_FLIP_NONE);
    m_passwordButton->SetSize(500, 70);
    m_passwordButton->Set2DPosition(400, 340);
    m_passwordButton->SetOnClick([this]() {
        printf("Password button clicked!\n");
        m_password = "";
        m_passwordDisplay = "";
        m_passwordText->SetText(m_password);
        m_currentFocus = InputFocus::PASSWORD;
        });

    m_passwordText = std::make_shared<Text>("password", font, textColor);
    m_passwordText->Set2DPosition(420, m_passwordButton->Get2DPosition().y + (m_passwordButton->GetHeight() - m_passwordText->GetHeight()) / 2);

    m_passwordRect = {
        static_cast<int>(m_passwordButton->Get2DPosition().x),
        static_cast<int>(m_passwordButton->Get2DPosition().y),
        static_cast<int>(m_passwordButton->GetWidth()),
        static_cast<int>(m_passwordButton->GetHeight())
    };

    // Password Input
    m_confirmPasswordButton = std::make_shared<MouseButton>(texture, SDL_FLIP_NONE);
    m_confirmPasswordButton->SetSize(500, 70);
    m_confirmPasswordButton->Set2DPosition(400, 450);
    m_confirmPasswordButton->SetOnClick([this]() {
        printf("Password button clicked!\n");
        m_confirmPassword = "";
        m_confirmPasswordDisplay = "";
        m_confirmPasswordText->SetText(m_confirmPassword);
        m_currentFocus = InputFocus::CONFIRMPASS;
        });

    m_confirmPasswordText = std::make_shared<Text>("confirm pass", font, textColor);
    m_confirmPasswordText->Set2DPosition(420, m_confirmPasswordButton->Get2DPosition().y + (m_confirmPasswordButton->GetHeight() - m_confirmPasswordText->GetHeight()) / 2);

    m_confirmPasswordRect = {
        static_cast<int>(m_confirmPasswordButton->Get2DPosition().x),
        static_cast<int>(m_confirmPasswordButton->Get2DPosition().y),
        static_cast<int>(m_confirmPasswordButton->GetWidth()),
        static_cast<int>(m_confirmPasswordButton->GetHeight())
    };
}

void GSSignUp::Exit()
{
    printf("Exiting Menu State...\n");
}

void GSSignUp::Pause()
{
}

void GSSignUp::Resume()
{
}

void GSSignUp::HandleEvents()
{
}

void GSSignUp::HandleKeyEvents(SDL_Event& e)
{
    if (e.type == SDL_KEYDOWN)
    {
        // Nếu đang focus vào username
        if (m_currentFocus == InputFocus::USERNAME)
        {
            // Kiểm tra phím bấm
            if (e.key.keysym.sym == SDLK_BACKSPACE && !m_username.empty()) {
                // Xóa ký tự cuối cùng nếu có
                m_username.pop_back();
            }
            else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_TAB) {
                // Chuyển focus sang password khi nhấn Enter
                m_currentFocus = InputFocus::PASSWORD;
                m_password = ""; // Reset password khi chuyển sang ô password
                m_passwordDisplay = ""; // Reset mật khẩu hiển thị
                m_passwordText->SetText(m_passwordDisplay);
                if (m_username.empty()) {
                    m_usernameText->SetColor({ 128,128,128,255 });
                    m_usernameText->SetText("username");
                }
            }
            else if (e.key.keysym.sym > SDLK_SPACE && e.key.keysym.sym <= SDLK_z) {
                // Thêm ký tự vào username (chỉ xử lý các ký tự hợp lệ)
                m_username += static_cast<char>(e.key.keysym.sym);
            }

            // Cập nhật lại text hiển thị
            if (!m_username.empty()) {
                m_usernameText->SetColor({ 0,0,0,255 });
                m_usernameText->SetText(m_username);
            }
        }
        // Nếu đang focus vào password
        else if (m_currentFocus == InputFocus::PASSWORD)
        {
            // Kiểm tra phím bấm
            if (e.key.keysym.sym == SDLK_BACKSPACE && !m_password.empty()) {
                // Xóa ký tự cuối cùng nếu có
                m_password.pop_back();
                m_passwordDisplay.pop_back(); // Cập nhật mật khẩu hiển thị dưới dạng dấu sao
            }
            else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_TAB) {
                // Chuyển focus sang password khi nhấn Enter hoặc Tab
                m_currentFocus = InputFocus::CONFIRMPASS;
                m_confirmPassword = ""; // Reset password khi chuyển sang ô password
                m_confirmPasswordDisplay = ""; // Reset mật khẩu hiển thị
                m_confirmPasswordText->SetText(m_confirmPasswordDisplay);
                if (m_password.empty()) {
                    m_passwordText->SetColor({ 128,128,128,255 });
                    m_passwordText->SetText("password");
                }
            }
            else if (e.key.keysym.sym > SDLK_SPACE && e.key.keysym.sym <= SDLK_z) {
                // Thêm ký tự vào password
                m_password += static_cast<char>(e.key.keysym.sym);
                m_passwordDisplay += '*'; // Thêm dấu sao vào mật khẩu hiển thị
            }

            // Cập nhật lại text hiển thị mật khẩu dạng dấu sao
            if (!m_password.empty()) {
                m_passwordText->SetColor({ 0,0,0,255 });
                m_passwordText->SetText(m_passwordDisplay);
            }
        }
        // Nếu đang focus vào Confirm Password
        else if (m_currentFocus == InputFocus::CONFIRMPASS)
        {
            // Kiểm tra phím bấm
            if (e.key.keysym.sym == SDLK_BACKSPACE && !m_confirmPassword.empty()) {
                // Xóa ký tự cuối cùng nếu có
                m_confirmPassword.pop_back();
                m_confirmPasswordDisplay.pop_back(); // Cập nhật mật khẩu hiển thị dưới dạng dấu sao
            }
            else if (e.key.keysym.sym == SDLK_RETURN) {
                m_isSignUpValid = CheckCredentials(m_username, m_password, m_confirmPassword); // Kiểm tra tài khoản

                if (m_isSignUpValid) {
                    printf("sign up successful!\n");
                    GameStateMachine::GetInstance()->ChangeState(StateType::STATE_LOGIN);
                }
                else {
                    printf("Invalid.\n");
                }
            }
            else if (e.key.keysym.sym > SDLK_SPACE && e.key.keysym.sym <= SDLK_z) {
                // Thêm ký tự vào password
                m_confirmPassword += static_cast<char>(e.key.keysym.sym);
                m_confirmPasswordDisplay += '*'; // Thêm dấu sao vào mật khẩu hiển thị
            }

            // Cập nhật lại text hiển thị mật khẩu dạng dấu sao
            m_confirmPasswordText->SetColor({ 0,0,0,255 });
            m_confirmPasswordText->SetText(m_confirmPasswordDisplay);
        }
    }
}



void GSSignUp::HandleTouchEvents(SDL_Event& e)
{
    // Xử lý sự kiện cho hai nút
    m_usernameButton->HandleTouchEvent(&e);
    m_passwordButton->HandleTouchEvent(&e);
    m_confirmPasswordButton->HandleTouchEvent(&e);
    m_signupButton->HandleTouchEvent(&e);

    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) // Khi click chuột
    {
        int mouseX = e.button.x;
        int mouseY = e.button.y;

        // Kiểm tra nếu chuột không nằm trong vùng của username hoặc password
        bool isOutsideUsername =
            mouseX < m_usernameRect.x ||
            mouseX > m_usernameRect.x + m_usernameRect.w ||
            mouseY < m_usernameRect.y ||
            mouseY > m_usernameRect.y + m_usernameRect.h;

        bool isOutsidePassword =
            mouseX < m_passwordRect.x ||
            mouseX > m_passwordRect.x + m_passwordRect.w ||
            mouseY < m_passwordRect.y ||
            mouseY > m_passwordRect.y + m_passwordRect.h;

        bool isOutsideConfirmPassword =
            mouseX < m_confirmPasswordRect.x ||
            mouseX > m_confirmPasswordRect.x + m_confirmPasswordRect.w ||
            mouseY < m_confirmPasswordRect.y ||
            mouseY > m_confirmPasswordRect.y + m_confirmPasswordRect.h;

        // Nếu không còn focus vào username hoặc password, confirmPass
        if (isOutsideUsername && isOutsidePassword && isOutsideConfirmPassword)
        {
            if (m_currentFocus == InputFocus::USERNAME && m_username.empty()) {
                m_usernameText->SetColor({ 128,128,128,255 });
                m_usernameText->SetText("username");
            }
            if (m_currentFocus == InputFocus::PASSWORD && m_password.empty()) {
                m_passwordText->SetColor({ 128,128,128,255 });
                m_passwordText->SetText("password");
            }
            if (m_currentFocus == InputFocus::CONFIRMPASS && m_confirmPassword.empty()) {
                m_confirmPasswordText->SetColor({ 128,128,128,255 });
                m_confirmPasswordText->SetText("confirm pass");
            }
            m_currentFocus = InputFocus::NONE;
            printf("Focus is now NONE\n");
        }
        else {
            // Nếu click vào ô username
            if (!isOutsideUsername && m_currentFocus != InputFocus::USERNAME) {
                m_username = "";
                m_usernameText->SetText(m_username);
                m_currentFocus = InputFocus::USERNAME;
                if (m_password.empty()) {
                    m_passwordText->SetColor({ 128,128,128,255 });
                    m_passwordText->SetText("password");
                }
                if (m_confirmPassword.empty()) {
                    m_confirmPasswordText->SetColor({ 128,128,128,255 });
                    m_confirmPasswordText->SetText("confirm pass");
                }
            }
            // Nếu click vào ô password
            if (!isOutsidePassword && m_currentFocus != InputFocus::PASSWORD) {
                m_password = "";
                m_passwordDisplay = "";
                m_passwordText->SetText(m_password);
                m_currentFocus = InputFocus::PASSWORD;
                if (m_username.empty()) {
                    m_usernameText->SetColor({ 128,128,128,255 });
                    m_usernameText->SetText("username");
                }
                if (m_confirmPassword.empty()) {
                    m_confirmPasswordText->SetColor({ 128,128,128,255 });
                    m_confirmPasswordText->SetText("confirm pass");
                }
            }
            // Nếu click vào ô confirmPass
            if (!isOutsideConfirmPassword && m_currentFocus != InputFocus::CONFIRMPASS) {
                m_confirmPassword = "";
                m_confirmPasswordDisplay = "";
                m_confirmPasswordText->SetText(m_confirmPassword);
                m_currentFocus = InputFocus::CONFIRMPASS;
                if (m_username.empty()) {
                    m_usernameText->SetColor({ 128,128,128,255 });
                    m_usernameText->SetText("username");
                }
                if (m_password.empty()) {
                    m_passwordText->SetColor({ 128,128,128,255 });
                    m_passwordText->SetText("password");
                }
            }
        }
    }
}


void GSSignUp::HandleMouseMoveEvents(int x, int y)
{
}

void GSSignUp::Update(float deltaTime)
{
}

void GSSignUp::Draw(SDL_Renderer* renderer)
{
    m_background->Draw(renderer);

    // Draw the title text
    m_signUp->Draw(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Input Username
    m_usernameButton->Draw(renderer);
    m_usernameText->Draw(renderer);
    SDL_RenderDrawRect(renderer, &m_usernameRect);

    // Input Password
    m_passwordButton->Draw(renderer);
    m_passwordText->Draw(renderer);
    SDL_RenderDrawRect(renderer, &m_passwordRect);

    // Input ConfirmPass
    m_confirmPasswordButton->Draw(renderer);
    m_confirmPasswordText->Draw(renderer);
    SDL_RenderDrawRect(renderer, &m_confirmPasswordRect);

    // Sign up Button
    m_signupButton->Draw(renderer);
    m_signupText->Draw(renderer);
    SDL_RenderDrawRect(renderer, &m_signupRect);
}

bool GSSignUp::CheckCredentials(const std::string& username, const std::string& password, const std::string& confirmPassword)
{
    if (username.empty() || password.empty()) {
        return false;
    }
    if (password != confirmPassword) {
        return false;
    }
    // Simulate registration success for now
    return true;
}