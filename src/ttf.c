/**
 * Utility functions related to TrueType font rendering.
 * Authors: GreatEmerald, 2011
 */
 
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <SDL_ttf.h>
#include "graphics.h"
#include <SDL_opengl.h>
#include "opengl.h"
#include "adapter.h"
#include "ttf.h"

/*typedef struct S_OpenGLTexture
{
    GLuint Texture;
    Size TextureSize;
} OpenGLTexture;

struct S_CachedCard
{
    OpenGLTexture TitleTexture;
    OpenGLTexture PictureTexture;
    OpenGLTexture* DescriptionTextures;
    int DescriptionNum;
    OpenGLTexture PriceTexture[3]; //GE: Bricks, gems, recruits
};*/

TTF_Font* CurrentFont; //GE: The font that is currently loaded. Only one right now.
GLuint** FontCache; //GE: An array of textures for quick rendering. Must match the CardDB[][] in D!

/**
 * A shortened initialisation function for TTF.
 */
void InitTTF()
{
    if (TTF_Init()) //GE: Abusing the fact that -1 is considered true
        FatalError(TTF_GetError());
    
    CurrentFont = TTF_OpenFont(GetFilePath("fonts/FreeSans.ttf"), FindOptimalFontSize()); //GE: Make sure D is initialised first here.
    if (CurrentFont == NULL)
        FatalError(TTF_GetError());
}

void QuitTTF()
{
    TTF_CloseFont(CurrentFont);
    TTF_Quit();
}

/**
 * Render a single line. Do not use for formatted text!
 * Authors: C-Junkie, GreatEmerald
 */ 
void DrawTextLine(char* text, SizeF location)
{

	SDL_Surface *initial;
	SDL_Rect rect = {0, 0, 0, 0};
	int w,h;
	int texture;
    SDL_Color color = {0, 0, 0};
    Size TextureSize;

	/* Use SDL_TTF to render our text */
	initial = TTF_RenderText_Blended(CurrentFont, text, color);

	/* Tell GL about our new texture */
    texture = SurfaceToTexture(initial);
    
    TTF_SizeText(CurrentFont, text, &w, &h);
    rect.w = w; rect.h = h;
    TextureSize.X = w; TextureSize.Y= h;
    DrawTexture(texture, TextureSize, rect, location, 1.0);

	/* Clean up */
	SDL_FreeSurface(initial);
	glDeleteTextures(1, &texture);
}

/**
 * Finds the best font size for the current resolution and deck. May be
 * slow.
 * 
 * It works in this fashion: It requests a string array (the words in
 * the description of a card) from D, then counts the length of the
 * words in 90px (9*10). It then tests if the word fits into the given
 * rectangle. If it does, it checks if a space and another word fits.
 * Repeats as necessary until it no longer fits, then tries to see if
 * it fits in a new line. If it doesn't, it halves the font size and
 * tries again.
 */
int FindOptimalFontSize()
{
    time_t DeltaTime = time(NULL);
    Size CardSize = {(int)(GetDrawScale()*2*88), (int)(GetDrawScale()*2*53)}; //GE: Size in pixels indicating the draw area of a card.
    int NumSentences;
    int* NumWords;
    
    char*** Words = GetCardDescriptionWords(&NumSentences, &NumWords);
    int Sentence, Word;
    TTF_Font* ProbeFont;
    
    int SizeMin=1, SizeMax=90, OptimalSize=90, TestedSize; //GE: The minimum and maximum possible font sizes we have probed so far.
    int SpaceLength, ScaledSpaceLength, LineHeight, ScaledLineHeight;
    float SizeScale;
    int LineLength=0, TextHeight=0;
    int WordLength;
    
    ProbeFont = TTF_OpenFont(GetFilePath("fonts/FreeSans.ttf"), SizeMax); //GE: Probe with 9*10, because it gives us good enough resolution (we won't support over 8000x6000 anyway)
    TTF_SizeText(ProbeFont, " ", &SpaceLength, NULL);
    LineHeight = TTF_FontLineSkip(ProbeFont);
    
    for (Sentence = 0; Sentence < NumSentences; Sentence++)
    {
        while(SizeMax-SizeMin > 1)
        {
            LineLength = 0;
            TestedSize = (SizeMin+SizeMax)/2;
            SizeScale = TestedSize/90.0;
            
            ScaledSpaceLength = SpaceLength*SizeScale;
            ScaledLineHeight = LineHeight*SizeScale;
            TextHeight = ScaledLineHeight;
            for (Word = 0; Word < NumWords[Sentence]; Word++)
            {
                TTF_SizeText(ProbeFont, Words[Sentence][Word], &WordLength, NULL); //GE: Set the current word length.
                WordLength *= SizeScale;
                
                if ((LineLength == 0 && LineLength + WordLength > CardSize.X)
                    ||(LineLength > 0 && LineLength + ScaledSpaceLength + WordLength > CardSize.X)) //GE: This word won't fit.
                    {
                        if (WordLength > CardSize.X)//GE: A single word won't fit, automatic fail
                        {
                            SizeMax = TestedSize;
                            break;
                        }
                        TextHeight += ScaledLineHeight; //GE: Let's see if we can fit it on a new line.
                        LineLength = WordLength;
                    }
                else if (LineLength == 0)
                    LineLength += WordLength;
                else
                    LineLength += ScaledSpaceLength + WordLength;
            }
            if (TextHeight > CardSize.Y || SizeMax == TestedSize)
                SizeMax = TestedSize;
            else
                SizeMin = TestedSize;
        }
        OptimalSize = Min(OptimalSize, SizeMax);
        //printf("Debug: FindOptimalFontSize: Optimal size is now %d, on sentence %d.\n", OptimalSize, Sentence);
        SizeMin = 1;
    }
    
    TTF_CloseFont(ProbeFont);
    
    DeltaTime = time(NULL) - DeltaTime;
    printf("Info: FindOptimalFontSize: You can save %d seconds if you precache your font size as %d!\n", DeltaTime, OptimalSize);
    return OptimalSize;
}


/**
 * Utility functions for RenderText.
 * Authors: C-Junkie
 */ 
/*int round(double x)
{
	return (int)(x + 0.5);
}*/

/**
 * ditto
 */ 
int nextpoweroftwo(int x)
{
	double logbase2 = log(x) / log(2);
	return round(pow(2,ceil(logbase2)));
}

int Min(int A, int B)
{
    if (A <= B)
        return A;
    else
        return B;
}
