#pragma once
#include <map>
#include "SDL_ttf.h"
#include <string>
#include "Singleton.h"

#include "TextureManager.h"


class ResourceManagers : public CSingleton<ResourceManagers>
{
public:
	ResourceManagers();
	~ResourceManagers();


	void AddTexture(const std::string& name);
	void AddFont(const std::string& name);

	void RemoveSound(const std::string& name) {};
	void RemoveTexture(const std::string& name);
	void RemoveFont(const std::string& name);

	std::shared_ptr<TextureManager> GetTexture(const std::string& name);
	std::shared_ptr<TextureManager> GetTextureText(TTF_Font* font, SDL_Color color, const std::string& textureText);
	TTF_Font* GetFont(const std::string& name, int ptsize);

private:
	std::map<std::string, std::shared_ptr<TextureManager>> m_MapTexture;
	std::map<std::string, TTF_Font*> m_MapFonts;
	std::string m_ShaderPath;
	std::string m_TexturePath;
	std::string m_ModelsPath;
	std::string m_FontPath;

	//Sound
	std::string m_SoundPath;

};
