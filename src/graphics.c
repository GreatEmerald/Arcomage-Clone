#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "BFont.h"
//#include "resize.h"
//#include "cards.h"
//#include "common.h"
//#include "config.h"
#include "graphics.h"
#include "adapter.h"
#include "opengl.h"
#include "ttf.h"
//#include "input.h"
//#include "sound.h"

SDL_Event event;
GLuint GfxData[GFX_CNT];
Size TextureCoordinates[GFX_CNT];
Picture *PictureHead = NULL;//GE: Linked list.

BFont_Info *numssmall=NULL;
BFont_Info *font=NULL;
BFont_Info *bigfont=NULL;

const int CPUWAIT=10; //DEBUG

void Graphics_Init()
{
    char fullscreen = GetConfig(Fullscreen);
    
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE)<0) //GE: NOPARACHUTE means no exception handling. Could be dangerous. Could be faster.
	FatalError("Couldn't initialise SDL");
    SDL_WM_SetCaption("Arcomage Clone",NULL);
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ); //GE: Enable OpenGL double buffer support.
    if (!SDL_SetVideoMode(GetConfig(ResolutionX),GetConfig(ResolutionY),0,(fullscreen*SDL_FULLSCREEN)|SDL_OPENGL)) //GE: Enable OpenGL, because without it SDL is just plain bad.
	FatalError("Couldn't initialise OpenGL");
    InitOpenGL();
    
    #ifdef linux
	LoadSurface(GetFilePath("boss_linux.png"),BOSS);
    #else
	LoadSurface(GetFilePath("boss_windows.png"),BOSS);
    #endif
    LoadSurface(GetFilePath("Sprites.PNG"),SPRITES);
    LoadSurface(GetFilePath("Title.PNG"),TITLE);
    LoadSurface(GetFilePath("Layout.PNG"),GAMEBG);
    /*if (!GetConfig(UseOriginalMenu))
    {
	LoadSurface(GetFilePath("menu.png"),&GfxData[MENU]);
	LoadSurface(GetFilePath("menuitems.png"),&GfxData[MENUITEMS]);
	LoadSurface(GetFilePath("gamebg.png"),&GfxData[GAMEBG]);
    }
    LoadSurface(GetFilePath("credits.png"),&GfxData[CREDITS]);
    if (!GetConfig(UseOriginalCards))
	LoadSurface(GetFilePath("deck.png"),&GfxData[DECK]);
    else
      LoadSurface(GetFilePath("SPRITES.bmp"),&GfxData[DECK]);
    SDL_SetColorKey(GfxData[DECK],SDL_SRCCOLORKEY,SDL_MapRGB(GfxData[DECK]->format,255,0,255));
    LoadSurface(GetFilePath("nums_big.png"),&GfxData[NUMSBIG]);
    SDL_SetColorKey(GfxData[NUMSBIG],SDL_SRCCOLORKEY,SDL_MapRGB(GfxData[NUMSBIG]->format,255,0,255));
    LoadSurface(GetFilePath("castle.png"),&GfxData[CASTLE]);*/

    LoadSurface(GetFilePath("dlgmsg.png"),DLGMSG);
    LoadSurface(GetFilePath("dlgerror.png"),DLGERROR);
    LoadSurface(GetFilePath("dlgnetwork.png"),DLGNETWORK);
    LoadSurface(GetFilePath("dlgwinner.png"),DLGWINNER);
    LoadSurface(GetFilePath("dlglooser.png"),DLGLOOSER);

    /*SDL_SetColorKey(GfxData[CASTLE],SDL_SRCCOLORKEY,SDL_MapRGB(GfxData[CASTLE]->format,255,0,255));

    numssmall=BFont_LoadFont(GetFilePath("nums_small.png"));
    if (!numssmall)
	FatalError("Data file 'nums_small.png' is missing or corrupt.");
    bigfont=BFont_LoadFont(GetFilePath("bigfont.png"));
    if (!bigfont)
	FatalError("Data file 'bigfont.png' is missing or corrupt.");
    font=BFont_LoadFont(GetFilePath("font.png"));
    if (!font)
	FatalError("Data file 'font.png' is missing or corrupt.");
    BFont_SetCurrentFont(font);*/
}

void PrecacheCards()
{
    int i, n;
    
    int NumPools;
    int* NumCards;
    GetCardDBSize(&NumPools, &NumCards); //GE: Must free NumCards in this function!
    
    CardCache = (CachedCard**) malloc(NumPools * sizeof(CachedCard*)); //GE: Must free CardCache when quitting!
    
    for (i=0; i<NumPools; i++)
    {
        CardCache[i] = (CachedCard*) malloc(NumCards[i] * sizeof(CachedCard));
        for (n=0; n<NumCards[i]; n++)
        {
            CardCache[i][n].DescriptionNum = 0; //GE: Init the number to 0, since we can't do that when defining the struct.
            CardCache[i][n].DescriptionTextures = NULL;
        }
    }
    
    free(NumCards);
    
    PrecacheFonts();
    //GE: TODO: Precache picture.
}

//GE: Add to the linked list.
void PrecacheCard(const char* File, size_t Size)
{
    Picture* CurrentPicture;
    Picture* CheckedPicture = PictureHead;
    int bNew=0;
    
    //printf(File);
    if (!PictureHead)//GE: No cards precached, so don't overdo this.
        bNew = 1;
    else //GE: Let's find out if we already have this somewhere.
    {
        while (CheckedPicture)
        {
            //printf("PrecacheCard: Is %s = %s?\n", File, CheckedPicture->File);
            if (!strcmp(File, CheckedPicture->File)) //GE: I thought strcpy() was counterintuitive. This takes the cake!
                return; //GE: It's already been precached. Nothing to do.
            //printf("No it's not.");
            CheckedPicture = CheckedPicture->Next;
        }
    }
    
    CurrentPicture = malloc(sizeof(Picture));
    if (CurrentPicture == NULL) //GE: Allocate the memory to store this picture.
        FatalError("Out of memory to allocate the image linked list! Please use fewer cards."); //GE: Oh noes, out of memory to allocate! ...actually they are but pointers, so I doubt you'd ever run out of it.
    CurrentPicture->File = malloc(Size);
    if (CurrentPicture->File == NULL) //GE: Allocate the memory to store this picture and string.
        FatalError("Out of memory to allocate the image filename! Please use fewer cards."); //GE: Oh noes, out of memory to allocate! This one's quite a bit bigger.
    strcpy(CurrentPicture->File, File);//GE: Set file and surface.
    LoadSurface(CurrentPicture->File, &CurrentPicture->Surface);
    if (bNew)
    {
        CurrentPicture->Next = NULL; //GE: This is added to the end of the list, so next is NULL.
        PictureHead = CurrentPicture;
    }
    else
    {
        CurrentPicture->Next = PictureHead->Next;
        PictureHead->Next = CurrentPicture;
    }
}

void Graphics_Quit()
{
	int i;
	/*BFont_FreeFont(numssmall);
	BFont_FreeFont(font);
	BFont_FreeFont(bigfont);*/
	for (i=0;i<GFX_CNT;i++)
		FreeTextures(GfxData[i]);
}

/**
 * Draws the cards on the screen.
 *
 * This function is only used for drawing the cards at the bottom of the screen.
 * Works for all types of players.
 *
 * Authors: GreatEmerald, STiCK.
 *
 * \param turn Player number.
 */
/*void DrawCards(int turn) //DEBUG
{
    int i,j;

    if (turn==aiplayer || turn==netplayer)
    {
        j=aiplayer;if (j==-1) j=netplayer;
        for (i=0;i<6;i++)
        //GE: This is info on where to put in on the screen.
            DrawFolded(j,8+106*i,342);
    }
    else
        for (i=0;i<6;i++)
            if (Requisite(&Player[turn],i))
                DrawCard(Player[turn].Hand[i],8+106*i,342,255);
            else
                DrawCard(Player[turn].Hand[i],8+106*i,342,CardTranslucency);
}*/

void Blit(int a,int b)
{
	printf("Warning: Blit is deprecated!");
	//SDL_BlitSurface(GfxData[a],NULL,GfxData[b],NULL);
}

//GE: This function updates the screen. Nothing is being remade. Fast.
void UpdateScreen()
{
	SDL_GL_SwapBuffers();
	//SDL_UpdateRect(GfxData[SCREEN],0,0,0,0);
}

void ClearScreen()
{
    glClear( GL_COLOR_BUFFER_BIT );
}

//GE: This function redraws the screen elements. Slow.
/*void RedrawScreen(int turn, struct Stats* Player) //DEBUG
{
    SDL_Flip(GfxData[SCREEN]);
    //Blit(BUFFER,SCREEN);
    //Blit(GAMEBG,SCREEN);
    //DrawStatus(turn,Player);
	//DrawCards(turn);
	//UpdateScreen();
}*/

/**
 * Redraws all the information on the screen the hard way.
 * 
 * You want to use RedrawScreen() instead most of the time, because this
 * function draws new things on screen. It's a very dumb way to update stuff.
 * 
 * Authors: STiCK, GreatEmerald
 */ 
/*void RedrawScreenFull() //DEBUG
{
    DrawStatus(turn,Player);

    DrawCards(turn);
    UpdateScreen();
}*/

inline void UpdateScreenRect(int x1,int y1,int x2,int y2)
{
	printf("Warning: UpdateScreenRect is deprecated!");
	//SDL_UpdateRect(GfxData[SCREEN],x1,y1,x2,y2);
}

void FillRect(int x,int y,int w,int h,Uint8 r,Uint8 g,Uint8 b)
{
	printf("Warning: FillRect is deprecated!");
	/*SDL_Rect rect;
	rect.x=x;rect.y=y;rect.w=w;rect.h=h;
	SDL_FillRect(GfxData[SCREEN],&rect,SDL_MapRGB(GfxData[SCREEN]->format,r,g,b));*/
}

void NewDrawCard(int C, int X, int Y, SDL_Surface* Sourface, Uint8 Alpha)//GE: SOURFACE! :(
{
    //SDL_Rect ScreenPosition, DeckPosition;
    //printf("Incoming crash!\n");
    //printf("We got the coordinates: %d:%d; %d:%d\n", D_getPictureCoords(0,C).x, D_getPictureCoords(0,C).y, D_getPictureCoords(0,C).w, D_getPictureCoords(0,C).h);
    //DeckPosition = D_getPictureCoords(0,C);
    printf("Warning: NewDrawCard: Function is obsolete!\n");
    //GE: HACK!
    /*DeckPosition.x = (int16_t) D_getPictureCoordX(0,C);
    DeckPosition.y = (int16_t) D_getPictureCoordY(0,C);
    DeckPosition.w = (uint16_t) D_getPictureCoordW(0,C);
    DeckPosition.h = (uint16_t) D_getPictureCoordH(0,C);
    //printf("Entered DrawNewCard.\n");
    ScreenPosition.x = X;
    ScreenPosition.y = Y;
    ScreenPosition.w = DeckPosition.w;
    ScreenPosition.h = DeckPosition.h;
    
    SDL_SetAlpha(Sourface,SDL_SRCALPHA,Alpha);
    //SDL_BlitSurface(Sourface,&DeckPosition,GfxData[SCREEN],&ScreenPosition);
    SDL_SetAlpha(Sourface,SDL_SRCALPHA,255);
    printf("Finished drawing the card.\n");*/
}

/**
 * Draws a card on the screen.
 * 
 * In order to do that, the function needs to know which card to draw
 * and where. A card itself consists of the background, picture and
 * text for the name, description and cost(s). This function uses the
 * adapter to automatically get all the information it needs from D's
 * database - it only needs the number of the card in hand and the
 * player in question.
 */
void DrawCardAlpha(char Player, char Number, float X, float Y, float Alpha)
{
    int i;
    
    int Pool, Card;
    GetCardHandle(Player, Number, &Pool, &Card);
    SizeF BoundingBox, TextureSize;
    float Spacing;
    int BlockHeight=0;
    
    //GE: Draw the background.
    //GE: First, get the background that we will be using.
    int Colour = GetColourType(Player, Number);
    
    SDL_Rect ItemPosition;
    SizeF ScreenPosition = {X, Y};
    float DrawScale = GetDrawScale();
    
    //GEm: Draw background.
    ItemPosition.x = Colour * 192;  ItemPosition.w = 192; //GE: Careful here. Colours must be in the same order as they are in the picture and the adapter must match.
    ItemPosition.y = 324;           ItemPosition.h = 256;
    
    DrawTextureAlpha(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale, Alpha);
    
    //GEm: Draw title text.
    ItemPosition = AbsoluteTextureSize(CardCache[Pool][Card].TitleTexture.TextureSize);
    
    ScreenPosition.X += 4/(float)GetConfig(ResolutionX); ScreenPosition.Y += 4/(float)GetConfig(ResolutionY);
    BoundingBox.X = 88/(float)GetConfig(ResolutionX); BoundingBox.Y = 12/(float)GetConfig(ResolutionY);
    TextureSize.X = CardCache[Pool][Card].TitleTexture.TextureSize.X/(float)GetConfig(ResolutionX); TextureSize.Y = CardCache[Pool][Card].TitleTexture.TextureSize.Y/(float)GetConfig(ResolutionY);
    ScreenPosition = CentreOnX(ScreenPosition, TextureSize, BoundingBox);
    
    DrawTextureAlpha(CardCache[Pool][Card].TitleTexture.Texture, CardCache[Pool][Card].TitleTexture.TextureSize, ItemPosition, ScreenPosition, 1.0, Alpha);
    
    //GEm: Draw description text.
    ScreenPosition.X = X + 4/(float)GetConfig(ResolutionX); ScreenPosition.Y = Y + 72/(float)GetConfig(ResolutionY);
    for (i=0; i<CardCache[Pool][Card].DescriptionNum; i++)
        BlockHeight += CardCache[Pool][Card].DescriptionTextures[i].TextureSize.Y; //GEm: Alternatively, I could just multiply one by DescriptionNum, but four iterations are not much.
    if (CardCache[Pool][Card].DescriptionTextures[CardCache[Pool][Card].DescriptionNum].TextureSize.X > 66*DrawScale*2 && CardCache[Pool][Card].DescriptionNum > 1 && BlockHeight <= 41*DrawScale*2) //GEm: If we'd overlap with price and have enough space
        Spacing = ((41*DrawScale*2-BlockHeight)/(CardCache[Pool][Card].DescriptionNum+1))/(float)GetConfig(ResolutionY);
    else
        Spacing = ((53*DrawScale*2-BlockHeight)/(CardCache[Pool][Card].DescriptionNum+1))/(float)GetConfig(ResolutionY);
    ScreenPosition.Y += Spacing;
    for (i=0; i<CardCache[Pool][Card].DescriptionNum; i++)
    {
        ItemPosition = AbsoluteTextureSize(CardCache[Pool][Card].DescriptionTextures[i].TextureSize);
        TextureSize.X = CardCache[Pool][Card].DescriptionTextures[i].TextureSize.X/(float)GetConfig(ResolutionX); TextureSize.Y = CardCache[Pool][Card].DescriptionTextures[i].TextureSize.Y/(float)GetConfig(ResolutionY);
        ScreenPosition = CentreOnX(ScreenPosition, TextureSize, BoundingBox);
        DrawTextureAlpha(CardCache[Pool][Card].DescriptionTextures[i].Texture, CardCache[Pool][Card].DescriptionTextures[i].TextureSize, ItemPosition, ScreenPosition, 1.0, Alpha);
        ScreenPosition.Y += Spacing + CardCache[Pool][Card].DescriptionTextures[i].TextureSize.Y/(float)GetConfig(ResolutionY);
        ScreenPosition.X = X + 4/(float)GetConfig(ResolutionX); //GEm: Reset X, keep Y.
    }
    
    //GEm: Draw card cost.
    BoundingBox.X = 19/(float)GetConfig(ResolutionX); BoundingBox.Y = 12/(float)GetConfig(ResolutionY);
    ScreenPosition.X = X + 77/(float)GetConfig(ResolutionX); ScreenPosition.Y = Y + 111/(float)GetConfig(ResolutionY);
    switch (Colour)
    {
        case CT_Blue:
            ItemPosition = AbsoluteTextureSize(CardCache[Pool][Card].PriceTexture[1].TextureSize);
            TextureSize.X = CardCache[Pool][Card].PriceTexture[1].TextureSize.X/(float)GetConfig(ResolutionX); TextureSize.Y = CardCache[Pool][Card].PriceTexture[1].TextureSize.Y/(float)GetConfig(ResolutionY);
            ScreenPosition = CentreOnX(ScreenPosition, TextureSize, BoundingBox);
            DrawTextureAlpha(CardCache[Pool][Card].PriceTexture[1].Texture, CardCache[Pool][Card].PriceTexture[1].TextureSize, ItemPosition, ScreenPosition, 1.0, Alpha);
            break;
        case CT_Green:
            ItemPosition = AbsoluteTextureSize(CardCache[Pool][Card].PriceTexture[2].TextureSize);
            TextureSize.X = CardCache[Pool][Card].PriceTexture[2].TextureSize.X/(float)GetConfig(ResolutionX); TextureSize.Y = CardCache[Pool][Card].PriceTexture[2].TextureSize.Y/(float)GetConfig(ResolutionY);
            ScreenPosition = CentreOnX(ScreenPosition, TextureSize, BoundingBox);
            DrawTextureAlpha(CardCache[Pool][Card].PriceTexture[2].Texture, CardCache[Pool][Card].PriceTexture[2].TextureSize, ItemPosition, ScreenPosition, 1.0, Alpha);
            break;
        case CT_White:
            FatalError("FIXME: White cards not yet supported!");
            break;
        default: //GEm: Black and red cards, and anything else strange goes here.
            ItemPosition = AbsoluteTextureSize(CardCache[Pool][Card].PriceTexture[0].TextureSize);
            TextureSize.X = CardCache[Pool][Card].PriceTexture[0].TextureSize.X/(float)GetConfig(ResolutionX); TextureSize.Y = CardCache[Pool][Card].PriceTexture[0].TextureSize.Y/(float)GetConfig(ResolutionY);
            ScreenPosition = CentreOnX(ScreenPosition, TextureSize, BoundingBox);
            DrawTextureAlpha(CardCache[Pool][Card].PriceTexture[0].Texture, CardCache[Pool][Card].PriceTexture[0].TextureSize, ItemPosition, ScreenPosition, 1.0, Alpha);
            break;
    }
    
	/*SDL_Rect recta,rectb;
	int RawX, RawY;
	
	char* File;
	Picture* CurrentPicture = PictureHead;
	
	if (D_getPictureFileSize(0,c) == 1)
	   return;
	
  File = malloc(D_getPictureFileSize(0,c));
  //getchar();
  printf("Drawing picture with size: %d\n", D_getPictureFileSize(0,c));
  //getchar();
  strcpy(File, D_getPictureFile(0,c));
  printf("Allocation complete.\n");
  //getchar();
	
	while (CurrentPicture)
	{
     if (!strcmp(CurrentPicture->File, File))
	   {
	     printf("Attempting to draw card.\n");  
	     NewDrawCard(c,x,y,CurrentPicture->Surface, a);
         free(File);
         printf("Freeing complete.\n");
         return;
     }
     CurrentPicture = CurrentPicture->Next;
  }
	getchar();
	if (!bUseOriginalCards)
	{
	   recta.x=x;recta.y=y;recta.w=96;recta.h=128;
     rectb.x=(c&0xFF)*96;rectb.y=(c>>8)*128;rectb.w=96;rectb.h=128;
	}*/
}

inline void DrawCard(char Player, char Number, float X, float Y)
{
	DrawCardAlpha(Player, Number, X, Y, 1.0);
}

void DrawFoldedAlpha(int Team, float X, float Y, float Alpha)
{
    SDL_Rect DeckPosition;
    SizeF ScreenPosition;
    float DrawScale = GetDrawScale();
    
    ScreenPosition.X = X; ScreenPosition.Y = Y;
    
    DeckPosition.x = 960+192*Team; DeckPosition.y = 324;
    DeckPosition.w = 192; DeckPosition.h = 256;
    
    DrawTextureAlpha(GfxData[SPRITES], TextureCoordinates[SPRITES], DeckPosition, ScreenPosition, DrawScale, Alpha);
}

inline void DrawFolded(int Team, float X, float Y)
{
    DrawFoldedAlpha(Team, X, Y, 1.0);
}

void DrawDiscard(int X, int Y)
{
    SDL_Rect ScreenPosition, DeckPosition;
    
    ScreenPosition.x = X; ScreenPosition.y = Y;
    ScreenPosition.w = 96; ScreenPosition.h = 128;
    
    DeckPosition.x = 0; DeckPosition.y = 256;
    DeckPosition.w = ScreenPosition.w; DeckPosition.h = ScreenPosition.h;
    
    //SDL_BlitSurface(GfxData[DECK],&DeckPosition,GfxData[SCREEN],&ScreenPosition);
}

/**
 * Draws the 'boss' screen.
 *
 * Easter egg, can be used to protect players from angry bosses. Activated by
 * pressing the B button.
 *
 * Authors: STiCK.
 */
/*void Boss() //DEBUG
{
    Blit(SCREEN,BUFFER);
    Blit(BOSS,SCREEN);
    UpdateScreen();
    switch (OPERATINGSYSTEM)
    {
        case 1:
            SDL_WM_SetCaption("mc - ~/.xmms",NULL);break;                   // Linux
        default:
            SDL_WM_SetCaption("C:\\WINNT\\system32\\cmd.exe",NULL);break;   // Windows
    }
    WaitForKey(0);
    WaitForKey(SDLK_b);
    Blit(BUFFER,SCREEN);
    UpdateScreen();
    SDL_WM_SetCaption("Arcomage v"ARCOVER,NULL);
    WaitForKey(0);
}*/

/**
 * Draws the card being played to imitate animation.
 * 
 * Bugs: There should be no need to do that manually; coordinates need to be
 * relative.
 * 
 * Authors: STiCK, GreatEmerald.
 */
/*void PlayCardAnimation(int c, int discrd) //DEBUG
{
    #define STEPS 10
    double d,x,y;
    
    FillRect(8+106*c,342,96,128,0,0,0);
    Blit(SCREEN,BUFFER);
    for (d=0.0;d<=1.0;d+=1.0/STEPS)
    {
        x=(8.0+106.0*c)+d*(272.0-(8.0+106.0*c));
        y=342.0+d*(96.0-342.0);
        Blit(BUFFER,SCREEN);
        DrawCard(Player[turn].Hand[c],(int)x,(int)y,CardTranslucency);
        if (discrd)
            DrawDiscard((int)x,(int)y);
        UpdateScreen();
        SDL_Delay(20);
    }
    Blit(GAMEBG,SCREEN);
    if (discrd)
    {
        DrawCard(Player[turn].Hand[c],272,96,CardTranslucency);
        DrawDiscard(272,96);
    }
    else
        DrawCard(Player[turn].Hand[c],272,96,255);
}*/

void DrawSmallNumber(int Number, SizeF Destination, SizeF BoundingBox)
{
    //GEm: Draw one, two or three numbers, aligned to the left.
    //GEm: TODO: implement more than 2 players
    SDL_Rect AbsoluteSize;
    SizeF ObjectSize;
    int i;
    float NumberLength = 0.0;
    
    int HundredsDigit, TensDigit, OnesDigit;
    
    HundredsDigit = Number/100;
    TensDigit = Number/10%10;
    OnesDigit = Number%10;
    
    if (HundredsDigit > 0)
        NumberLength += NumberCache[Numbers_Small][HundredsDigit].TextureSize.X/(float)GetConfig(ResolutionX);
    if (TensDigit > 0 || HundredsDigit > 0)
        NumberLength += NumberCache[Numbers_Small][TensDigit].TextureSize.X/(float)GetConfig(ResolutionX);
    NumberLength += NumberCache[Numbers_Small][OnesDigit].TextureSize.X/(float)GetConfig(ResolutionX);
    ObjectSize.X = NumberLength; ObjectSize.Y = BoundingBox.Y;
    Destination = CentreOnX(Destination, ObjectSize, BoundingBox);
    
    if (HundredsDigit > 0)
    {
        AbsoluteSize = AbsoluteTextureSize(NumberCache[Numbers_Small][HundredsDigit].TextureSize);
        DrawTexture(NumberCache[Numbers_Small][HundredsDigit].Texture, NumberCache[Numbers_Small][HundredsDigit].TextureSize, AbsoluteSize, Destination, 1.0);
        
        Destination.X += NumberCache[Numbers_Small][HundredsDigit].TextureSize.X/(float)GetConfig(ResolutionX);
    }
    
    if (TensDigit > 0 || HundredsDigit > 0)
    {
        AbsoluteSize = AbsoluteTextureSize(NumberCache[Numbers_Small][TensDigit].TextureSize);
        DrawTexture(NumberCache[Numbers_Small][TensDigit].Texture, NumberCache[Numbers_Small][TensDigit].TextureSize, AbsoluteSize, Destination, 1.0);
        
        Destination.X += NumberCache[Numbers_Small][TensDigit].TextureSize.X/(float)GetConfig(ResolutionX);
    }
    
    AbsoluteSize = AbsoluteTextureSize(NumberCache[Numbers_Small][OnesDigit].TextureSize);
    DrawTexture(NumberCache[Numbers_Small][OnesDigit].Texture, NumberCache[Numbers_Small][OnesDigit].TextureSize, AbsoluteSize, Destination, 1.0);
}

void DrawMediumNumbers(int Player)
{
    //GEm: Draw one, two or three numbers, aligned to the left.
    //GEm: TODO: implement more than 2 players
    SDL_Rect AbsoluteSize;
    SizeF ScreenPosition;
    int i, Resource;
    
    int HundredsDigit, TensDigit, OnesDigit;
    
    for (i=0; i<3; i++)
    {
        switch(i)
        {
            case 0:
                Resource = GetCurrentBricks(Player);
                break;
            case 1:
                Resource = GetCurrentGems(Player);
                break;
            default:
                Resource = GetCurrentRecruits(Player);
        }
        HundredsDigit = Resource/100;
        TensDigit = Resource/10%10;
        OnesDigit = Resource%10;
        
        ScreenPosition.X = (11+706*Player)/800.0; ScreenPosition.Y = (263-13+72*i)/600.0;
        
        if (HundredsDigit > 0)
        {
            AbsoluteSize = AbsoluteTextureSize(NumberCache[Numbers_Medium][HundredsDigit].TextureSize);
            DrawTexture(NumberCache[Numbers_Medium][HundredsDigit].Texture, NumberCache[Numbers_Medium][HundredsDigit].TextureSize, AbsoluteSize, ScreenPosition, 1.0);
            
            ScreenPosition.X += NumberCache[Numbers_Medium][HundredsDigit].TextureSize.X/(float)GetConfig(ResolutionX);
        }
        
        if (TensDigit > 0 || HundredsDigit > 0)
        {
            AbsoluteSize = AbsoluteTextureSize(NumberCache[Numbers_Medium][TensDigit].TextureSize);
            DrawTexture(NumberCache[Numbers_Medium][TensDigit].Texture, NumberCache[Numbers_Medium][TensDigit].TextureSize, AbsoluteSize, ScreenPosition, 1.0);
            
            ScreenPosition.X += NumberCache[Numbers_Medium][TensDigit].TextureSize.X/(float)GetConfig(ResolutionX);
        }
        
        AbsoluteSize = AbsoluteTextureSize(NumberCache[Numbers_Medium][OnesDigit].TextureSize);
        DrawTexture(NumberCache[Numbers_Medium][OnesDigit].Texture, NumberCache[Numbers_Medium][OnesDigit].TextureSize, AbsoluteSize, ScreenPosition, 1.0);
    }
}

void DrawBigNumbers(int Player)
{
    //GEm: Draw one or two numbers, aligned to the left.
    //GEm: TODO: implement more than 2 players
    SDL_Rect AbsoluteSize;
    SizeF ScreenPosition;
    int i, Resource;
    
    int TensDigit;
    int OnesDigit;
    
    for (i=0; i<3; i++)
    {
        switch(i)
        {
            case 0:
                Resource = GetCurrentQuarry(Player);
                break;
            case 1:
                Resource = GetCurrentMagic(Player);
                break;
            default:
                Resource = GetCurrentDungeon(Player);
        }
        TensDigit = Resource/10;
        OnesDigit = Resource%10;
        
        ScreenPosition.X = (15+706*Player)/800.0; ScreenPosition.Y = (241-15+72*i)/600.0;
        
        if (TensDigit > 0)
        {
            AbsoluteSize = AbsoluteTextureSize(NumberCache[Numbers_Big][TensDigit].TextureSize);
            DrawTexture(NumberCache[Numbers_Big][TensDigit].Texture, NumberCache[Numbers_Big][TensDigit].TextureSize, AbsoluteSize, ScreenPosition, 1.0);
            
            ScreenPosition.X += NumberCache[Numbers_Big][TensDigit].TextureSize.X/(float)GetConfig(ResolutionX);
        }
        
        AbsoluteSize = AbsoluteTextureSize(NumberCache[Numbers_Big][OnesDigit].TextureSize);
        DrawTexture(NumberCache[Numbers_Big][OnesDigit].Texture, NumberCache[Numbers_Big][OnesDigit].TextureSize, AbsoluteSize, ScreenPosition, 1.0);
    }
}

void DrawStatus()
{
    int i;
    char* Name;
    SDL_Rect AbsoluteSize;
    SizeF ScreenPosition, RelativeSize, BoundingBox;
    
    //GEm: TODO: implement more than 2 players
    for (i=0; i<2; i++)
    {
        //GEm: Draw the name of the players, centred
        AbsoluteSize = AbsoluteTextureSize(NameCache[i].TextureSize);
        
        ScreenPosition.X = (11+706*i)/800.0; ScreenPosition.Y = 168/600.0;
        RelativeSize.X = NameCache[i].TextureSize.X/(float)GetConfig(ResolutionX);
        RelativeSize.Y = NameCache[i].TextureSize.Y/(float)GetConfig(ResolutionY);
        BoundingBox.X = 72/800.0; BoundingBox.Y = 7/600.0;
        ScreenPosition = CentreOnX(ScreenPosition, RelativeSize, BoundingBox);
        
        DrawTexture(NameCache[i].Texture, NameCache[i].TextureSize, AbsoluteSize, ScreenPosition, 1.0);
        
        //GEm: Draw the facility numbers.
        DrawBigNumbers(i);
        //GEm: Draw the resource numbers.
        DrawMediumNumbers(i);
        
        //GEm: Draw the tower height.
        ScreenPosition.X = (103+551*i)/800.0; ScreenPosition.Y = (443-3)/600.0;
        BoundingBox.X = 43/800.0; BoundingBox.Y = 7/600.0;
        DrawSmallNumber(GetCurrentTower(i), ScreenPosition, BoundingBox);
        
        //GEm: Draw the wall height.
        ScreenPosition.X = (166+433*i)/800.0; ScreenPosition.Y = (443-3)/600.0;
        BoundingBox.X = 36/800.0; BoundingBox.Y = 7/600.0;
        DrawSmallNumber(GetCurrentWall(i), ScreenPosition, BoundingBox);
    }
}

/**
 * Draws the static menu elements and the buttons (all unselected).
 */ 
void DrawMenuBackground()
{
    int i;
    
    DrawBackground();
    DrawLogo();
    for (i=0;i<6;i++)
		DrawMenuItem(i,0);
}

//GE: Draw menu buttons.
void DrawMenuItem(int Type, char Lit)
{
    SDL_Rect SourceCoords = {0,0,0,0}; //GE: Make sure they are initialised.
    SizeF DestinationCoords = {0.0,0.0};
    float ResX = (float)GetConfig(ResolutionX);
    float ResY = (float)GetConfig(ResolutionY);
    float DrawScale = GetDrawScale();
    
    if (Type < 3)
    {
        SourceCoords.x=0+250*Lit; SourceCoords.y=108*Type; SourceCoords.w=250; SourceCoords.h=108;
        DestinationCoords.X = ((2.0*Type+1.0)/6.0)-(((float)(SourceCoords.w*DrawScale)/ResX)/2.0); DestinationCoords.Y = ((130.0/600.0)-((float)(SourceCoords.h*DrawScale)/600.0))/2.0;
        //printf("Debug: DrawMenuItem: DestinationCoords.Y is %f\n", DestinationCoords.Y);
    }
    else
    {
        SourceCoords.x=250*2+250*Lit; SourceCoords.y=108*(Type-3); SourceCoords.w=250; SourceCoords.h=108;
        DestinationCoords.X = ((2.0*(Type-3)+1.0)/6.0)-(((float)(SourceCoords.w*DrawScale)/ResX)/2.0); DestinationCoords.Y = ((600.0-130.0/2.0)-(float)(SourceCoords.h*DrawScale)/2.0)/600.0;
        //printf("Debug: DrawMenuItem: LOWER BUTTONS: DestinationCoords.Y is %f\n", DestinationCoords.Y);
    }
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], SourceCoords, DestinationCoords, DrawScale);
}

int Menu()
{
	int i,/*j,*/value=-1;
	float ResX = (float)GetConfig(ResolutionX);
	float ResY = (float)GetConfig(ResolutionY);
	float DrawScale = GetDrawScale();
	int LitButton = -1; //GE: Which button is lit.

    DrawMenuBackground();
	UpdateScreen();
	
	//Sound_Play(TITLE);

	while (value == -1)
	{
        if (!SDL_PollEvent(&event)) //GE: Read the event loop. If it's empty, sleep instead of repeating the old events.
        {
            SDL_Delay(0);
            continue;
        }
        switch (event.type)
	    {
        case SDL_QUIT:
		    value=QUIT;
		    break;
		case SDL_MOUSEMOTION:
            for (i=0; i<6; i++)
		    {
                if ( (i < 3
                && FInRect(event.motion.x/ResX, event.motion.y/ResY,
                (2.0*i+1.0)/6.0-(250.0*DrawScale/ResX/2.0), //GE: These correspond to entries in DrawMenuItem().
                ((130.0/600.0)-(108.0*DrawScale/600.0))/2.0,
                (2.0*i+1.0)/6.0+(250.0*DrawScale/ResX/2.0),
                ((130.0/600.0)+(108.0*DrawScale/600.0))/2.0))
                || (i >= 3
                && FInRect(event.motion.x/ResX, event.motion.y/ResY,
                (2.0*(i-3.0)+1.0)/6.0-(250.0*DrawScale/ResX/2.0),
                ((600.0-130.0/2.0)-(108.0*DrawScale/2.0))/600.0,
                (2.0*(i-3.0)+1.0)/6.0+(250.0*DrawScale/ResX/2.0),
                ((600.0-130.0/2.0)+(108.0*DrawScale/2.0))/600.0))
                )
                {
                    if (LitButton < 0) //GE: We are on a button, and there are no lit buttons. Light the current one.
                    {
                        DrawMenuBackground();
                        DrawMenuItem(i, 1);
                        UpdateScreen();
                        LitButton = i;
                    }
                }
                else if (LitButton == i) //GE: We are not on the current button, yet it is lit.
                {
                    DrawMenuBackground();
                    UpdateScreen();
                    LitButton = -1;
                }
		    }
		    break;
		case SDL_MOUSEBUTTONUP:
		    if (event.button.button==SDL_BUTTON_LEFT)
		    {
                for (i=0; i<6; i++)
                {
                    if ( (i < 3
                    && FInRect(event.motion.x/ResX, event.motion.y/ResY,
                    (2.0*i+1.0)/6.0-(250.0*DrawScale/ResX/2.0), //GE: These correspond to entries in DrawMenuItem().
                    ((130.0/600.0)-(108.0*DrawScale/600.0))/2.0,
                    (2.0*i+1.0)/6.0+(250.0*DrawScale/ResX/2.0),
                    ((130.0/600.0)+(108.0*DrawScale/600.0))/2.0))
                    || (i >= 3
                    && FInRect(event.motion.x/ResX, event.motion.y/ResY,
                    (2.0*(i-3.0)+1.0)/6.0-(250.0*DrawScale/ResX/2.0),
                    ((600.0-130.0/2.0)-(108.0*DrawScale/2.0))/600.0,
                    (2.0*(i-3.0)+1.0)/6.0+(250.0*DrawScale/ResX/2.0),
                    ((600.0-130.0/2.0)+(108.0*DrawScale/2.0))/600.0))
                    )
                    {
                        printf("Debug: Menu: MouseUp with %d\n", i);
                        value = i;
                    }
                }
                UpdateScreen();//GE: Workaround for black screen on certain drivers
                UpdateScreen();
		    }
		    break;
	    }
	    SDL_Delay(0);//CPUWAIT); //GE: FIXME: This is not the same between platforms and causes major lag in Linux.
	}
	return value;
}

void DrawBackground()
{
    int i;
    float ResX = (float)GetConfig(ResolutionX);
    float ResY = (float)GetConfig(ResolutionY);
    
    //GE: Draw the background. The whole system is a difficult way of caltulating the bounding box to fit the thing in without stretching.
    SDL_Rect SourceCoords = {0, 0, 640, 311};
    SizeF BoundingBox = {800.f/(float)GetConfig(ResolutionX), 300.f/(float)GetConfig(ResolutionY)};
    float DrawScale = FMax(BoundingBox.X/((float)TextureCoordinates[GAMEBG].X/(float)GetConfig(ResolutionX)), BoundingBox.Y/((float)TextureCoordinates[GAMEBG].Y/(float)GetConfig(ResolutionY)));
    SizeF NewSize = {((float)TextureCoordinates[GAMEBG].X/(float)GetConfig(ResolutionX))*DrawScale, ((float)TextureCoordinates[GAMEBG].Y/(float)GetConfig(ResolutionY))*DrawScale};
    SizeF Pivot = {(BoundingBox.X-NewSize.X)/2.f, (BoundingBox.Y-NewSize.Y)/2.f};
    SizeF DestinationCoords = {Pivot.X+0.f, Pivot.Y+(BoundingBox.Y/2.f)};
    DrawTexture(GfxData[GAMEBG], TextureCoordinates[GAMEBG], SourceCoords, DestinationCoords, DrawScale);
    
    
    //GE: Draw the card area backgrounds.
    SizeF DestCoords = {0.0, 0.0};
    SizeF DestWH = {1.f, 129.f/600.f};
    SDL_Colour RectCol = {0,16,8,255};
    DrawRectangle(DestCoords, DestWH, RectCol);
    DestCoords.Y = (600.0-129.0)/600.0;
    DrawRectangle(DestCoords, DestWH, RectCol);
    
    //GE: Draw the gradients on top and bottom of the screen.
    DestCoords.Y = 129.0/600.0;
    DestWH.Y = 14.3/600.0;
    SDL_Colour RectColA = {0,16,8,255};
    SDL_Colour RectColB = {16,66,41,255};
    DrawGradient(DestCoords, DestWH, RectColA, RectColB);
    DestCoords.Y = 143.3/600.0;
    DestWH.Y = 7.7/600.0;
    RectColA.r=16; RectColA.g=66; RectColA.b=41;
    RectColB.r=57; RectColB.g=115; RectColB.b=82;
    DrawGradient(DestCoords, DestWH, RectColA, RectColB);
    
    DestCoords.Y = 450.0/600.0;
    DestWH.Y = 7.7/600.0;
    RectColA.r=57; RectColA.g=115; RectColA.b=82;
    RectColB.r=16; RectColB.g=66; RectColB.b=41;
    DrawGradient(DestCoords, DestWH, RectColA, RectColB);
    DestCoords.Y = (450.0+7.7)/600.0;
    DestWH.Y = 14.3/600.0;
    RectColA.r=16; RectColA.g=66; RectColA.b=41;
    RectColB.r=0; RectColB.g=16; RectColB.b=8;
    DrawGradient(DestCoords, DestWH, RectColA, RectColB);
}

void DrawUI()
{
    //GE: Draw status boxes
    SDL_Rect ItemPosition;
    SizeF ScreenPosition;
    float DrawScale = GetDrawScale();
    
    ItemPosition.x = 1181; ItemPosition.y = 0;
    ItemPosition.w = 78; ItemPosition.h = 216;
    
    ScreenPosition.X = 8.0/800.0; ScreenPosition.Y = 196.0/600.0;
    
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale*2.0);
    
    ScreenPosition.X = (800.0-8.0-78.0)/800.0; ScreenPosition.Y = 196.0/600.0;
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale*2.0);
    
    //GE: Draw two towers
    ItemPosition.x = 1000; ItemPosition.y = 0;
    ItemPosition.w = 68; ItemPosition.h = 94+200*((float)GetConfig(TowerLevels)/(float)GetConfig(TowerVictory));
    
    ScreenPosition.X = 92.0/800.0; ScreenPosition.Y = (433.0-(float)ItemPosition.h)/600.0;
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale*2.0*284.0/294.0);
    
    ItemPosition.x = 1068;
    ScreenPosition.X = (800.0-ItemPosition.w-92.0)/800.0;
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale*2.0*284.0/294.0);
    
    //GE: Draw two walls
    ItemPosition.x = 1136; ItemPosition.y = 0;
    ItemPosition.w = 45; ItemPosition.h = 38+200*((float)GetConfig(WallLevels)/(float)GetConfig(MaxWall));
    
    ScreenPosition.X = 162.0/800.0; ScreenPosition.Y = (433.0-(float)ItemPosition.h)/600.0;
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale*2.0*284.0/294.0);
    
    ScreenPosition.X = (800.0-ItemPosition.w-162.0)/800.0;
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale*2.0*284.0/294.0);
    
    //GE: Draw the tower/wall boxes
    //GE: Tower
    ItemPosition.x = 1246; ItemPosition.y = 276;
    ItemPosition.w = 98; ItemPosition.h = 48;
    
    ScreenPosition.X = 100.0/800.0; ScreenPosition.Y = 434.0/600.0;
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale);
    
    ScreenPosition.X = (800.0-(ItemPosition.w*DrawScale)-100.0)/800.0;
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale);
    
    //GE: Wall
    ItemPosition.x = 1162; ItemPosition.y = 276;
    ItemPosition.w = 84; ItemPosition.h = 48;
    
    ScreenPosition.X = 163.0/800.0; ScreenPosition.Y = 434.0/600.0;
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale);
    
    ScreenPosition.X = (800.0-(ItemPosition.w*DrawScale)-163.0)/800.0;
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale);
    
    //GE: Names
    ItemPosition.x = 1188; ItemPosition.y = 228;
    ItemPosition.w = 156; ItemPosition.h = 48;
    
    ScreenPosition.X = 8.0/800.0; ScreenPosition.Y = 162.0/600.0;
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale);
    
    ScreenPosition.X = (800.0-(ItemPosition.w*DrawScale)-8.0)/800.0;
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale);
    
}

void DrawLogo()
{
    SDL_Rect ItemPosition;
    SizeF ScreenPosition;
    float DrawScale = GetDrawScale();
    
    ScreenPosition.X = (800.0/2.0-TextureCoordinates[TITLE].X/2.0)/800.0; ScreenPosition.Y = (600.0/2.0-TextureCoordinates[TITLE].Y/2.0)/600.0;
    
    ItemPosition.x = 0; ItemPosition.y = 0;
    ItemPosition.w = (int)TextureCoordinates[TITLE].X; ItemPosition.h = (int)TextureCoordinates[TITLE].Y;
    
    DrawTexture(GfxData[TITLE], TextureCoordinates[TITLE], ItemPosition, ScreenPosition, DrawScale*2.0);
}

////////////////////////////////////////////////////////////////////////////////

/**
 * Returns the element draw size depending on the currently selected
 * window resolution.
 */ 
inline float GetDrawScale()
{
    return FMin((float)GetConfig(ResolutionX)/1600.0, (float)GetConfig(ResolutionY)/1200.0);
}

float FMax(float A, float B)
{
    if (A >= B)
        return A;
    else
        return B;
}

float FMin(float A, float B)
{
    if (A <= B)
        return A;
    else
        return B;
}

/**
 * Returns the whole size of the texture in SDL_Rect
 */ 
SDL_Rect AbsoluteTextureSize(Size TextureSize)
{
    SDL_Rect Result;
    Result.x = 0; Result.w = TextureSize.X;
    Result.y = 0; Result.h = TextureSize.Y;
    return Result;
}

int ValidInputChar(int c)
{
	if ((c>='a')&&(c<='z')) return c;
	if ((c=='.')||(c=='-')) return c;
	if ((c>='0')&&(c<='9')) return c;
	if ((c>='A')&&(c<='Z')) return c;
	if (c==' ') return c;
	if ((c>=SDLK_KP0)&&(c<=SDLK_KP9)) return c-SDLK_KP0+'0';
	if (c==SDLK_KP_PERIOD) return '.';
	return 0;
}

char *DialogBox(int type,const char *fmt,...)
{
	SDL_Rect rect;
	char *val;
	int vallen;
	va_list args;
	char *buf;
	int i,h,cnt=0;
	char *ptr[20];
	char *p;

	buf=(char *)malloc(4096);
	val=(char *)malloc(4096);
	va_start(args,fmt);
	vsnprintf(buf,4095,fmt,args);
	va_end(args);

	p=buf;
	ptr[0]=p;cnt=1;
	while (*p)
	{
		if (*p=='\n')
		{
			ptr[cnt++]=p+1;
			*p=0;
		}
		p++;
	}
	rect.w=352;
	rect.h=128;
	rect.x=(GetConfig(ResolutionX)-rect.w) >> 1;
	rect.y=(GetConfig(ResolutionY)-rect.h) >> 1;
	//SDL_BlitSurface(GfxData[type],NULL,GfxData[SCREEN],&rect);//FIXME
	rect.w-=4;
	rect.h-=4;
	rect.x+=2;
	rect.y+=2;
	
	if (type==DLGWINNER || type==DLGLOOSER)
		BFont_SetCurrentFont(bigfont);
	
	h=BFont_FontHeight(BFont_GetCurrentFont());
	/*for (i=0;i<cnt;i++)
		BFont_CenteredPutString(GfxData[SCREEN],240-h*cnt/2+h*i,ptr[i]);
	UpdateScreenRect(rect.x-2,rect.y-2,rect.w+4,rect.h+4);*///FIXME
	
	if (type==DLGWINNER || type==DLGLOOSER)
		BFont_SetCurrentFont(font);

	free(buf);

	if (type!=DLGNETWORK) return NULL;

	val[0]='_';val[1]=0;vallen=1;h=0;

	while (!h)
	{
		rect.x=160;
		rect.y=272;
		rect.w=320;
		rect.h=16;
		//SDL_FillRect(GfxData[SCREEN],&rect,0);
		i=BFont_TextWidth(val);
		/*if (i<312)
			BFont_PutString(GfxData[SCREEN],164,276,val);
		else
			BFont_PutString(GfxData[SCREEN],164+(312-i),276,val);
		SDL_UpdateRect(GfxData[SCREEN],160,272,320,16);*/ //FIXME 

		while (event.type!=SDL_KEYDOWN)
		{
			SDL_PollEvent(&event);		// wait for keypress
			SDL_Delay(CPUWAIT);
		}
		if (event.type==SDL_KEYDOWN)
		{
			i=ValidInputChar(event.key.keysym.sym);
			if (i&&(vallen<4095))
			{
				val[vallen-1]=i;
				val[vallen++]='_';
				val[vallen]=0;
			}
			if (((vallen>1)&&(event.key.keysym.sym==SDLK_BACKSPACE))||(event.key.keysym.sym==SDLK_DELETE))
			{
				val[--vallen]=0;
				val[vallen-1]='_';
			}
			if ((event.key.keysym.sym==SDLK_KP_ENTER)||(event.key.keysym.sym==SDLK_RETURN)) h=1;
			if (event.key.keysym.sym==SDLK_ESCAPE) h=2;
			while (event.type!=SDL_KEYUP)
			{
				SDL_PollEvent(&event);	// wait for keyrelease
					SDL_Delay(CPUWAIT);
			}
		}
	}
	if (h==2)
		return NULL;
	else
	{
		val[vallen-1]=0;
		return val;
	}
}

int InRect(int x, int y, int x1, int y1, int x2, int y2)
{
	return (x>=x1)&&(x<=x2)&&(y>=y1)&&(y<=y2);
}

float FInRect(float x, float y, float x1, float y1, float x2, float y2)
{
	return (x>=x1)&&(x<=x2)&&(y>=y1)&&(y<=y2);
}

void LoadSurface(char* filename, int Slot)
{
	SDL_Surface* Surface; Surface = IMG_Load(filename);
	if (!Surface)
	    FatalError("File '%s' is missing or corrupt.",filename);
	GfxData[Slot] = SurfaceToTexture(Surface);
	TextureCoordinates[Slot].X = (*Surface).w; TextureCoordinates[Slot].Y = (*Surface).h;
	SDL_FreeSurface(Surface);
}

/*void DrawRectangle(int x, int y, int w, int h, int Colour)
{
    SDL_Rect rec;*/
    
    //GE: 4 "fill" rects.
	/*rec.x=x; rec.y=y; rec.w=w; rec.h=1; //DEPRECATED
	SDL_FillRect(GfxData[SCREEN], &rec, Colour);
	rec.x=x; rec.y=y; rec.w=1; rec.h=h;
	SDL_FillRect(GfxData[SCREEN], &rec, Colour);
	rec.x=x+w-1; rec.y=y; rec.w=1; rec.h=h;
	SDL_FillRect(GfxData[SCREEN], &rec, Colour);
	rec.x=x; rec.y=y+h-1; rec.w=w; rec.h=1;
	SDL_FillRect(GfxData[SCREEN], &rec, Colour);*/
//}

void DoCredits()
{
	#define HGHT 30
	char *text[]={
		"Arcomage 1.alpha.XX.XX" /*ARCOVER,*/
		"by STiCK and GreatEmerald (2005-2011)",
		"",
		"This program was originally created",
		"as Individual Software Project",
		"at Charles University",
		"Prague, Czech Republic",
		"",
		"Since 2009 it became easier to access",
		"and its development was continued.",
		"",
		"http://stick.gk2.sk/projects/arcomage/",
		"",
		"This is a clone of Arcomage, a card game",
		"originally released by New World Computing",
		"and the 3DO Company as a part of",
		"Might and Magic VII: For Blood and Honor",
		"and re-released in 2001 as a stand-alone",
		"application.",
		"Since it didn't support any kind of",
		"modifications, this open source project",
		"aims to completely remake the original",
		"and make it more flexible.",
		"",
		"Original credits follow.",
		"",
		NULL
	};
	int i,ypos=GetConfig(ResolutionY);
	BFont_SetCurrentFont(bigfont);
	while (event.type!=SDL_KEYDOWN || event.key.keysym.sym!=SDLK_ESCAPE)
	{
		//Blit(CREDITS,SCREEN);
		i=0;
		while (text[i])
		{
			/*if (ypos+i*HGHT>=-20 && ypos+i*HGHT<=GetConfig(ResolutionX))
				BFont_CenteredPutString(GfxData[SCREEN],ypos+i*HGHT,text[i]);
			i++;*///FIXME
		}
		UpdateScreen();
		SDL_Delay(20);
		ypos--;
		SDL_PollEvent(&event);
	}
	BFont_SetCurrentFont(font);
}
