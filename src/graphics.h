#ifndef _GRAPHICS_H_
#define _GRAPHICS_ 1

#include <SDL.h>
#include <SDL_opengl.h>

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
typedef struct S_CardHandle
{
	int Pool; int Card;
} CardHandle;

typedef struct S_OpenGLTexture
{
    GLuint Texture;
    Size TextureSize;
} OpenGLTexture;

typedef struct S_CachedCard
{
    OpenGLTexture TitleTexture;
    int PictureHandle; //GE: Links to PictureFileCache[PictureHandle]
    SDL_Rect PictureCoords;
    OpenGLTexture* DescriptionTextures;
    int DescriptionNum;
    OpenGLTexture PriceTexture[3]; //GE: Bricks, gems, recruits
} CachedCard;
CachedCard** CardCache;
SizeF** CardLocations; //GE: Where on the screen all our cards are.

typedef struct S_TableCard
{
	CardHandle CH; char bDiscarded;
} TableCard;
TableCard* CardsOnTable; //GEm: Cards that are displayed on screen after they were played.
int CardsOnTableSize;
char CardInTransit; //GEm: If (-1) and which card is in transit.
char bDiscardedInTransit; //GEm: Whether the card in transit has been discarded.

OpenGLTexture* PictureFileCache;
int PictureFileCacheSize;

const int CPUWAIT; //DEBUG

void PrecacheCards();
void PrecachePictures(int NumPools, int* NumCards);
void InitCardLocations(int NumPlayers);

void Graphics_Init();
void Graphics_Quit();
void Blit(int a,int b);
void UpdateScreen();
void RedrawScreen();
void UpdateScreenRect(int x1,int y1,int x2,int y2);
//void FillRect(int x,int y,int w,int h,Uint8 r,Uint8 g,Uint8 b);
void DrawMenuItem(int Type, char Lit);
int Menu();
void DrawBackground();
void DrawUI();
void DrawScene();
void DrawLogo();
void DrawCard(char Player, char Number, float X, float Y);
void DrawCardAlpha(char Player, char Number, float X, float Y, float Alpha);
void DrawAllPlayerCards();
void DrawXPlayerCards(int PlayerNum, int CardNum);
void DrawFoldedAlpha(int Team, float X, float Y, float Alpha);
void DrawFolded(int Team, float X, float Y);
void DrawDiscard(float X, float Y);
void DrawCardsOnTable();
void DrawXCardsOnTable();
void DrawSmallNumber(int Number, SizeF Destination, SizeF BoundingBox);
void DrawMediumNumbers(int Player);
void DrawBigNumbers(int Player);
void DrawStatus();
void PlayCardAnimation(int CardPlace, char bDiscarded, char bSameTurn);
void PlayCardPostAnimation(int CardPlace);
SizeF GetCardOnTableLocation(int CardSlot);
char* DrawDialog(int type,const char *fmt,...);
int InRect(int x, int y, int x1, int y1, int x2, int y2);
float FInRect(float x, float y, float x1, float y1, float x2, float y2);
//void DrawRectangle(int x, int y, int w, int h, int Colour);
void LoadSurface(char* filename, int Slot);
void DoCredits();
float FMax(float A, float B);
float FMin(float A, float B);
SDL_Rect AbsoluteTextureSize(Size TextureSize);
float GetDrawScale();

#endif
