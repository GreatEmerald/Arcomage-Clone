/**
 * The main file for the Arcomage Frontend.
 * This file initialises Arcomage graphics and input, then interfaces with
 * libarcomage.
 */  

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
    int m;
    srand((unsigned)time(NULL));

    ParseArgs(argc,argv);

    Init();

    while ((m=Menu())!=5)
    {
        switch (m)
        {
        case 1:
            aiplayer=1;
            Player[0].Name = "Player";
            Player[1].Name = "A.I.";
            DoGame();
            break;
        case 2:
            aiplayer=-1;
            Player[0].Name = "Player 1";
            Player[1].Name = "Player 2";
            DoGame();
            break;
        case 3:
            Network_Init();

            aiplayer=-1;
            DoNetwork();

            Network_Quit();
            break;
        case 4:
            DoCredits();
            break;
        }
    }

    Quit();

    return 0;
}
