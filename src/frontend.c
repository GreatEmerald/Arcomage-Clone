/**
 * The main file for the Arcomage Frontend.
 * This file initialises Arcomage graphics and input, then interfaces with
 * libarcomage.
 */  

//#include "input.h"
//#include "graphics.h"
//#include "sound.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "adapter.h"
#include "graphics.h"

/**
 * Utility function for printing out error messages.
 */

void FatalError(char *fmt,...)
{
    va_list args;
    va_start(args,fmt);
    printf(fmt);
    vfprintf(stderr,fmt,args);
    fprintf(stderr,"\n");
    va_end(args);
    exit(3);
}

/**
 * Game termination and memory cleanup.
 */ 
void Quit()
{
    Graphics_Quit();
    //Sound_Quit();
    rt_term(); //GE: Terminate D
}

/**
 * Main function and menu.
 *
 * Calls initialisation functions and performs main menu functions.
 *
 * Bugs: Should allow the player to input his name. Alternatively, use Lua.
 *
 * Authors: STiCK.
 */
int main(int argc,char *argv[])
{
    int MenuAction;
    ////srand((unsigned)time(NULL));

    rt_init(); //Init D
    InitArcomage(); //Init libarcomage
    //if (Config.SoundEnabled) //Init SDL
    //    Sound_Init();
    Graphics_Init();
    initGame(); //Init a 1vs1 game, will choose player types later
    DrawBackground();
    DrawLogo();
    UpdateScreen();

    MenuAction = Menu();//while ((m=Menu())!=4)//5)
    printf("Debug: main: MenuAction is %d\n", MenuAction);
    //{
    switch (MenuAction)//    switch (m)
    {
        case START:
            SetPlayerInfo(Turn, "Player", 0); //GE: Set up a player VS AI game.
            SetPlayerInfo(GetEnemy(), "AI", 1);//Player[GetEnemy()].AI = 1;
            
            ClearScreen();
            DrawBackground();
            DrawFoldedAlpha(0, 120.0/800.0, 166.0/600.0, (float)GetConfig(CardTranslucency)/255.0);
            DrawUI();
            UpdateScreen();
            getchar();
            //Player[Turn].Name = "Player";
            //Player[GetEnemy()].Name = "A.I.";
            //DoGame(); //Start the input loop
            //break;
    //    case 2:
    //        Player[Turn].Name  = "Player 1";
    //        Player[GetEnemy()].Name = "Player 2";
            //DoGame();
    //        break;
    //    case 3:
        /*    Network_Init();

            aiplayer=-1;
            DoNetwork();

            Network_Quit();
            break;
        case 4:*/
            //DoCredits();
    //        break;
    //    }
    default: break;
    }//}

    Quit();

    return 0;
}
