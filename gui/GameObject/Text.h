#pragma once
#include "SDL.h"
#include <string>
#include <memory>
#include "Sprite2D.h"
class TextureManager;

class Text : public Sprite2D
{
private:
	std::string m_text;
	TTF_Font* m_font;
	SDL_Color m_color;
	bool m_isVisible;
public:
	Text(std::string text, TTF_Font* font,SDL_Color color);
	void		Init();
	void SetText(std::string text);
	void		Draw(SDL_Renderer* renderer, SDL_Rect *clip = NULL) ;
	void		Update(float deltatime) ;
	void GetTexture();
	void SetFont(TTF_Font* font);
	void SetColor(SDL_Color color);
	void SetVisible(bool visible);


};
