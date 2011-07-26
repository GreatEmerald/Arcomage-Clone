#include <SDL_mixer.h>
#include "common.h"
#include "sound.h"

Mix_Chunk *SfxData[SFX_CNT];
int SoundEnabled=0;
int bUseMMSounds=1;

void LoadSound(char *filename,Mix_Chunk **snd)
{
	*snd=Mix_LoadWAV(filename);
	if (!*snd) FatalError("File '%s' is missing or corrupt.",filename);
}

void Sound_Init()
{
	// initialize audio
	if (Mix_OpenAudio(22050,AUDIO_S16LSB,2,4096)!=0)
		return;
	if (bUseMMSounds)
	{
		SoundEnabled=1;
		LoadSound(ARCODATADIR "damage.wav",&SfxData[DAMAGE]);
		LoadSound(ARCODATADIR "deal.wav",&SfxData[DEAL]);
		LoadSound(ARCODATADIR "defeat.wav",&SfxData[DEFEAT]);
		LoadSound(ARCODATADIR "resb_down.wav",&SfxData[RESB_DOWN]);
		LoadSound(ARCODATADIR "resb_up.wav",&SfxData[RESB_UP]);
		LoadSound(ARCODATADIR "ress_down.wav",&SfxData[RESS_DOWN]);
		LoadSound(ARCODATADIR "ress_up.wav",&SfxData[RESS_UP]);
		LoadSound(ARCODATADIR "shuffle.wav",&SfxData[SHUFFLE]);
		LoadSound(ARCODATADIR "title.wav",&SfxData[TITLE]);
		LoadSound(ARCODATADIR "tower_up.wav",&SfxData[TOWER_UP]);
		LoadSound(ARCODATADIR "victory.wav",&SfxData[VICTORY]);
		LoadSound(ARCODATADIR "wall_up.wav",&SfxData[WALL_UP]);
	}
	else
	{
		SoundEnabled=1;
		LoadSound(ARCODATADIR "damageO.wav",&SfxData[DAMAGE]);
		LoadSound(ARCODATADIR "dealO.wav",&SfxData[DEAL]);
		LoadSound(ARCODATADIR "defeatO.wav",&SfxData[DEFEAT]);
		LoadSound(ARCODATADIR "resb_downO.wav",&SfxData[RESB_DOWN]);
		LoadSound(ARCODATADIR "resb_upO.wav",&SfxData[RESB_UP]);
		LoadSound(ARCODATADIR "ress_downO.wav",&SfxData[RESS_DOWN]);
		LoadSound(ARCODATADIR "ress_upO.wav",&SfxData[RESS_UP]);
		LoadSound(ARCODATADIR "shuffleO.wav",&SfxData[SHUFFLE]);
		LoadSound(ARCODATADIR "titleO.wav",&SfxData[TITLE]);
		LoadSound(ARCODATADIR "tower_upO.wav",&SfxData[TOWER_UP]);
		LoadSound(ARCODATADIR "victoryO.wav",&SfxData[VICTORY]);
		LoadSound(ARCODATADIR "wall_upO.wav",&SfxData[WALL_UP]);
	}
}

void Sound_Quit()
{
	int i;
	if (SoundEnabled)
	{
		Mix_HaltChannel(-1);
		for (i=0;i<SFX_CNT;i++)
			Mix_FreeChunk(SfxData[i]);
		Mix_CloseAudio();
	}
}

int Sound_Play(int which)
{
	if (!SoundEnabled) return -1;
	if (which<0) return which;
	return Mix_PlayChannel(-1,SfxData[which],0);
}
