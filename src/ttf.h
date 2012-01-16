#ifndef _TTF_H_
#define _TTF_H_

#include <SDL_ttf.h>

enum
{
    Font_Title = 0,
    Font_Description,
    Font_Count
};

enum
{
    Numbers_Big = 0,
    Numbers_Medium,
    Numbers_Small,
    Numbers_Count
};

void InitTTF();
void QuitTTF();
void DrawTextLine(char* text, SizeF location);
int FindOptimalFontSize();
void PrecacheFonts();
void PrecacheTitleText();
void PrecacheDescriptionText();
void PrecachePriceText();
void PrecacheNumbers();
//int round(double x);
int nextpoweroftwo(int x);
int Min(int A, int B);
GLuint TextToTexture(TTF_Font* Font, char* Text);
SizeF CentreOnX(SizeF Destination, SizeF ObjectSize, SizeF BoundingBox);

#endif
