#ifndef _TTF_H_
#define _TTF_H_

#include <SDL_ttf.h>

enum
{
    Font_Title = 0,
    Font_Description,
    Font_Name, //GEm: This is the font for the player's name.
    Font_Count
};

enum
{
    Numbers_Big = 0,
    Numbers_Medium,
    Numbers_Small,
    Numbers_Count
};

OpenGLTexture NumberCache[Numbers_Count][10];
OpenGLTexture NameCache[2]; //GEm: FIXME - needs to be dynamic

void InitTTF();
void QuitTTF();
void DrawTextLine(char* text, SizeF location);
int FindOptimalFontSize();
void PrecacheFonts();
void PrecacheTitleText();
void PrecacheDescriptionText();
void PrecachePriceText();
void PrecacheNumbers();
void PrecachePlayerNames();
//int round(double x);
int nextpoweroftwo(int x);
int Min(int A, int B);
GLuint TextToTextureColour(TTF_Font* Font, char* Text, SDL_Color Colour);
GLuint TextToTexture(TTF_Font* Font, char* Text);
SizeF CentreOnX(SizeF Destination, SizeF ObjectSize, SizeF BoundingBox);

#endif
