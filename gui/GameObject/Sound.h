#pragma once
#include "SDL_mixer.h"
#include <string>
#include <map>
#include "Singleton.h"
class Sound :public CSingleton<Sound>
{
public:
	Sound();
	~Sound();
	bool Init();
	void PlaySound(const std::string& name);
	void PauseSound();
	void ResumeSound();
	void StopSound();
	void LoadSound(const std::string& name);
	void LoadSfx(const std::string& name);
	void PlaySfx(const std::string& name, int repeat = 0);
	void CleanUp();
	void ToggleMute();
private:
	std::string m_PathMusic;
	//Mix_Music* m_Music = NULL;
	std::map< std::string, Mix_Chunk*> m_Sfxs;
	std::map<std::string, Mix_Music*> m_Sounds;
	bool m_isMuted = false;
	std::string currentMusic;
};
