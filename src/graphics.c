#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "BFont.h"
//#include "resize.h"
//#include "cards.h"
//#include "common.h"
//#include "config.h"
#include "frontend.h"
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

int CardsOnTableSize=0;
TableCard* CardsOnTable=NULL;
char CardInTransit = -1;

const int CPUWAIT=10; //DEBUG

void Graphics_Init()
{
    char fullscreen = GetConfig(Fullscreen);
    
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE)<0) //GE: NOPARACHUTE means no exception handling. Could be dangerous. Could be faster.
	GeneralProtectionFault("Couldn't initialise SDL");
    SDL_WM_SetCaption("Arcomage Clone",NULL);
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ); //GE: Enable OpenGL double buffer support.
    if (!SDL_SetVideoMode(GetConfig(ResolutionX),GetConfig(ResolutionY),0,(fullscreen*SDL_FULLSCREEN)|SDL_OPENGL)) //GE: Enable OpenGL, because without it SDL is just plain bad.
	GeneralProtectionFault("Couldn't initialise OpenGL");
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
	GeneralProtectionFault("Data file 'nums_small.png' is missing or corrupt.");
    bigfont=BFont_LoadFont(GetFilePath("bigfont.png"));
    if (!bigfont)
	GeneralProtectionFault("Data file 'bigfont.png' is missing or corrupt.");
    font=BFont_LoadFont(GetFilePath("font.png"));
    if (!font)
	GeneralProtectionFault("Data file 'font.png' is missing or corrupt.");
    BFont_SetCurrentFont(font);*/
    
    InitCardLocations(2);
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
    
    PrecacheFonts();
    
    PrecachePictures(NumPools, NumCards);
}

void PrecachePictures(int NumPools, int* NumCards)
{
    char*** PicturePaths = GetCardPicturePaths();
    SDL_Rect** PictureCoords = GetCardPictureCoords();
    int Pool, Card, EarlierCard;
    int ArraySize=0;
    int CurrentElement=0;
    char bDuplicate=0;
    SDL_Surface* Surface;
    int* PictureMapping;
    int i;
    
    //GEm: Find out what size the PictureFileCache array has to be.
    for (Pool=0; Pool<NumPools; Pool++)
    {
        for (Card=0; Card<NumCards[Pool]; Card++)
        {
            for (EarlierCard=0; EarlierCard<Card; EarlierCard++)
            {
                if (strcmp(PicturePaths[Pool][EarlierCard], PicturePaths[Pool][Card]) == 0)
                {
                    bDuplicate = 1;
                    break;
                }
            }
            if (!bDuplicate) //GEm: 0th element passes this automatically
                ArraySize++;
            bDuplicate = 0;
        }
    }
    
    //GEm: Allocate the needed memory.
    PictureFileCacheSize = ArraySize;
    PictureFileCache = (OpenGLTexture*) malloc(PictureFileCacheSize * sizeof(OpenGLTexture));
    PictureMapping = (int*) malloc(PictureFileCacheSize * sizeof(int));
    
    //GEm: Do the same thing, just write data this time.
    bDuplicate = 0;
    for (Pool=0; Pool<NumPools; Pool++)
    {
        for (Card=0; Card<NumCards[Pool]; Card++)
        {
            for (EarlierCard=0; EarlierCard<Card; EarlierCard++)
            {
                if (strcmp(PicturePaths[Pool][EarlierCard], PicturePaths[Pool][Card]) == 0)
                {
                    for (i=0; i<PictureFileCacheSize; i++)
                    {
                        if (EarlierCard == PictureMapping[i])
                        {
                            CardCache[Pool][Card].PictureHandle = i;
                            CardCache[Pool][Card].PictureCoords = PictureCoords[Pool][Card];
                            break;
                        }
                    }
                    bDuplicate = 1;
                    break;
                }
            }
            if (!bDuplicate)
            {
                Surface = IMG_Load(PicturePaths[Pool][Card]);
                if (!Surface)
                    GeneralProtectionFault("File '%s' is missing or corrupt.", PicturePaths[Pool][Card]);
                PictureFileCache[CurrentElement].Texture = SurfaceToTexture(Surface);
                PictureFileCache[CurrentElement].TextureSize.X = (*Surface).w;
                PictureFileCache[CurrentElement].TextureSize.Y = (*Surface).h;
                SDL_FreeSurface(Surface); Surface = NULL;
                
                CardCache[Pool][Card].PictureHandle = CurrentElement;
                CardCache[Pool][Card].PictureCoords = PictureCoords[Pool][Card];
                
                PictureMapping[CurrentElement] = Card;
                
                CurrentElement++;
            }
            bDuplicate = 0;
        }
    }
    
    free(PictureMapping);
    
    for (Pool=0; Pool<NumPools; Pool++)
    {
        free(PicturePaths[Pool]);
        free(PictureCoords[Pool]);
    }
    free(PicturePaths);
    free(PictureCoords);
    
    free(NumCards);
}

/**
 * Initialise the position of each card in both hands. It is slightly randomised
 * on Z to provide an illusion of being true cards (which are rarely neatly
 * aligned in the real world).
 */ 
void InitCardLocations(int NumPlayers)
{
    int i, n;
    int NumCards = GetConfig(CardsInHand);
    float DrawScale = GetDrawScale();
    float CardWidth = NumCards*192*DrawScale/GetConfig(ResolutionX);
    float Spacing = (1.0-CardWidth)/(NumCards+1);
    
    CardLocations = (SizeF**) malloc(NumPlayers * sizeof(SizeF*));
    for (i=0; i < NumPlayers; i++)
    {
        CardLocations[i] = (SizeF*) malloc(NumCards * sizeof(SizeF));
        for (n=0; n < NumCards; n++)
        {
            CardLocations[i][n].X = Spacing*(n+1)+192*DrawScale*n/GetConfig(ResolutionX);
            CardLocations[i][n].Y = ((FRand()*12.0-6.0)+(6 + 466*!i))/600.0; //GEm: TODO: Implement more than 2 players - how to solve this?
        }
    }
    
    
}

void Graphics_Quit()
{
	int i;
    
    for (i=0; i < 2; i++) //GEm: TODO: Implement multiple players
        free(CardLocations[i]);
    free(CardLocations);
    
    //GEm: TODO: Free CardCache and all its textures
    //printf("DEBUG: Graphics_Quit: Freeing...\n");
    for (i=0; i<PictureFileCacheSize; i++)
        FreeTextures(PictureFileCache[i].Texture);
    free(PictureFileCache);
    
	for (i=0;i<GFX_CNT;i++)
		FreeTextures(GfxData[i]);
    
    if (CardsOnTableSize > 0)
        free(CardsOnTable);
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

//void FillRect(int x,int y,int w,int h,Uint8 r,Uint8 g,Uint8 b)
//{
//	printf("Warning: FillRect is deprecated!");
	/*SDL_Rect rect;
	rect.x=x;rect.y=y;rect.w=w;rect.h=h;
	SDL_FillRect(GfxData[SCREEN],&rect,SDL_MapRGB(GfxData[SCREEN]->format,r,g,b));*/
//}

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
 * Draws a card on the screen by a given card handle.
 * 
 * In order to do that, the function needs to know which card to draw
 * and where. A card itself consists of the background, picture and
 * text for the name, description and cost(s). This function requires the full
 * card handle, as defined in D's CardDB(). For the function that needs only
 * the current player number and the place in hand, see DrawCardAlpha().
 */

void DrawHandleCardAlpha(int Pool, int Card, float X, float Y, float Alpha)
{
    int i;
    float ResX = (float)GetConfig(ResolutionX);
    float ResY = (float)GetConfig(ResolutionY);
    
    SizeF BoundingBox, TextureSize;
    float Spacing;
    int BlockHeight=0;
    
    //GE: Draw the background.
    //GE: First, get the background that we will be using.
    int Colour = GetColourType(Pool, Card);
    
    SDL_Rect ItemPosition;
    SizeF ScreenPosition; ScreenPosition.X = X; ScreenPosition.Y = Y;
    float DrawScale = GetDrawScale();
    
    //GEm: Draw background.
    ItemPosition.x = Colour * 192;  ItemPosition.w = 192; //GE: Careful here. Colours must be in the same order as they are in the picture and the adapter must match.
    ItemPosition.y = 324;           ItemPosition.h = 256;
    
    DrawTextureAlpha(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale, Alpha);
    
    //GEm: Draw title text.
    ItemPosition = AbsoluteTextureSize(CardCache[Pool][Card].TitleTexture.TextureSize);
    
    ScreenPosition.X += 4/ResX; ScreenPosition.Y += 4/ResY;
    BoundingBox.X = 88/ResX; BoundingBox.Y = 12/ResY;
    TextureSize.X = CardCache[Pool][Card].TitleTexture.TextureSize.X/ResX; TextureSize.Y = CardCache[Pool][Card].TitleTexture.TextureSize.Y/ResY;
    ScreenPosition = CentreOnX(ScreenPosition, TextureSize, BoundingBox);
    
    DrawTextureAlpha(CardCache[Pool][Card].TitleTexture.Texture, CardCache[Pool][Card].TitleTexture.TextureSize, ItemPosition, ScreenPosition, 1.0, Alpha);
    
    //GEm: Draw description text.
    ScreenPosition.X = X + 4/ResX; ScreenPosition.Y = Y + 72/ResY;
    for (i=0; i<CardCache[Pool][Card].DescriptionNum; i++)
        BlockHeight += CardCache[Pool][Card].DescriptionTextures[i].TextureSize.Y; //GEm: Alternatively, I could just multiply one by DescriptionNum, but four iterations are not much.
    if (CardCache[Pool][Card].DescriptionTextures[CardCache[Pool][Card].DescriptionNum].TextureSize.X > 66*DrawScale*2 && CardCache[Pool][Card].DescriptionNum > 1 && BlockHeight <= 41*DrawScale*2) //GEm: If we'd overlap with price and have enough space
        Spacing = ((41*DrawScale*2-BlockHeight)/(CardCache[Pool][Card].DescriptionNum+1))/ResY;
    else
        Spacing = ((53*DrawScale*2-BlockHeight)/(CardCache[Pool][Card].DescriptionNum+1))/ResY;
    ScreenPosition.Y += Spacing;
    for (i=0; i<CardCache[Pool][Card].DescriptionNum; i++)
    {
        ItemPosition = AbsoluteTextureSize(CardCache[Pool][Card].DescriptionTextures[i].TextureSize);
        TextureSize.X = CardCache[Pool][Card].DescriptionTextures[i].TextureSize.X/ResX; TextureSize.Y = CardCache[Pool][Card].DescriptionTextures[i].TextureSize.Y/ResY;
        ScreenPosition = CentreOnX(ScreenPosition, TextureSize, BoundingBox);
        DrawTextureAlpha(CardCache[Pool][Card].DescriptionTextures[i].Texture, CardCache[Pool][Card].DescriptionTextures[i].TextureSize, ItemPosition, ScreenPosition, 1.0, Alpha);
        ScreenPosition.Y += Spacing + CardCache[Pool][Card].DescriptionTextures[i].TextureSize.Y/ResY;
        ScreenPosition.X = X + 4/ResX; //GEm: Reset X, keep Y.
    }
    
    //GEm: Draw card cost.
    BoundingBox.X = 19/800.0; BoundingBox.Y = 12/600.0;
    ScreenPosition.X = X + 77/800.0; ScreenPosition.Y = Y + 111/600.0;
    switch (Colour)
    {
        case CT_Blue:
            ItemPosition = AbsoluteTextureSize(CardCache[Pool][Card].PriceTexture[1].TextureSize);
            TextureSize.X = CardCache[Pool][Card].PriceTexture[1].TextureSize.X/ResX; TextureSize.Y = CardCache[Pool][Card].PriceTexture[1].TextureSize.Y/ResY;
            ScreenPosition = CentreOnX(ScreenPosition, TextureSize, BoundingBox);
            DrawTextureAlpha(CardCache[Pool][Card].PriceTexture[1].Texture, CardCache[Pool][Card].PriceTexture[1].TextureSize, ItemPosition, ScreenPosition, 1.0, Alpha);
            break;
        case CT_Green:
            ItemPosition = AbsoluteTextureSize(CardCache[Pool][Card].PriceTexture[2].TextureSize);
            TextureSize.X = CardCache[Pool][Card].PriceTexture[2].TextureSize.X/ResX; TextureSize.Y = CardCache[Pool][Card].PriceTexture[2].TextureSize.Y/ResY;
            ScreenPosition = CentreOnX(ScreenPosition, TextureSize, BoundingBox);
            DrawTextureAlpha(CardCache[Pool][Card].PriceTexture[2].Texture, CardCache[Pool][Card].PriceTexture[2].TextureSize, ItemPosition, ScreenPosition, 1.0, Alpha);
            break;
        case CT_White:
            GeneralProtectionFault("FIXME: White cards not yet supported!");
            break;
        default: //GEm: Black and red cards, and anything else strange goes here.
            ItemPosition = AbsoluteTextureSize(CardCache[Pool][Card].PriceTexture[0].TextureSize);
            TextureSize.X = CardCache[Pool][Card].PriceTexture[0].TextureSize.X/ResX; TextureSize.Y = CardCache[Pool][Card].PriceTexture[0].TextureSize.Y/ResY;
            ScreenPosition = CentreOnX(ScreenPosition, TextureSize, BoundingBox);
            DrawTextureAlpha(CardCache[Pool][Card].PriceTexture[0].Texture, CardCache[Pool][Card].PriceTexture[0].TextureSize, ItemPosition, ScreenPosition, 1.0, Alpha);
            break;
    }
    
    
	//GEm: Draw card image.
    ItemPosition = CardCache[Pool][Card].PictureCoords;
    BoundingBox.X = 88/800.0; BoundingBox.Y = 52/600.0;
    float CustomDrawScale = FMax(BoundingBox.X/(ItemPosition.w/ResX), BoundingBox.Y/(ItemPosition.h/ResY));
    SizeF NewSize; NewSize.X = (ItemPosition.w/ResX)*CustomDrawScale; NewSize.Y = (ItemPosition.h/ResY)*CustomDrawScale;
    SizeF DeltaSize; DeltaSize.X = NewSize.X-BoundingBox.X; DeltaSize.Y = NewSize.Y-BoundingBox.Y;
    ItemPosition.x += DeltaSize.X*ResX/2.0; ItemPosition.w -=  DeltaSize.X*ResX;
    ItemPosition.y += DeltaSize.Y*ResY/2.0; ItemPosition.h -=  DeltaSize.Y*ResY;
    ScreenPosition.X = X + 4/800.0; ScreenPosition.Y = Y + 19/600.0;
    DrawTextureAlpha(PictureFileCache[CardCache[Pool][Card].PictureHandle].Texture, PictureFileCache[CardCache[Pool][Card].PictureHandle].TextureSize, ItemPosition, ScreenPosition, CustomDrawScale, Alpha);
}

inline void DrawHandleCard(int Pool, int Card, float X, float Y)
{
    DrawHandleCardAlpha(Pool, Card, X, Y, 1.0);
}

void DrawCardAlpha(char Player, char Number, float X, float Y, float Alpha)
{
    int Pool, Card;
    GetCardHandle(Player, Number, &Pool, &Card);
    
    DrawHandleCardAlpha(Pool, Card, X, Y, Alpha);
}

inline void DrawCard(char Player, char Number, float X, float Y)
{
	DrawCardAlpha(Player, Number, X, Y, 1.0);
}

void DrawAllPlayerCards()
{
    int i, n;
    
    for (n=0; n<2; n++) //GEm: TODO More than 2 players
    {
        for (i=0; i<GetConfig(CardsInHand); i++)
        {
            if (GetCanAffordCard(n, i))
                DrawCard(n, i, CardLocations[n][i].X, CardLocations[n][i].Y);
            else
                DrawCardAlpha(n, i, CardLocations[n][i].X, CardLocations[n][i].Y, GetConfig(CardTranslucency)/255.0);
        }
    }
}

void DrawXPlayerCards(int PlayerNum, int CardNum)
{
    int i, n;
    
    for (n=0; n<2; n++) //GEm: TODO More than 2 players
    {
        for (i=0; i<GetConfig(CardsInHand); i++)
        {
            if (n == PlayerNum && i == CardNum)
                continue;
            if (GetCanAffordCard(n, i))
                DrawCard(n, i, CardLocations[n][i].X, CardLocations[n][i].Y);
            else
                DrawCardAlpha(n, i, CardLocations[n][i].X, CardLocations[n][i].Y, GetConfig(CardTranslucency)/255.0);
        }
    }
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

void DrawDiscard(float X, float Y)
{
    SDL_Rect ItemPosition;
    SizeF ScreenPosition; ScreenPosition.X = X; ScreenPosition.Y = Y;
    float DrawScale = GetDrawScale();
    SizeF TextureSize, CardSize;
    
    ItemPosition.x = 1259;  ItemPosition.w = 146;
    ItemPosition.y = 0;     ItemPosition.h = 32;
    
    TextureSize.X = 146*DrawScale/(float)GetConfig(ResolutionX);
    TextureSize.Y = 32*DrawScale/(float)GetConfig(ResolutionX);
    CardSize.X = 192*DrawScale/(float)GetConfig(ResolutionX);
    CardSize.Y = 256*DrawScale/(float)GetConfig(ResolutionX);
    ScreenPosition = CentreOnX(ScreenPosition, TextureSize, CardSize);
    ScreenPosition.Y += (CardSize.Y - TextureSize.Y)/2.0;
    
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale);
}

void DrawCardsOnTable()
{
    int i;
    SizeF Destination;
    
    for (i=0; i<CardsOnTableSize; i++)
    {
        Destination = GetCardOnTableLocation(i+1);
        DrawHandleCardAlpha(CardsOnTable[i].CH.Pool, CardsOnTable[i].CH.Card, Destination.X, Destination.Y, GetConfig(CardTranslucency)/255.0);
        if (CardsOnTable[i].bDiscarded)
            DrawDiscard(Destination.X, Destination.Y);
    }
}

/**
 * Draw the cards on table, except for the last one.
 */ 
void DrawXCardsOnTable()
{
    int i;
    SizeF Destination;
    
    for (i=0; i<CardsOnTableSize-1; i++)
    {
        Destination = GetCardOnTableLocation(i+1);
        DrawHandleCardAlpha(CardsOnTable[i].CH.Pool, CardsOnTable[i].CH.Card, Destination.X, Destination.Y, GetConfig(CardTranslucency)/255.0);
        if (CardsOnTable[i].bDiscarded)
            DrawDiscard(Destination.X, Destination.Y);
    }
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
                Resource = GetResource(Player, RT_Bricks);
                break;
            case 1:
                Resource = GetResource(Player, RT_Gems);
                break;
            default:
                Resource = GetResource(Player, RT_Recruits);
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
                Resource = GetResource(Player, RT_Quarry);
                break;
            case 1:
                Resource = GetResource(Player, RT_Magic);
                break;
            default:
                Resource = GetResource(Player, RT_Dungeon);
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
        DrawSmallNumber(GetResource(i, RT_Tower), ScreenPosition, BoundingBox);
        
        //GEm: Draw the wall height.
        ScreenPosition.X = (166+433*i)/800.0; ScreenPosition.Y = (443-3)/600.0;
        BoundingBox.X = 36/800.0; BoundingBox.Y = 7/600.0;
        DrawSmallNumber(GetResource(i, RT_Wall), ScreenPosition, BoundingBox);
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
                        //printf("Debug: Menu: MouseUp with %d\n", i);
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
    SDL_Rect SourceCoords = {0,0,0,0};
    SourceCoords.w = TextureCoordinates[GAMEBG].X; SourceCoords.h = TextureCoordinates[GAMEBG].Y;
    SizeF BoundingBox; BoundingBox.X = 800.f/(float)GetConfig(ResolutionX); BoundingBox.Y = 300.f/(float)GetConfig(ResolutionY);
    float DrawScale = FMax(BoundingBox.X/((float)TextureCoordinates[GAMEBG].X/(float)GetConfig(ResolutionX)), BoundingBox.Y/((float)TextureCoordinates[GAMEBG].Y/(float)GetConfig(ResolutionY)));
    SizeF NewSize; NewSize.X = ((float)TextureCoordinates[GAMEBG].X/(float)GetConfig(ResolutionX))*DrawScale; NewSize.Y = ((float)TextureCoordinates[GAMEBG].Y/(float)GetConfig(ResolutionY))*DrawScale;
    SizeF Pivot; Pivot.X = (BoundingBox.X-NewSize.X)/2.f; Pivot.Y = (BoundingBox.Y-NewSize.Y)/2.f;
    SizeF DestinationCoords; DestinationCoords.X = Pivot.X+0.f; DestinationCoords.Y = Pivot.Y+(BoundingBox.Y/2.f);
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
    ItemPosition.w = 68; ItemPosition.h = 94+200*(GetResource(0, RT_Tower)/(float)GetConfig(TowerVictory)); //GEm: TODO: Implement more than 2 players
    
    ScreenPosition.X = 92.0/800.0; ScreenPosition.Y = (433.0-(float)ItemPosition.h)/600.0;
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale*2.0*284.0/294.0);
    
    ItemPosition.x = 1068;
    ItemPosition.h = 94+200*(GetResource(1, RT_Tower)/(float)GetConfig(TowerVictory));
    ScreenPosition.X = (800.0-ItemPosition.w-92.0)/800.0; ScreenPosition.Y = (433.0-(float)ItemPosition.h)/600.0;
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale*2.0*284.0/294.0);
    
    //GE: Draw two walls
    ItemPosition.x = 1136; ItemPosition.y = 0;
    ItemPosition.w = 45; ItemPosition.h = 38+200*(GetResource(0, RT_Wall)/(float)GetConfig(MaxWall));
    
    ScreenPosition.X = 162.0/800.0; ScreenPosition.Y = (433.0-(float)ItemPosition.h)/600.0;
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], ItemPosition, ScreenPosition, DrawScale*2.0*284.0/294.0);
    
    ItemPosition.h = 38+200*(GetResource(1, RT_Wall)/(float)GetConfig(MaxWall));
    ScreenPosition.X = (800.0-ItemPosition.w-162.0)/800.0; ScreenPosition.Y = (433.0-(float)ItemPosition.h)/600.0;
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

/**
 * Draws all of the non-moving elements in the main game scene. That means
 * everything except for cards.
 */
void DrawScene()
{
    ClearScreen();
    DrawBackground();
    SizeF BankLocation = GetCardOnTableLocation(0);
    DrawFoldedAlpha(0, BankLocation.X, BankLocation.Y, (float)GetConfig(CardTranslucency)/255.0);
    DrawUI();
    DrawStatus();
    if (CardInTransit > -1)
    {
        DrawXCardsOnTable();
        SizeF TransitingCardLocation;
        TransitingCardLocation.X = 0.5-192*GetDrawScale()/2/800.0;
        TransitingCardLocation.Y = 0.5-256*GetDrawScale()/2/600.0;
        if (bDiscardedInTransit)
        {
            DrawHandleCardAlpha(CardsOnTable[CardsOnTableSize-1].CH.Pool, CardsOnTable[CardsOnTableSize-1].CH.Card, TransitingCardLocation.X, TransitingCardLocation.Y, GetConfig(CardTranslucency)/255.0);
            DrawDiscard(TransitingCardLocation.X, TransitingCardLocation.Y);
        }
        else
            DrawHandleCard(CardsOnTable[CardsOnTableSize-1].CH.Pool, CardsOnTable[CardsOnTableSize-1].CH.Card, TransitingCardLocation.X, TransitingCardLocation.Y);
        DrawXPlayerCards(Turn, CardInTransit);
    }
    else
    {
        DrawCardsOnTable();
        DrawAllPlayerCards();
    }
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

void PlayCardAnimation(int CardPlace, char bDiscarded, char bSameTurn)
{
    const int FloatToHnsecs = 1000000;
    
    SizeF Destination; Destination.X = 0.5-192*GetDrawScale()/2/800.0; Destination.Y = 0.5-256*GetDrawScale()/2/600.0;
    SizeF CurrentLocation;
    long long AnimDuration = 5*FloatToHnsecs;
    long long StartTime = GetCurrentTimeD(), CurrentTime = GetCurrentTimeD();
    float ElapsedPercentage = (CurrentTime-StartTime)/(float)AnimDuration;
    
    int i;
    SizeF BankLocation = GetCardOnTableLocation(0);
    
    while (CurrentTime < StartTime + AnimDuration)
    {
        ClearScreen();
        DrawBackground();
        
        //GEm: Draw the cards moving into the bank.
        if (!bSameTurn)
        {
            for (i=0; i<CardsOnTableSize; i++)
            {
                CurrentLocation = GetCardOnTableLocation(i+1);
                CurrentLocation.X = CurrentLocation.X + (BankLocation.X - CurrentLocation.X)*ElapsedPercentage;
                CurrentLocation.Y = CurrentLocation.Y + (BankLocation.Y - CurrentLocation.Y)*ElapsedPercentage;
                DrawHandleCardAlpha(CardsOnTable[i].CH.Pool, CardsOnTable[i].CH.Card, CurrentLocation.X, CurrentLocation.Y, (1.0-ElapsedPercentage)*(GetConfig(CardTranslucency)/255.0));
            }
        }
        else
            DrawCardsOnTable();
        
        DrawFoldedAlpha(0, BankLocation.X, BankLocation.Y, (float)GetConfig(CardTranslucency)/255.0);
        DrawUI();
        DrawStatus();
        DrawXPlayerCards(Turn, CardPlace);
        
        //GEm: Draw the card in transit.
        CurrentLocation.X = CardLocations[Turn][CardPlace].X + (Destination.X - CardLocations[Turn][CardPlace].X)*ElapsedPercentage;
        CurrentLocation.Y = CardLocations[Turn][CardPlace].Y + (Destination.Y - CardLocations[Turn][CardPlace].Y)*ElapsedPercentage;
        if (bDiscarded) //GEm: If it's discarded, draw it translucent from the get-go.
            DrawCardAlpha(Turn, CardPlace, CurrentLocation.X, CurrentLocation.Y, (GetConfig(CardTranslucency)/255.0-1.0)*ElapsedPercentage+1.0);
        else
            DrawCard(Turn, CardPlace, CurrentLocation.X, CurrentLocation.Y);
        
        UpdateScreen();
        SDL_Delay(10);
        
        CurrentTime = GetCurrentTimeD();
        ElapsedPercentage = (CurrentTime-StartTime)/(float)AnimDuration;
    }
    
    CardInTransit = CardPlace;
    if (bDiscarded)
        bDiscardedInTransit = 1;
    else
        bDiscardedInTransit = 0;
        
    if (!bSameTurn) //GEm: New turn
    {
        if (CardsOnTable == NULL || CardsOnTableSize > 1) //GEm: Alternatively, use CardsOnTableMax and CardsOnTableSize to cut down on reallocation and increase RAM usage
            CardsOnTable = (TableCard*) realloc(CardsOnTable, sizeof(TableCard));
        CardsOnTableSize = 1;
    }
    else
    {
        CardsOnTableSize++;
        CardsOnTable = (TableCard*) realloc(CardsOnTable, CardsOnTableSize*sizeof(TableCard)); //GEm: Hm, there is no way to find the length of a pointer array, yet realloc does it just fine?..
    }
    GetCardHandle(Turn, CardPlace, &(CardsOnTable[CardsOnTableSize-1].CH.Pool), &(CardsOnTable[CardsOnTableSize-1].CH.Card));
    if (bDiscarded)
        CardsOnTable[CardsOnTableSize-1].bDiscarded = 1;
    else
        CardsOnTable[CardsOnTableSize-1].bDiscarded = 0;
}

/**
 * Plays the animation of the card in translit going to the right location on
 * the table and the bank handing out a card to the player.
 */ 
void PlayCardPostAnimation(int CardPlace)
{
    DrawScene();
    UpdateScreen();
    SDL_Delay(500);
    
    const int FloatToHnsecs = 1000000;
    
    SizeF Source; Source.X = 0.5-192*GetDrawScale()/2/800.0; Source.Y = 0.5-256*GetDrawScale()/2/600.0;
    SizeF Destination = GetCardOnTableLocation(CardsOnTableSize);
    SizeF CurrentLocation;
    long long AnimDuration = 5*FloatToHnsecs;
    long long StartTime = GetCurrentTimeD(), CurrentTime = GetCurrentTimeD();
    float ElapsedPercentage = (CurrentTime-StartTime)/(float)AnimDuration;
    
    //int i;
    SizeF BankLocation = GetCardOnTableLocation(0);
    //int Pool, Card;
    
    while (CurrentTime < StartTime + AnimDuration) //GEm: Move transient card to the table
    {
        ClearScreen();
        DrawBackground();
        DrawFoldedAlpha(0, BankLocation.X, BankLocation.Y, GetConfig(CardTranslucency)/255.0);
        DrawXCardsOnTable();
        DrawUI();
        DrawStatus();
        DrawXPlayerCards(Turn, CardPlace);
        
        CurrentLocation.X = Source.X + (Destination.X - Source.X)*ElapsedPercentage;
        CurrentLocation.Y = Source.Y + (Destination.Y - Source.Y)*ElapsedPercentage;
        if (bDiscardedInTransit)
        {
            DrawCardAlpha(Turn, CardPlace, CurrentLocation.X, CurrentLocation.Y, GetConfig(CardTranslucency)/255.0);
            DrawDiscard(CurrentLocation.X, CurrentLocation.Y);
        }
        else
            DrawCardAlpha(Turn, CardPlace, CurrentLocation.X, CurrentLocation.Y, (GetConfig(CardTranslucency)/255.0-1.0)*ElapsedPercentage+1.0); //GEm: (Alpha-1)*x+1=f(x)
        
        UpdateScreen();
        SDL_Delay(10);
        
        CurrentTime = GetCurrentTimeD();
        ElapsedPercentage = (CurrentTime-StartTime)/(float)AnimDuration;
    }
    
    StartTime = GetCurrentTimeD();
    CurrentTime = GetCurrentTimeD();
    ElapsedPercentage = (CurrentTime-StartTime)/(float)AnimDuration;
    
    //GetCardHandle(Turn, CardPlace, &Pool, &Card);
    CardLocations[Turn][CardPlace].Y = ((FRand()*12.0-6.0)+(6 + 466*!Turn))/600.0; //GEm: TODO implement more than 2 players
    Destination.X = CardLocations[Turn][CardPlace].X;
    Destination.Y = CardLocations[Turn][CardPlace].Y;
    
    while (CurrentTime < StartTime + AnimDuration) //GEm: Move a folded card from bank to hand
    {
        ClearScreen();
        DrawBackground();
        DrawFoldedAlpha(0, BankLocation.X, BankLocation.Y, (float)GetConfig(CardTranslucency)/255.0);
        DrawCardsOnTable();
        DrawUI();
        DrawStatus();
        DrawXPlayerCards(Turn, CardPlace);
        //DrawCard(Turn, CardPlace, Destination.X, Destination.Y);
        
        CurrentLocation.X = BankLocation.X + (Destination.X - BankLocation.X)*ElapsedPercentage;
        CurrentLocation.Y = BankLocation.Y + (Destination.Y - BankLocation.Y)*ElapsedPercentage;
        DrawFolded(Turn, CurrentLocation.X, CurrentLocation.Y);
        
        UpdateScreen();
        SDL_Delay(10);
        
        CurrentTime = GetCurrentTimeD();
        ElapsedPercentage = (CurrentTime-StartTime)/(float)AnimDuration;
    }
    
    CardInTransit = -1;
    bDiscardedInTransit = 0;
}

////////////////////////////////////////////////////////////////////////////////

/**
 * Deduces the location of a given card on the table. They are all fitted inside
 * a bounding box.
 * This could be precached, if it takes too long.
 */
SizeF GetCardOnTableLocation(int CardSlot)
{
    int i;
    SizeF Result;
    //GEm: Bounding box: 15% margin from left and right, 25% from top and bottom
    
    //GEm: Figure out how many cards fit the bounding box
    //GEm: Good thing that int = float in C means int = floor(float)!
    float CardWidth = 192*GetDrawScale()/GetConfig(ResolutionX);
    float CardHeight = 256*GetDrawScale()/GetConfig(ResolutionY);
    int CardsX = 0.7/CardWidth;
    int CardsY = 0.5/CardHeight;
    float CombinedCardWidth = CardWidth*CardsX;
    float CombinedCardHeight = CardHeight*CardsY;
    float SpacingX = (0.7-CombinedCardWidth)/(CardsX-1);
    float SpacingY = (0.5-CombinedCardHeight)/(CardsY+1);
    
    for (i=0; i<CardsY; i++)
    {
        if (CardSlot/(CardsX*(i+1)) > 0) //GEm: Does not fit to this line!
            continue;
        
        CardSlot -= CardsX*i;
        Result.X = SpacingX*(CardSlot+1-1)+CardWidth*CardSlot+0.15;
        Result.Y = SpacingY*(i+1)+CardHeight*i+0.25;
        return Result;
    }
    printf("GetCardOnTableLocation: Error: Cards do not fit on the table!");
    return Result;
}

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

char *DrawDialog(int type,const char *fmt,...)
{
	SizeF Position;
    Size TextSize;
    SizeF RelativeTextSize;
    SizeF Resolution; Resolution.X = GetConfig(ResolutionX); Resolution.Y = GetConfig(ResolutionY);
	char *val;
	int vallen;
	va_list args;
	char *buf;
	int i,cnt=0;
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
	Position.X = 0.5-TextureCoordinates[type].X/Resolution.X/2.0;
    Position.Y = 0.5-TextureCoordinates[type].Y/Resolution.Y/2.0;
	DrawTexture(GfxData[type], TextureCoordinates[type], AbsoluteTextureSize(TextureCoordinates[type]), Position, GetDrawScale()*2);
	
	for (i=0;i<cnt;i++)
    {
		TTF_SizeText(Fonts[Font_Message], ptr[i], &(TextSize.X), &(TextSize.Y));
        Position.X = 0.5-TextSize.X/Resolution.X/2.0;
        Position.Y = 0.5-TextSize.Y/Resolution.Y*cnt/2.0+TextSize.Y/Resolution.Y*i;
        RelativeTextSize.X = TextSize.X/Resolution.X; RelativeTextSize.Y = TextSize.Y/Resolution.Y;
        DrawCustomTextCentred(ptr[i], Font_Message, Position, RelativeTextSize);
    }

	free(buf);

	if (type!=DLGNETWORK) return NULL;

	/*val[0]='_';val[1]=0;vallen=1;h=0; //GE: TODO: Input

	while (!h)
	{
		rect.x=160;
		rect.y=272;
		rect.w=320;
		rect.h=16;
		//SDL_FillRect(GfxData[SCREEN],&rect,0);
		i=BFont_TextWidth(val);*/
		/*if (i<312)
			BFont_PutString(GfxData[SCREEN],164,276,val);
		else
			BFont_PutString(GfxData[SCREEN],164+(312-i),276,val);
		SDL_UpdateRect(GfxData[SCREEN],160,272,320,16);*/ //FIXME 

		/*while (event.type!=SDL_KEYDOWN)
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
	if (h==2)*/
		return NULL;
	/*else
	{
		val[vallen-1]=0;
		return val;
	}*/
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
	    GeneralProtectionFault("File '%s' is missing or corrupt.",filename);
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
