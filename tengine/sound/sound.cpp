#include "sound.h"

#include <stdio.h>
#include <SDL.h>
#include <SDL_mixer.h>

#include <strutils.h>

CSoundLibrary* CSoundLibrary::s_pSoundLibrary = NULL;
float CSoundLibrary::s_flMasterVolume = 1;
static CSoundLibrary g_SoundLibrary = CSoundLibrary();

CSoundLibrary::CSoundLibrary()
{
	s_pSoundLibrary = this;

	SDL_Init(SDL_INIT_AUDIO);

	if(Mix_OpenAudio(22050, AUDIO_S16, 2, 4096))
		SDL_Quit();

	Mix_ChannelFinished(&CSoundLibrary::ChannelFinished);
}

CSoundLibrary::~CSoundLibrary()
{
	for (size_t i = 0; i < m_apSounds.size(); i++)
	{
		delete m_apSounds[i];
	}

	Mix_CloseAudio();

	s_pSoundLibrary = NULL;
}

size_t CSoundLibrary::AddSound(const tstring& pszFilename)
{
	size_t iSound = FindSound(pszFilename);
	if (iSound != ~0)
		return iSound;

	m_apSounds.push_back(new CSound(pszFilename));

	iSound = m_apSounds.size()-1;
	return iSound;
}

CSound* CSoundLibrary::GetSound(size_t i)
{
	if (i >= m_apSounds.size())
		return NULL;

	return m_apSounds[i];
}

size_t CSoundLibrary::FindSound(const tstring& pszFilename)
{
	for (size_t i = 0; i < m_apSounds.size(); i++)
	{
		if (m_apSounds[i]->m_sFilename == pszFilename)
			return i;
	}

	return ~0;
}

void CSoundLibrary::PlaySound(CBaseEntity* pEntity, const tstring& pszFilename, float flVolume, bool bLoop)
{
	if (Get()->m_aiActiveSounds.find(pEntity) != Get()->m_aiActiveSounds.end())
	{
		if (Get()->m_aiActiveSounds[pEntity].find(pszFilename) != Get()->m_aiActiveSounds[pEntity].end())
			StopSound(pEntity, pszFilename);
	}

	size_t iSound = Get()->FindSound(pszFilename);

	if (iSound >= Get()->m_apSounds.size())
		return;

	CSound* pSound = Get()->m_apSounds[iSound];

	if( pSound->m_pSound == NULL )
		return;

	int iChannel = Mix_PlayChannel(-1, pSound->m_pSound, bLoop?-1:0);
	if (iChannel < 0)
		return;

	Get()->m_aiActiveSounds[pEntity][pszFilename].iChannel = iChannel;
	Get()->m_aiActiveSounds[pEntity][pszFilename].flVolume = flVolume;
	Mix_Volume(Get()->m_aiActiveSounds[pEntity][pszFilename].iChannel, (int)(flVolume*s_flMasterVolume*MIX_MAX_VOLUME));
}

void CSoundLibrary::StopSound(CBaseEntity* pEntity, const tstring& pszFilename)
{
	if (!pEntity)
	{
		// Un-register the channel finish callback for the time being because we clear the active sound list by ourselves.
		Mix_ChannelFinished(NULL);

		eastl::map<CBaseEntity*, eastl::map<tstring, CSoundInstance> >::iterator it = Get()->m_aiActiveSounds.begin();

		while (it != Get()->m_aiActiveSounds.end())
		{
			eastl::map<tstring, CSoundInstance>::iterator it2 = Get()->m_aiActiveSounds[(*it).first].begin();
			while (it2 != Get()->m_aiActiveSounds[(*it).first].end())
			{
				Mix_HaltChannel(Get()->m_aiActiveSounds[(*it).first][(*it2).first].iChannel);
				it2++;
			}

			it++;
		}

		Get()->m_aiActiveSounds.clear();

		Mix_ChannelFinished(&CSoundLibrary::ChannelFinished);
		return;
	}

	if (Get()->m_aiActiveSounds.find(pEntity) == Get()->m_aiActiveSounds.end())
		return;

	if (pszFilename != _T(""))
	{
		// Un-register the channel finish callback for the time being because we clear the active sound list by ourselves.
		Mix_ChannelFinished(NULL);

		eastl::map<tstring, CSoundInstance>::iterator it2 = Get()->m_aiActiveSounds[pEntity].begin();
		while (it2 != Get()->m_aiActiveSounds[pEntity].end())
		{
			Mix_HaltChannel(Get()->m_aiActiveSounds[pEntity][(*it2).first].iChannel);
			it2++;
		}

		Get()->m_aiActiveSounds[pEntity].clear();

		Mix_ChannelFinished(&CSoundLibrary::ChannelFinished);
		return;
	}

	if (Get()->m_aiActiveSounds[pEntity].find(pszFilename) == Get()->m_aiActiveSounds[pEntity].end())
		return;

	Mix_HaltChannel(Get()->m_aiActiveSounds[pEntity][pszFilename].iChannel);
}

bool CSoundLibrary::IsSoundPlaying(CBaseEntity* pEntity, const tstring& pszFilename)
{
	if (Get()->m_aiActiveSounds.find(pEntity) == Get()->m_aiActiveSounds.end())
		return false;

	if (Get()->m_aiActiveSounds[pEntity].find(pszFilename) == Get()->m_aiActiveSounds[pEntity].end())
		return false;

	return true;
}

Mix_Music *g_pMusic = NULL;
void CSoundLibrary::PlayMusic(const tstring& pszFilename, bool bLoop)
{
	if (g_pMusic)
		StopMusic();

	g_pMusic = Mix_LoadMUS(convertstring<tchar, char>(pszFilename).c_str());

	Mix_PlayMusic(g_pMusic, bLoop?-1:0);
}

void CSoundLibrary::StopMusic()
{
	if (!g_pMusic)
		return;

	Mix_HaltMusic();
	Mix_FreeMusic(g_pMusic);	// Because music wants to be free!

	g_pMusic = NULL;
}

bool CSoundLibrary::IsMusicPlaying()
{
	return !!Mix_PlayingMusic();
}

void CSoundLibrary::SetSoundVolume(CBaseEntity* pEntity, const tstring& pszFilename, float flVolume)
{
	if (Get()->m_aiActiveSounds.find(pEntity) == Get()->m_aiActiveSounds.end())
		return;

	if (Get()->m_aiActiveSounds[pEntity].find(pszFilename) == Get()->m_aiActiveSounds[pEntity].end())
		return;

	Get()->m_aiActiveSounds[pEntity][pszFilename].flVolume = flVolume;
	Mix_Volume(Get()->m_aiActiveSounds[pEntity][pszFilename].iChannel, (int)(flVolume*s_flMasterVolume*MIX_MAX_VOLUME));
}

void CSoundLibrary::SetSoundVolume(float flVolume)
{
	s_flMasterVolume = flVolume;
	for (eastl::map<CBaseEntity*, eastl::map<tstring, CSoundInstance> >::iterator it = Get()->m_aiActiveSounds.begin(); it != Get()->m_aiActiveSounds.end(); it++)
	{
		for (eastl::map<tstring, CSoundInstance>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
			Mix_Volume(it2->second.iChannel, (int)(it2->second.flVolume*s_flMasterVolume*MIX_MAX_VOLUME));
	}
}

void CSoundLibrary::SetMusicVolume(float flVolume)
{
	Mix_VolumeMusic((int)(flVolume*128));
}

void CSoundLibrary::ChannelFinished(int iChannel)
{
	eastl::map<CBaseEntity*, eastl::map<tstring, CSoundInstance> >::iterator it = Get()->m_aiActiveSounds.begin();

	while (it != Get()->m_aiActiveSounds.end())
	{
		CBaseEntity* pEntity = (*it).first;
		eastl::map<tstring, CSoundInstance>::iterator it2 = Get()->m_aiActiveSounds[pEntity].begin();
		while (it2 != Get()->m_aiActiveSounds[pEntity].end())
		{
			tstring pszFilename = (*it2).first;
			if (iChannel == Get()->m_aiActiveSounds[pEntity][(*it2).first].iChannel)
			{
				Get()->m_aiActiveSounds[pEntity].erase(Get()->m_aiActiveSounds[pEntity].find(pszFilename));
				if (Get()->m_aiActiveSounds[pEntity].size() == 0)
					Get()->m_aiActiveSounds.erase(Get()->m_aiActiveSounds.find(pEntity));
				return;
			}

			it2++;
		}

		it++;
	}
}

void CSoundLibrary::EntityDeleted(CBaseEntity* pEntity)
{
	// Remove from the active sound list. Let the sounds finish what they're playing.
	if (Get()->m_aiActiveSounds.find(pEntity) != Get()->m_aiActiveSounds.end())
		Get()->m_aiActiveSounds.erase(Get()->m_aiActiveSounds.find(pEntity));
}

CSound::CSound(const tstring& pszFilename)
{
	SDL_RWops* pRW = SDL_RWFromFile(convertstring<tchar, char>(pszFilename).c_str(), "rb");

	m_pSound = Mix_LoadWAV_RW(pRW, 1);

	m_sFilename = pszFilename;
}

CSound::~CSound()
{
	if( m_pSound != NULL )
	{
		Mix_FreeChunk( m_pSound );
	}
}
