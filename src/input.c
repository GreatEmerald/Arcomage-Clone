#include <SDL.h>
#include "common.h"
#include "graphics.h"
#include "sound.h"
#include "config.h"
//#include "arco.h"
//extern int turn, aiplayer;

SDL_Event event;
int bRefreshNeeded=0; ///< True if we need to refresh the screen. Used in the input loop.

void WaitForKey(int sym)
{
	if (!sym)
	{
		do
			{
				SDL_PollEvent(&event);
				SDL_Delay(CPUWAIT);
			} while (event.type!=SDL_KEYUP);
	} else {
		do
		{
			SDL_PollEvent(&event);
			SDL_Delay(CPUWAIT);
		} while (event.type!=SDL_KEYDOWN || event.key.keysym.sym!=sym);
	}
}

void WaitForInput()
{
	/*
	do
	{
		SDL_PollEvent(&event);
		SDL_Delay(CPUWAIT);
	} while (!((event.type==SDL_KEYDOWN)||((event.type==SDL_MOUSEBUTTONDOWN)&&(event.button.button==SDL_BUTTON_LEFT))));

	if (event.type!=SDL_KEYDOWN)
	{
		do
		{
			SDL_PollEvent(&event);
			SDL_Delay(CPUWAIT);
		} while (event.type!=SDL_MOUSEBUTTONUP||event.button.button!=SDL_BUTTON_LEFT);
	}
	*/

	SDL_PumpEvents();
	do {
		SDL_PollEvent(&event);
		SDL_Delay(CPUWAIT);
	} while (!((event.type==SDL_KEYUP)||((event.type==SDL_MOUSEBUTTONUP)&&(event.button.button==SDL_BUTTON_LEFT))));
	SDL_PumpEvents();
}

/**
 * The main loop in the game.
 *
 * Includes the event loop, victory/loss handling, AI and network support.
 *
 * Authors: STiCK, GreatEmerald.
 */
void DoGame()
{
    int i;
    int crd,netcard,discrd;
    int quit=0;
    //GE: Stupid C not supporting string functions :(
    //GE: I correct myself: stupid C not supporting strings at all! Who designed this?!

    InitGame();
    // init screen
    Blit(GAMEBG,SCREEN);
    RedrawScreenFull();

    while (!quit && !Winner(0) && !Winner(1))
    {

        while(SDL_PollEvent(&event));//GE: Delete all events from the event queue before our turn.

        if (turn==aiplayer)
        {
            AIPlay(&i,&discrd);
            SDL_Delay(500);
            if (CanPlayCard(i,discrd))
			{
				PlayCardAnimation(i, discrd);
				PlayCard(i,discrd);
			}
        } else
        if (turn==netplayer)
        {
            if (NetRemPlay(&i,&discrd) && CanPlayCard(i,discrd))
			{
				PlayCardAnimation(i, discrd);
				PlayCard(i,discrd);
			}
            else {
                DialogBox(DLGERROR,"Server dropped connection ...");
                WaitForInput();
                return;
            }
        } else {
            while (!quit)
            {
                if (!SDL_PollEvent(&event))
                    continue;
                SDL_Delay(CPUWAIT);
                quit=(event.type==SDL_KEYUP&&event.key.keysym.sym==SDLK_ESCAPE);
                if (event.type==SDL_KEYDOWN&&event.key.keysym.sym==SDLK_b) //GE: Keeping as "down" since it's urgent ;)
                    Boss();
                if ( event.type == SDL_MOUSEMOTION && InRect(event.motion.x, event.motion.y,   8,342,  8+94,468) ) //GE: Support for highlighting cards, to be done: card tooltips.
                {
                    Blit(SCREEN, BUFFER);
                    //DrawRectangle(8,342,96,128, 0xFF0000);//OBSOLETE
                    UpdateScreen();
                    bRefreshNeeded=1;
                }
                else if(bRefreshNeeded)
                {
                    RedrawScreen(turn, Player);
                    bRefreshNeeded=0;
                }

                if (event.type!=SDL_MOUSEBUTTONUP || event.button.button>3) continue;
                discrd=(event.button.button==2)||(event.button.button==3);
                if (InRect(event.button.x,event.button.y,  8,342,  8+94,468)&&(discrd||Requisite(&Player[turn],0))) {crd=0;break;}
                if (InRect(event.button.x,event.button.y,114,342,114+94,468)&&(discrd||Requisite(&Player[turn],1))) {crd=1;break;}
                if (InRect(event.button.x,event.button.y,220,342,220+94,468)&&(discrd||Requisite(&Player[turn],2))) {crd=2;break;}
                if (InRect(event.button.x,event.button.y,326,342,326+94,468)&&(discrd||Requisite(&Player[turn],3))) {crd=3;break;}
                if (InRect(event.button.x,event.button.y,432,342,432+94,468)&&(discrd||Requisite(&Player[turn],4))) {crd=4;break;}
                if (InRect(event.button.x,event.button.y,535,342,532+94,468)&&(discrd||Requisite(&Player[turn],5))) {crd=5;break;}
            }
            if (!quit)
            {
                netcard = Player[turn].Hand[crd];
                if (CanPlayCard(i,discrd))
				{
					PlayCardAnimation(crd, discrd);
					PlayCard(crd,discrd);
				}
                
                if (netplayer!=-1)
                    NetLocPlay(crd,discrd,netcard);
            }
        }
        SDL_Delay(CPUWAIT);
    }
    if (!quit)
       SDL_Delay(1000);
    if (Winner(0) || Winner(1))
    {
        if (Winner(0)&&Winner(1))
        {
            DialogBox(DLGWINNER,"Draw!");
            Sound_Play(VICTORY);
        }
        else
        {
            if (aiplayer!=-1 || netplayer!=-1)              // 1 local Player
            {
                i=aiplayer;if (i==-1) i=netplayer;i=!i;
                if (Winner(i))
                {
                    if (Player[i].t>=TowerVictory)
                        DialogBox(DLGWINNER, "You win by a\ntower building victory!");
                    else if (Player[!i].t<=0)
                        DialogBox(DLGWINNER, "You win by a tower\ndestruction victory!");
                    else DialogBox(DLGWINNER, "You win by a\nresource victory!");
                    Sound_Play(VICTORY);
                }
                else
                {
                    if (Player[!i].t>=TowerVictory)
                        DialogBox(DLGLOOSER, "You lose by a\ntower building defeat!");
                    else if (Player[i].t<=0)
                        DialogBox(DLGLOOSER, "You lose by a\ntower destruction defeat!");
                    else DialogBox(DLGLOOSER, "You lose by a\nresource defeat!");
                    Sound_Play(DEFEAT);
                }
            } else {                                         // 2 local Players
                DialogBox(DLGWINNER,"Winner is\n%s !",Player[Winner(1)].Name);
                Sound_Play(VICTORY);
            }
        }
        WaitForInput();
    }
}
