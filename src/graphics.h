#ifndef _GRAPHICS_H_
#define _GRAPHICS_ 1

#include <SDL.h>

enum {
		/*SCREEN=0,
		BUFFER,*/
		TITLE=0,
		SPRITES,
		//CREDITS,
		//DECK,
		//NUMSBIG,
		GAMEBG,
		//CASTLE,
		BOSS,
		DLGWINNER,
		DLGLOOSER,
		DLGNETWORK,
		DLGERROR,
		DLGMSG,
		ORIGINALSPRITES,
		GFX_CNT
};

enum {
	START=0,
	HOTSEAT,
	MULTIPLAYER,
	SCORE,
	CREDITS,
	QUIT
};

typedef struct PictureInfo{
    char* File;
    SDL_Surface* Surface;
    struct PictureInfo* Next;
} Picture; //GE: Pointers EVERYWHERE!

typedef struct S_Size
{
	int X; int Y;
} Size;
typedef struct S_Range
{
	float X; float Y;
} SizeF;
void PrecacheCard(const char* File, size_t Size);

void Graphics_Init();
void Graphics_Quit();
void Blit(int a,int b);
void UpdateScreen();
void RedrawScreen();
void UpdateScreenRect(int x1,int y1,int x2,int y2);
void FillRect(int x,int y,int w,int h,Uint8 r,Uint8 g,Uint8 b);
void DrawMenuItem(int Type, char Lit);
int Menu();
void DrawCard(int8 Player, int8 Number, float X, float Y)
void DrawCardAlpha(int8 Player, int8 Number, float X, float Y, float Alpha)
void DrawFoldedAlpha(int Team, float X, float Y, float Alpha);
void DrawFolded(int Team, float X, float Y);
//void DrawStatus(int turn,struct Stats *Player);
char *DialogBox(int type,const char *fmt,...);
int InRect(int x, int y, int x1, int y1, int x2, int y2);
float FInRect(float x, float y, float x1, float y1, float x2, float y2);
//void DrawRectangle(int x, int y, int w, int h, int Colour);
void LoadSurface(char* filename, int Slot);
void DoCredits();
float FMax(float A, float B);
float FMin(float A, float B);

#endif
