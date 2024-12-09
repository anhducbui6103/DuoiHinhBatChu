#pragma once
#include <string>

#include "Renderer.h"
#include <vector>
class TextureManager  
{
private:
	SDL_Texture* m_texture;
public:
	TextureManager();
	~TextureManager();

	int originWidth;
	int originHeight;
	SDL_Texture* LoadImage(const std::string& path);
	SDL_Texture* LoadTextureText( TTF_Font * font, SDL_Color color, const std::string& textureText);
	//Render texture at given point

	void setColor(Uint8 red, Uint8 green, Uint8 blue);

	//Set blending
	void SetBlendMode(SDL_BlendMode blending);
	//Set Alpha
	void SetAlpha(Uint8 alpha);
	void SetOriginalTextureSurface(SDL_Surface* surface);
	SDL_Texture* GetTextureObj();
	void SetTextureObj(SDL_Texture* texture);
	int GetOriginalWidth();
	int GetOriginalHeight();


};


