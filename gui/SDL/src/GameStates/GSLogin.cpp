#include "GSLogin.h"
#include "ResourceManagers.h"

GSLogin::GSLogin() : GameStateBase(StateType::STATE_LOGIN), m_username(""), m_password(""){}

GSLogin::~GSLogin() {}

void GSLogin::Init()
{
    // background
    auto texture = ResourceManagers::GetInstance()->GetTexture("white.png");
    m_background = std::make_shared<Sprite2D>(texture, SDL_FLIP_NONE);
    m_background->SetSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    m_background->Set2DPosition(0, 0);

    // Title
    auto font = ResourceManagers::GetInstance()->GetFont("OpenSans_Condensed-Regular.ttf", 80);
    SDL_Color textColor = { 0, 0, 0, 255 };
    m_login = std::make_shared<Text>("Log in", font, textColor);
    m_login->Set2DPosition(400, 120);

    // Login Button
    m_loginButton = std::make_shared<MouseButton>(texture, SDL_FLIP_NONE);
    m_loginButton->SetSize(200, 70);
    m_loginButton->Set2DPosition(700, 480);
    m_loginButton->SetOnClick([this]() {
        m_isLoginValid = CheckCredentials(m_username, m_password); // Kiểm tra tài khoản

        if (m_isLoginValid) {
            printf("Login successful!\n");
            GameStateMachine::GetInstance()->ChangeState(StateType::STATE_MENU);
            // Thực hiện các hành động khi login thành công
        }
        else {
            printf("Invalid username or password.\n");
            // Xử lý khi tài khoản không hợp lệ
        }
        });

    font = ResourceManagers::GetInstance()->GetFont("OpenSans-Regular.ttf", 50);
    textColor = { 0, 0, 0, 255 };
    m_loginText = std::make_shared<Text>("Log in", font, textColor);
    m_loginText->Set2DPosition(m_loginButton->Get2DPosition().x + (m_loginButton->GetWidth() - m_loginText->GetWidth()) / 2, m_loginButton->Get2DPosition().y + (m_loginButton->GetHeight() - m_loginText->GetHeight()) / 2);

    m_loginRect = {
        static_cast<int>(m_loginButton->Get2DPosition().x),
        static_cast<int>(m_loginButton->Get2DPosition().y),
        static_cast<int>(m_loginButton->GetWidth()),
        static_cast<int>(m_loginButton->GetHeight())
    };

    // Sign up Button
    m_signupButton = std::make_shared<MouseButton>(texture, SDL_FLIP_NONE);
    m_signupButton->SetSize(200, 70);
    m_signupButton->Set2DPosition(400, 480);
    m_signupButton->SetOnClick([this]() {
        printf("Sign up button clicked!\n");
        GameStateMachine::GetInstance()->ChangeState(StateType::STATE_SIGNUP);
        });

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
    m_usernameButton->Set2DPosition(400, 250);
    m_usernameButton->SetOnClick([this]() {
        printf("Username button clicked!\n");
        m_username = "";
        m_usernameText->SetText(m_username);
        m_currentFocus = InputFieldFocus::USERNAME;
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
    m_passwordButton->Set2DPosition(400, 360);
    m_passwordButton->SetOnClick([this]() {
        printf("Password button clicked!\n");
        m_password = "";
        m_passwordDisplay = "";
        m_passwordText->SetText(m_password);
        m_currentFocus = InputFieldFocus::PASSWORD;
        });

    m_passwordText = std::make_shared<Text>("password", font, textColor);
    m_passwordText->Set2DPosition(420, m_passwordButton->Get2DPosition().y + (m_passwordButton->GetHeight() - m_passwordText->GetHeight()) / 2);

    m_passwordRect = {
        static_cast<int>(m_passwordButton->Get2DPosition().x),
        static_cast<int>(m_passwordButton->Get2DPosition().y),
        static_cast<int>(m_passwordButton->GetWidth()),
        static_cast<int>(m_passwordButton->GetHeight())
    };
}

void GSLogin::Exit()
{
    printf("Exiting Menu State...\n");
}

void GSLogin::Pause()
{
}

void GSLogin::Resume()
{
}

void GSLogin::HandleEvents()
{
}

void GSLogin::HandleKeyEvents(SDL_Event& e)
{
    if (e.type == SDL_KEYDOWN)
    {
        // Nếu đang focus vào username
        if (m_currentFocus == InputFieldFocus::USERNAME)
        {
            // Kiểm tra phím bấm
            if (e.key.keysym.sym == SDLK_BACKSPACE && !m_username.empty()) {
                // Xóa ký tự cuối cùng nếu có
                m_username.pop_back();
            }
            else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_TAB) {
                // Chuyển focus sang password khi nhấn Enter hoặc Tab
                m_currentFocus = InputFieldFocus::PASSWORD;
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
            if(!m_username.empty()) {
                m_usernameText->SetColor({ 0,0,0,255 });
                m_usernameText->SetText(m_username);
            }
        }
        // Nếu đang focus vào password
        else if (m_currentFocus == InputFieldFocus::PASSWORD)
        {
            // Kiểm tra phím bấm
            if (e.key.keysym.sym == SDLK_BACKSPACE && !m_password.empty()) {
                // Xóa ký tự cuối cùng nếu có
                m_password.pop_back();
                m_passwordDisplay.pop_back(); // Cập nhật mật khẩu hiển thị dưới dạng dấu sao
            }
            else if (e.key.keysym.sym == SDLK_RETURN) {
                m_isLoginValid = CheckCredentials(m_username, m_password); // Kiểm tra tài khoản

                if (m_isLoginValid) {
                    printf("Login successful!\n");
                    GameStateMachine::GetInstance()->ChangeState(StateType::STATE_MENU);
                    // Thực hiện các hành động khi login thành công
                }
                else {
                    printf("Invalid username or password.\n");
                    // Xử lý khi tài khoản không hợp lệ
                }
            }
            else if (e.key.keysym.sym > SDLK_SPACE && e.key.keysym.sym <= SDLK_z) {
                // Thêm ký tự vào password
                m_password += static_cast<char>(e.key.keysym.sym);
                m_passwordDisplay += '*'; // Thêm dấu sao vào mật khẩu hiển thị
            }

            // Cập nhật lại text hiển thị mật khẩu dạng dấu sao
            m_passwordText->SetColor({ 0,0,0,255 });
            m_passwordText->SetText(m_passwordDisplay);
        }
    }
}



void GSLogin::HandleTouchEvents(SDL_Event& e)
{
    // Xử lý sự kiện cho hai nút
    m_usernameButton->HandleTouchEvent(&e);
    m_passwordButton->HandleTouchEvent(&e);
    m_loginButton->HandleTouchEvent(&e);
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

        // Nếu không còn focus vào username hoặc password
        if (isOutsideUsername && isOutsidePassword)
        {
            if (m_currentFocus == InputFieldFocus::USERNAME && m_username.empty()) {
                m_usernameText->SetColor({ 128,128,128,255 });
                m_usernameText->SetText("username");
            }
            if (m_currentFocus == InputFieldFocus::PASSWORD && m_password.empty()) {
                m_passwordText->SetColor({ 128,128,128,255 });
                m_passwordText->SetText("password");
            }
            m_currentFocus = InputFieldFocus::NONE;
            printf("Focus is now NONE\n");
        }
        else {
            // Nếu click vào ô username
            if (!isOutsideUsername && m_currentFocus != InputFieldFocus::USERNAME) {
                m_username = "";
                m_usernameText->SetText(m_username);
                m_currentFocus = InputFieldFocus::USERNAME;
                if (m_password.empty()) {
                    m_passwordText->SetColor({ 128,128,128,255 });
                    m_passwordText->SetText("password");
                }
            }
            // Nếu click vào ô password
            if (!isOutsidePassword && m_currentFocus != InputFieldFocus::PASSWORD) {
                m_password = "";
                m_passwordDisplay = "";     
                m_passwordText->SetText(m_password);
                m_currentFocus = InputFieldFocus::PASSWORD;
                if (m_username.empty()) {
                    m_usernameText->SetColor({ 128,128,128,255 });
                    m_usernameText->SetText("username");
                }
            }
        }
    }
}


void GSLogin::HandleMouseMoveEvents(int x, int y)
{
}

void GSLogin::Update(float deltaTime)
{
}

void GSLogin::Draw(SDL_Renderer* renderer)
{
    m_background->Draw(renderer);

    // Draw the title text
    m_login->Draw(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Play Button
    m_usernameButton->Draw(renderer);
    m_usernameText->Draw(renderer);
    SDL_RenderDrawRect(renderer, &m_usernameRect);

    // Exit Button
    m_passwordButton->Draw(renderer);
    m_passwordText->Draw(renderer);
    SDL_RenderDrawRect(renderer, &m_passwordRect);

    // Sign up Button
    m_signupButton->Draw(renderer);
    m_signupText->Draw(renderer);
    SDL_RenderDrawRect(renderer, &m_signupRect);

    // Log in Button
    m_loginButton->Draw(renderer);
    m_loginText->Draw(renderer);
    SDL_RenderDrawRect(renderer, &m_loginRect);
}

bool GSLogin::CheckCredentials(const std::string& username, const std::string& password)
{
    // Đây là phương thức giả, bạn sẽ kết nối tới database và kiểm tra tài khoản ở đây.
    // Sau khi kiểm tra, trả về true nếu đúng, false nếu sai.

    // Ví dụ kiểm tra đơn giản:
    if (username == "username" && password == "password") {
        return true;
    }
    return false;
}