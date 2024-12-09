#include "Text.h"
#include "TextureManager.h"
#include "ResourceManagers.h"

Text::Text(std::string text, TTF_Font* font, SDL_Color color) 
    : m_text(text), m_font(font), m_color(color), m_isVisible(true)
{
    Init();
}

void Text::Init()
{
    m_position = Vector3(-1.0f, 1.0f, 1.0f);
    m_pTexture = std::make_shared<TextureManager>();
    GetTexture(); // Lấy texture của văn bản
    TTF_SizeText(m_font, m_text.c_str(), &m_iWidth, &m_iHeight); // Cập nhật kích thước dựa trên văn bản và font
    SetSize(m_iWidth, m_iHeight); // Cập nhật kích thước của đối tượng Text
}

void Text::Draw(SDL_Renderer* renderer, SDL_Rect* clip)
{
    if (m_isVisible && m_pTexture != nullptr)
    {
        SDL_Rect dstRect = { static_cast<int>(m_position.x), static_cast<int>(m_position.y), m_iWidth, m_iHeight };
        if (clip != NULL)
        {
            dstRect.w = clip->w;
            dstRect.h = clip->h;
        }
        SDL_RenderCopy(Renderer::GetInstance()->GetRenderer(), m_pTexture->GetTextureObj(), clip, &dstRect);
    }
}

void Text::Update(float deltaTime)
{
    // Không cần thay đổi gì trong hàm Update nếu không cần thực hiện thao tác nào.
}

void Text::GetTexture()
{
    if (m_pTexture != nullptr)
    {
        m_pTexture->LoadTextureText(m_font, m_color, m_text);
        TTF_SizeText(m_font, m_text.c_str(), &m_iWidth, &m_iHeight); // Cập nhật kích thước sau khi thay đổi nội dung text
        SetSize(m_iWidth, m_iHeight); // Cập nhật lại kích thước của đối tượng Text
    }
    else
    {
        printf("Error!!! Can not get texture!!!!");
    }
}

void Text::SetText(std::string text)
{
    m_text = text;
    GetTexture(); // Khi thay đổi văn bản, cập nhật lại kích thước và texture
}

void Text::SetFont(TTF_Font* font)
{
    m_font = font;
    GetTexture(); // Khi thay đổi font, cập nhật lại kích thước và texture
}

void Text::SetColor(SDL_Color color)
{
    m_color = color;
    GetTexture(); // Khi thay đổi màu sắc, cập nhật lại kích thước và texture
}

void Text::SetVisible(bool visible)
{
    m_isVisible = visible;
}