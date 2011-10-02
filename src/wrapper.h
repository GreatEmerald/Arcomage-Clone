/**
 * The adapter-pattern-based wrapper for interfacing C to D.
 * This is a collection of extern global variable and C function declarations
 * needed to translate D types to C in order to abstract them. It interfaces
 * with libarcomage's wrapper.d file and the individual other D files. Use this
 * for reference about what parts of libarcomage you can hoop up to from C.
 */  

#ifndef __DWRAPPER__
#define __DWRAPPER__

extern int Turn;
extern int Test;
extern int Toot;

void SetPlayerInfo(int PlayerNum, char* Name, char AI);
void initGame();
#endif
