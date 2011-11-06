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

/*
 * GE: Lua reform: We want the modder in Lua to give the picture and coordinates
 * and parse those to load the required surfaces in C. Then when sending to D,
 * we want to send the things already parsed - instead of a string and four
 * doubles, we want a pointer and four ints/struct of coords. When we attempt
 * to draw a card, we pass the card number and pool, from where C code asks D
 * for the pointer and coords, which it then uses to display the thing.     
 */
//GE: c - card ID, x,y - position on the screen.
/*void DrawCard(int c,int x,int y, Uint8 a) //DEBUG
{
	SDL_Rect recta,rectb;
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
	}
	else
	{*/
    	/*
    	 * GE: Transform.
    	 * Got 1*96, 1*128
    	 * Got 11*96, 1*128 -> 1*96, 2*128
    	 * Got 1*96, 2*128 -> 1*96, 5*128
    	 * Three special cards: 0,0 is red, 0,1 is blue, 0,3 is Discard.
    	 * Original doesn't have blue, and Discard is not in a shape of a card.             
    	 */
    	
/*      RawX=(c&0xFF); //DEBUG
    	RawY=(c>>8);
    	
    	//GE: Start special case handling --------------------------------------------
    	if (RawX == 0)
    	{
          if (RawY == 0 || RawY == 1) //GE: back of the card
          {
              rectb.x=192;rectb.y=0;rectb.w=96;rectb.h=128;
              recta.x=x;recta.y=y;
          }
          else //GE: Discard
          {
              rectb.x=843;rectb.y=200;rectb.w=73;rectb.h=16;
              SDL_SetColorKey(GfxData[DECK], SDL_SRCCOLORKEY, 0x000000FF);
              recta.x=x+11;recta.y=y+56;
          }
      }
      else //GE: End special case handling -----------------------------------------
      {
        	RawX-=1;
          RawY=RawX/10+(RawY*4);
          RawX=RawX%10;
        	
        	recta.x=x;recta.y=y;recta.w=96;recta.h=128;
          rectb.x=RawX*96;rectb.y=RawY*128+220;rectb.w=96;rectb.h=128;
      }
  }
	SDL_BlitSurface(GfxData[DECK],&rectb,GfxData[SCREEN],&recta);
}

void DrawFolded(int Team, int X, int Y)
{
    SDL_Rect ScreenPosition, DeckPosition;
    
    ScreenPosition.x = X; ScreenPosition.y = Y;
    ScreenPosition.w = 96; ScreenPosition.h = 128;
    
    if (!bUseOriginalCards) //GE: New
    {
        DeckPosition.x = 0;
        if (Team) //GE: Blue
            DeckPosition.y = 128;
        else //GE: Red
            DeckPosition.y = 0;
    }
    else //GE: Original
        DeckPosition.x = 192; DeckPosition.y = 0;
    DeckPosition.w = ScreenPosition.w; DeckPosition.h = ScreenPosition.h;
    
    SDL_BlitSurface(GfxData[DECK],&DeckPosition,GfxData[SCREEN],&ScreenPosition);
}*/

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

void DrawSmallNumber(int Resource, int X, int Y, int Offset)
{
    char str[4];
    int i;
    sprintf(str, "%d", Resource);
    for (i=0; str[i]!=0; i++)
        str[i]=str[i]-'0'+Offset;
    //BFont_PutStringFont(GfxData[SCREEN],numssmall,X,Y,str); //DEBUG
}

void DrawBigNumber(int Resource, int X, int Y)
{
    SDL_Rect recta,rectb;
    int d1, d2;
    d1 = Resource/10;
    d2 = Resource%10;
    
    recta.w=22;recta.h=17;
    recta.x=X;
    recta.y=Y;
    
    rectb.y=0;
    rectb.w=22;rectb.h=17;
    
    if (d1)
    {
       rectb.x=22*d1;
       //SDL_BlitSurface(GfxData[NUMSBIG],&rectb,GfxData[SCREEN],&recta);//DEBUG
       recta.x += 22;
    }
    rectb.x=22*d2;
    //SDL_BlitSurface(GfxData[NUMSBIG],&rectb,GfxData[SCREEN],&recta);//DEBUG
}

/*void DrawStatus(int turn,struct Stats *Players) //DEBUG
{
	SDL_Rect recta,rectb;
	int i,j;
	char b[4];
//	print players' names
	recta.w=70;recta.h=16;recta.y=13+4;
	if (turn) recta.x=555+4;
		else recta.x=8+4;
	SDL_FillRect(GfxData[SCREEN],&recta,SDL_MapRGB(GfxData[SCREEN]->format,(Uint8)(0xC0*!turn),0,(Uint8)(0xC0*turn)));
	BFont_PutString(GfxData[SCREEN], 15+32-BFont_TextWidth(Players[0].Name)/2,20,Players[0].Name);
	BFont_PutString(GfxData[SCREEN],562+32-BFont_TextWidth(Players[1].Name)/2,20,Players[1].Name);
	int Offset[3]={33,43,53};
    for (i=0;i<2;i++)
	{
	    for (j=0;j<3;j++)
	    {
            DrawSmallNumber((&Players[i].b)[j], 10+i*547, 115+j*72, Offset[j]);
            DrawBigNumber((&Players[i].q)[j], 13+547*i, 91+j*72);
        }*/
        
        
        /*int d1, d2;
        d1 = Players[i].q/10;
        d2 = Players[i].q%10;
        
        j=0;
        recta.w=22;recta.h=17;
	    recta.x=13+547*i;recta.y=91+j*72;
        rectb.y=0;
        rectb.w=22;rectb.h=17;
        
        if (d1)
        {
           rectb.x=22*d1;
           SDL_BlitSurface(GfxData[NUMSBIG],&rectb,GfxData[SCREEN],&recta);
           recta.x += 22;
        }
        rectb.x=22*d2;
        SDL_BlitSurface(GfxData[NUMSBIG],&rectb,GfxData[SCREEN],&recta);*/
    	
        //for (j=0;j<3;j++)
		//{
            
            /*recta.w=22;recta.h=17;
			recta.x=13+547*i;recta.y=91+j*72;
			if (j==0) rectb.x=22*Players[i].q;
			if (j==1) rectb.x=22*Players[i].m;
			if (j==2) rectb.x=22*Players[i].d;
            rectb.y=0;
			rectb.w=22;rectb.h=17;
		//	print big (yellow) numbers (quarry,magic,dungeons)
			SDL_BlitSurface(GfxData[NUMSBIG],&rectb,GfxData[SCREEN],&recta);*/
			/*if (j==0) {sprintf(b,"%d",Players[i].b);b[0]-='0'-'!';if (b[1]) b[1]-='0'-'!';if (b[2]) b[2]-='0'-'!';}
			if (j==1) {sprintf(b,"%d",Players[i].g);b[0]-='0'-'+';if (b[1]) b[1]-='0'-'+';if (b[2]) b[2]-='0'-'+';}
			if (j==2) {sprintf(b,"%d",Players[i].r);b[0]-='0'-'5';if (b[1]) b[1]-='0'-'5';if (b[2]) b[2]-='0'-'5';}*/
			//sprintf(b,"%d",Players[i].b);
			
            //b[0]='!'+1; b[1]='!'+5; b[2]='!'+15; b[3]=0;
            //b[0]-='0'-1; b[1]-='0'-1; b[2]-='0'-1; b[3]=0;
		//	print small numbers (bricks,gems,recruits)
			//BFont_PutStringFont(GfxData[SCREEN],numssmall,10+i*547,115+j*72,b);
		//}
    /*} //DEBUG
//	print tower/wall numbers
	sprintf(b,"%d",Players[0].t);BFont_PutString(GfxData[SCREEN],160-BFont_TextWidth(b)/2,317,b);
	sprintf(b,"%d",Players[0].w);BFont_PutString(GfxData[SCREEN],242-BFont_TextWidth(b)/2,317,b);
	sprintf(b,"%d",Players[1].w);BFont_PutString(GfxData[SCREEN],398-BFont_TextWidth(b)/2,317,b);
	sprintf(b,"%d",Players[1].t);BFont_PutString(GfxData[SCREEN],resY-BFont_TextWidth(b)/2,317,b);
//	draw left tower
	rectb.x=0;rectb.y=0;rectb.w=68;rectb.h=292-(200-Players[0].t);
	recta.x=126;recta.y=16+(200-Players[0].t);recta.w=68;recta.h=292-(200-Players[0].t);
	SDL_BlitSurface(GfxData[CASTLE],&rectb,GfxData[SCREEN],&recta);
//	draw right tower
	rectb.x=68;rectb.y=0;rectb.w=68;rectb.h=292-(200-Players[1].t);
	recta.x=446;recta.y=16+(200-Players[1].t);recta.w=68;recta.h=292-(200-Players[1].t);
	SDL_BlitSurface(GfxData[CASTLE],&rectb,GfxData[SCREEN],&recta);
//	draw left wall
	rectb.x=68*2;rectb.y=0;rectb.w=68;rectb.h=292-(200-Players[0].w);
	recta.x=230;recta.y=16+(200-Players[0].w);recta.w=68;recta.h=292-(200-Players[0].w);
	SDL_BlitSurface(GfxData[CASTLE],&rectb,GfxData[SCREEN],&recta);
//	draw right wall
	rectb.x=68*2;rectb.y=0;rectb.w=68;rectb.h=292-(200-Players[1].w);
	recta.x=386;recta.y=16+(200-Players[1].w);recta.w=68;recta.h=292-(200-Players[1].w);
	SDL_BlitSurface(GfxData[CASTLE],&rectb,GfxData[SCREEN],&recta);
}*/

//GE: Draw menu buttons.
void DrawMenuItem(int Type, char Lit)
{
    SDL_Rect SourceCoords = {0,0,0,0}; //GE: Make sure they are initialised.
    SizeF DestinationCoords = {0.0,0.0};
    float ResX = (float)GetConfig(ResolutionX);
    float ResY = (float)GetConfig(ResolutionY);
    float DrawScale = FMin((float)ResX/1600.0, (float)ResY/1200.0);
    
    printf("Info: DrawMenuItem: Lit is %d\n", (int)Lit);
    
    if (Type < 3)
    {
	SourceCoords.x=0+250*Lit; SourceCoords.y=108*Type; SourceCoords.w=250; SourceCoords.h=108;
	DestinationCoords.X = ((2.0*Type+1.0)/6.0)-(((float)(SourceCoords.w*DrawScale)/ResX)/2.0); DestinationCoords.Y = ((130.0/600.0)-((float)(SourceCoords.h*DrawScale)/600.0))/2.0;
	printf("Debug: DrawMenuItem: DestinationCoords.Y is %f\n", DestinationCoords.Y);
    }
    else
    {
	SourceCoords.x=250*2+250*Lit; SourceCoords.y=108*Type; SourceCoords.w=250; SourceCoords.h=108;
	DestinationCoords.X = ((2.0*(Type-3)+1.0)/6.0)-(((float)(SourceCoords.w*DrawScale)/ResX)/2.0); DestinationCoords.Y = ((600.0-130.0/2.0)-(float)(SourceCoords.h*DrawScale)/2.0)/600.0;
	printf("Debug: DrawMenuItem: LOWER BUTTONS: DestinationCoords.Y is %f\n", DestinationCoords.Y);
    }
    DrawTexture(GfxData[SPRITES], TextureCoordinates[SPRITES], SourceCoords, DestinationCoords, DrawScale);
}

int Menu()
{
	int i,/*j,*/value=0;
	float ResX = (float)GetConfig(ResolutionX);
	float ResY = (float)GetConfig(ResolutionY);
	float DrawScale = FMin((float)ResX/1600.0, (float)ResY/1200.0);
	int LitButton = -1; //GE: Which button is lit.

	for (i=0;i<6;i++)
		DrawMenuItem(i,0);
	
	UpdateScreen();
	
	//Sound_Play(TITLE);

	while (!value)
	{
		SDL_PollEvent(&event);
		switch (event.type)
		{
            case SDL_QUIT:
                value=Quit;
                break;
            case SDL_MOUSEMOTION:
                for (i=0; i<6; i++)
                {
                    if ( (i < 3
                    && FInRect(event.motion.x/ResX, event.motion.y/ResY,
                    (2.0*i+1.0)/6.0-(250.0*DrawScale/ResX/2.0),
                    (2.0*i+1.0)/6.0+(250.0*DrawScale/ResX/2.0),
                    ((130.0/600.0)-(108.0*DrawScale/600.0))/2.0,
                    ((130.0/600.0)+(108.0*DrawScale/600.0))/2.0))
                    /*&& (float)event.motion.x/ResX >= (2.0*i+1.0)/6.0-(250.0*DrawScale/ResX/2.0) //GE: These correspond to entries in DrawMenuItem().
                    && (float)event.motion.x/ResX <= (2.0*i+1.0)/6.0+(250.0*DrawScale/ResX/2.0)
                    && (float)event.motion.y/ResY >= ((130.0/600.0)-(108.0*DrawScale/600.0))/2.0
                    && (float)event.motion.y/ResY <= ((130.0/600.0)+(108.0*DrawScale/600.0))/2.0)*/
                    || (i >= 3
                    && (float)event.motion.x/ResX >= (2.0*(i-3.0)+1.0)/6.0-(250.0*DrawScale/ResX/2.0)
                    && (float)event.motion.x/ResX <= (2.0*(i-3.0)+1.0)/6.0+(250.0*DrawScale/ResX/2.0)
                    && (float)event.motion.y/ResY >= ((600.0-130.0/2.0)-(108.0*DrawScale/2.0))/600.0
                    && (float)event.motion.y/ResY <= ((600.0-130.0/2.0)+(108.0*DrawScale/2.0))/600.0)
                    )
                    {
                    if (LitButton < 0) //GE: We are on a button, and there are no lit buttons. Light the current one.
                    {
                        printf("Debug: Menu: We are checking the rect starting with X %f and ending with X %f\n", (2.0*(i-3.0)+1.0)/6.0-(250.0*DrawScale/ResX/2.0), (2.0*(i-3.0)+1.0)/6.0+(250.0*DrawScale/ResX/2.0));
                        DrawMenuItem(i, 1);
                        UpdateScreen();
                        LitButton = i;
                    }
                    }
                    else if (LitButton == i) //GE: We are not on the current button, yet it is lit.
                    {
                    DrawMenuItem(i, 0);
                    UpdateScreen();
                    LitButton = -1;
                    }
                }
                break;
            case SDL_MOUSEBUTTONUP:
                //if ((event.button.button==SDL_BUTTON_LEFT) && InRect(event.button.x, event.button.y,buttonDistanceX,buttonDistanceY,buttonDistanceX+buttonWidth,buttonDistanceY+buttonHeight*buttonNum))
                //{	// menuitem
                    //j=(event.button.y-buttonDistanceY)/buttonHeight;
                    //value=j+1;
                //}
                break;
		}
		SDL_Delay(0);//CPUWAIT); //GE: FIXME: This is not the same between platforms and causes major lag in Linux.
	}
	return value;
}

void DrawGUIElements()
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
    
    UpdateScreen();
}

////////////////////////////////////////////////////////////////////////////////

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
		"Arcomage vAlpha" /*ARCOVER,*/
		"by STiCK and GreatEmerald (2005-2009)",
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
