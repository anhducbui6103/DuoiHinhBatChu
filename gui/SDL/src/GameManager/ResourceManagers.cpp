#include "ResourceManagers.h"

ResourceManagers::ResourceManagers()
{
	std::string dataPath = "Data/";
	m_TexturePath = dataPath + "Textures/";
	m_FontPath = dataPath + "Fonts/";
}

ResourceManagers::~ResourceManagers()
{
	//// free texture

	
	for (auto it = m_MapTexture.begin(); it != m_MapTexture.end(); ++it)
	{
		
		if (it->second != nullptr) {
			SDL_DestroyTexture(it->second.get()->GetTextureObj());
			it->second.get()->SetTextureObj(NULL);
		}
	}
	m_MapTexture.clear();
	//// free font
	for (auto it = m_MapFonts.begin(); it != m_MapFonts.end(); ++it)
	{
		
		if (it->second != nullptr) {
			if (TTF_WasInit()) {
				TTF_CloseFont(it->second);
			}
			it->second = nullptr;
		}
	}
	m_MapFonts.clear();
}

TTF_Font* ResourceManagers::GetFont(const std::string& name, int ptsize)
{
	
	auto it = m_MapFonts.find(name);
	if (it != m_MapFonts.end())
	{
		return it->second;
	}
	std::string fontPath = m_FontPath + name;
	TTF_Font* font = TTF_OpenFont(fontPath.c_str(), ptsize);
	if (font != nullptr)
	{
		m_MapFonts.insert(std::pair<std::string, TTF_Font*>(name, (font)));
	}
	else
	{
		printf(" Couldn't open font : %s", name.c_str());
	}
	return font;
}

void ResourceManagers::RemoveTexture(const std::string& name)
{
	m_MapTexture.erase(name);
}

void ResourceManagers::RemoveFont(const std::string& name)
{
	m_MapFonts.erase(name);
}

std::shared_ptr<TextureManager> ResourceManagers::GetTexture(const std::string& name)
{
	auto it = m_MapTexture.find(name);
	if (it != m_MapTexture.end())
	{
		return it->second;
	}
	
	std::string file = m_TexturePath + name;

	std::shared_ptr<TextureManager> textureLoaded = std::make_shared<TextureManager>();
	if (textureLoaded->LoadImage(file.c_str()) != nullptr)
	{
		m_MapTexture.insert(std::pair<std::string, std::shared_ptr<TextureManager>>(name, (textureLoaded)));
	}
	else 
	{
		printf("!WARNING! Can not loaded texture: %s", SDL_GetError());
	}
	return textureLoaded;
}

std::shared_ptr<TextureManager> ResourceManagers::GetTextureText(TTF_Font* font, SDL_Color color, const std::string& textureText)
{
	auto it = m_MapTexture.find(textureText);
	if (it != m_MapTexture.end())
	{
		return it->second;
	}
	std::shared_ptr<TextureManager> textureLoaded = std::make_shared<TextureManager>();
	if (textureLoaded->LoadTextureText(font, color, textureText.c_str()) != nullptr)
	{
		m_MapTexture.insert(std::pair<std::string, std::shared_ptr<TextureManager>>(textureText, (textureLoaded)));
	}
	else
	{
		printf("!WARNING! Can not loaded texture text: %s", SDL_GetError());
	}
	return textureLoaded;
}



