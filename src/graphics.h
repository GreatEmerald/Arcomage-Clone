#ifndef _GRAPHICS_H_
#define _GRAPHICS_ 1

//#include "cards.h"

enum {	/*SCREEN=0,
		BUFFER,*/
		MENU=0,
		MENUITEMS,
		CREDITS,
		DECK,
		NUMSBIG,
		GAMEBG,
		CASTLE,
		BOSS,
		DLGWINNER,
		DLGLOOSER,
		DLGNETWORK,
		DLGERROR,
		DLGMSG,
		GFX_CNT};
typedef struct PictureInfo{
    char* File;
    SDL_Surface* Surface;
    struct PictureInfo* Next;
} Picture; //GE: Pointers EVERYWHERE!
void PrecacheCard(const char* File, size_t Size);

void Graphics_Init(int fullscreen);
void Graphics_Quit();
void Blit(int a,int b);
void UpdateScreen();
void RedrawScreen();
void UpdateScreenRect(int x1,int y1,int x2,int y2);
void FillRect(int x,int y,int w,int h,Uint8 r,Uint8 g,Uint8 b);
int Menu();
void DrawCard(int c,int x,int y, Uint8 a);
void DrawCardAlpha(int c,int x,int y,Uint8 a);
void DrawFolded(int Team, int X, int Y);
//void DrawStatus(int turn,struct Stats *Player);
char *DialogBox(int type,const char *fmt,...);
int InRect(int x, int y, int x1, int y1, int x2, int y2);
void DrawRectangle(int x, int y, int w, int h, int Colour);
void LoadSurface(char *filename,SDL_Surface **surface);
void DoCredits();

#endif
