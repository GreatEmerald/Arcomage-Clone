/**
 * The adapter-pattern-based wrapper for interfacing C to D.
 * This is a collection of extern global variable declarations and C functions
 * needed to translate D types to C in order to abstract them. It interfaces
 * with libarcomage's wrapper.d file and the individual other D files.
 */  

struct CardInfo {
    char* Name; //string in D
    char* Description; //string in D
    int Frequency;
    int BrickCost;
    int GemCost;
    int RecruitCost;
    char Cursed; //bool in D
    char* Colour; //string in D
    struct PictureInfo Picture;
    char* Keywords; //string in D
    int (PlayFunction) (void); //int delegate() in D
    float (AIFunction) (void); //float delegate() in D
    CardInfo* (fromFile) (char*); //static CardInfo[] fromFile(in char[]) in D
};
struct Stats
{
    int Bricks, Gems, Recruits;
    int Quarry, Magic, Dungeon;
    int Tower, Wall;
    char* Name; //string in D
    char AI; //bool in D
    (struct CardInfo)* Hand; //Cardinfo[] in D
};
extern (struct Stats)* Player; //Stats[] in D
extern int Turn;
