#ifndef _SOUND_H_
#define _SOUND_H_ 1

enum {	DAMAGE=0,
		DEAL,
		DEFEAT,
		RESS_DOWN,
		RESS_UP,
		RESB_DOWN,
		RESB_UP,
		SHUFFLE,
		TITLE,
		TOWER_UP,
		VICTORY,
		WALL_UP,
		SFX_CNT};

void Sound_Init();
void Sound_Quit();
int Sound_Play(int which);

#endif
