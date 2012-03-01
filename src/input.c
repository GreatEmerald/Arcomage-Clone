#include <SDL.h>
#include "graphics.h"
#include "adapter.h"
//#include "sound.h"

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
    int i, n;
    int crd,netcard;
    char bDiscarded=0;

    while (!IsVictorious(0) && !IsVictorious(1))
    {
        DrawStaticScene();
        DrawAllPlayerCards();
        UpdateScreen();

        while (SDL_PollEvent(&event));//GE: Delete all events from the event queue before our turn.

        if (GetIsAI(Turn))
        {
            SDL_Delay(500);
            AIPlay();
        } /*else //GEm: TODO Netplay
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
        } */
        else
        {
            if (!SDL_PollEvent(&event))
                continue;
            SDL_Delay(0); //GEm: HACK
            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE) //GEm: Return if Esc is pressed.
                return;
            /*if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_b) //GE: Keeping as "down" since it's urgent ;)
                Boss();*/ //GEm: TODO boss screen
            /*if ( event.type == SDL_MOUSEMOTION && InRect(event.motion.x, event.motion.y,   8,342,  8+94,468) ) //GE: Support for highlighting cards, to be done: card tooltips.
            {
                Blit(SCREEN, BUFFER);
                UpdateScreen();
                bRefreshNeeded=1;
            }
            else if(bRefreshNeeded)
            {
                RedrawScreen(turn, Player);
                bRefreshNeeded=0;
            }*/ //GEm: TODO: Card highlighting

            if (event.type != SDL_MOUSEBUTTONUP || event.button.button > 3)
                continue;
            bDiscarded = (event.button.button == 2) || (event.button.button == 3);
            for (i=0; i<GetConfig(CardsInHand); i++)
            {
                if (InRect(event.button.x,event.button.y, CardLocations[Turn][i].X, CardLocations[Turn][i].Y,
                    (CardLocations[Turn][i].X+94)/600.0, (CardLocations[Turn][i].Y+128)/600.0)
                    &&  (bDiscarded || GetCanPlayCard(Turn, i, bDiscarded)))
                {
                    crd=i;
                    break;
                }
            }
            //netcard = Player[turn].Hand[crd];//GEm: TODO: Netplay
            PlayCard(crd, bDiscarded);
            
            /*if (netplayer!=-1)
                NetLocPlay(crd,discrd,netcard);*/ //GEm: TODO: Netplay
        }
        SDL_Delay(CPUWAIT);
    }
    
    SDL_Delay(1000);
    
    /*if (IsVictorious(0) && IsVictorious(1)) //GEm: TODO: Message boxes and sound
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
    }*/
    WaitForInput();
}
