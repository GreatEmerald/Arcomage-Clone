/**
 * The main file for the Arcomage Frontend.
 * This file initialises Arcomage graphics and input, then interfaces with
 * libarcomage.
 */  

//#include "input.h"
//#include "graphics.h"
//#include "sound.h"
#include <stdio.h>
#include "adapter.h"

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
    //int m;
    ////srand((unsigned)time(NULL));

    rt_init(); //Init D
    InitArcomage(); //Init libarcomage
    //if (Config.SoundEnabled) //Init SDL
    //    Sound_Init();
    Graphics_Init(GetConfig(Fullscreen));
    initGame(); //Init a 1vs1 game, will choose player types later

    //while ((m=Menu())!=4)//5)
    //{
    //    switch (m)
    //    {
    //    case 1:
            SetPlayerInfo(Turn, "Player", 0);
            SetPlayerInfo(GetEnemy(), "AI", 1);//Player[GetEnemy()].AI = 1;
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
    //}

    Quit();

    return 0;
}
