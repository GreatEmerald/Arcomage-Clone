/**
 * The adapter-pattern-based wrapper for interfacing C to D.
 * This is a collection of global variable and C function declarations
 * needed to translate D types to C in order to abstract them. It interfaces
 * with libarcomage's wrapper.d file and the individual other D files. Use this
 * for reference about what parts of libarcomage you can hoop up to from C.
 */  

#ifndef __ADAPTER_H__
#define __ADAPTER_H__

int Turn;

enum ConfigTypes {
    Fullscreen=0,
    SoundEnabled,
    CardTranslucency,
    CardsInHand,
    TowerLevels,
    WallLevels,
    QuarryLevels,
    MagicLevels,
    DungeonLevels,
    BrickQuantities,
    GemQuantities,
    RecruitQuantities,
    MaxWall,
    TowerVictory,
    ResourceVictory,
    OneResourceVictory,
    UseOriginalCards,
    UseOriginalMenu,
    ResolutionX,
    ResolutionY
};

enum ColourTypes {
    CT_Red=0,
    CT_Blue,
    CT_Green,
    CT_White,
    CT_Black
};

void SetPlayerInfo(int PlayerNum, char* Name, char AI);
void SetSoundPlay(void (*SoundPlay) (int));
void SetRedrawScreen(void (*RedrawScreen) () );
void SetPlayCardAnimation(void (*PlayCardAnimation) (int, int) );
int GetConfig(int Type);
int GetColourType(char PlayerNum, char CardNum);

char* GetFilePath(char* FileName);
char*** GetCardDescriptionWords(int* NumSentences, int** NumWords);
void initGame();

#endif
