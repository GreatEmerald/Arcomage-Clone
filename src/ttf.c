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
#include "adapter.h"
#include "ttf.h"

TTF_Font* CurrentFont; //GE: The font that is currently loaded. Only one right now.
GLuint** PrecachedDescriptions; //GE: An array of textures for quick rendering. Must match the CardDB[][] in D!

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

/**
 * Render a single line. Do not use for formatted text!
 * Authors: C-Junkie, GreatEmerald
 */ 
void RenderLine(char* text, SizeF location)
{

	SDL_Surface *initial;
	SDL_Surface *intermediary;
	SDL_Rect rect = {0, 0, 0, 0};
	int w,h;
	int texture;
    SDL_Color color = {0, 0, 0};
    Size TextureSize;

	/* Use SDL_TTF to render our text */
	initial = TTF_RenderText_Blended(CurrentFont, text, color);

	/* Convert the rendered text to a known format */
    //GE: Probably not necessary when we have support for non-power of 2 textures.
	w = nextpoweroftwo(initial->w);
	h = nextpoweroftwo(initial->h);

	intermediary = SDL_CreateRGBSurface(0, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

	SDL_BlitSurface(initial, 0, intermediary, 0);

	/* Tell GL about our new texture */
    texture = SurfaceToTexture(intermediary);
    
    TTF_SizeText(CurrentFont, text, &w, &h); //GE: Reusing w and h, since they are expendable at this point
    rect.w = w; rect.h = h;
    TextureSize.X = w; TextureSize.Y= h;
    DrawTexture(texture, TextureSize, rect, location, 1.0);

	/* Clean up */
	SDL_FreeSurface(initial);
	SDL_FreeSurface(intermediary);
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
    Size CardSize = {(int)(GetDrawScale()*0.5*88), (int)(GetDrawScale()*0.5*53)}; //GE: Size in pixels indicating the draw area of a card.
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
    //char Fits=1;
    clock_t DeltaTime = clock();
    
    ProbeFont = TTF_OpenFont(GetFilePath("fonts/FreeSans.ttf"), SizeMax); //GE: Probe with 9*10, because it gives us good enough resolution (we won't support over 8000x6000 anyway)
    TTF_SizeText(ProbeFont, " ", &SpaceLength, NULL);
    LineHeight = TTF_FontLineSkip(ProbeFont);
    
    for (Sentence = 0; Sentence < NumSentences; Sentence++)
    {
        while(SizeMax-SizeMin > 1)
        {
            TestedSize = (SizeMin+SizeMax)/2;
            SizeScale = TestedSize/90.0;
            
            TextHeight = LineHeight;
            for (Word = 0; Word < NumWords[Sentence]; Word++)
            {
                printf("Debug: FindOptimalFontSize: Iterating through %s, total sentences %d and words %d, iteration %d.\n", Words[Sentence][Word], NumSentences, NumWords[Sentence], Word);
                TTF_SizeText(ProbeFont, Words[Sentence][Word], &WordLength, NULL); //GE: Set the current word length.
                WordLength *= SizeScale;
                ScaledSpaceLength = SpaceLength*SizeScale;
                ScaledLineHeight = LineHeight*SizeScale;
                if ((LineLength == 0 && LineLength + WordLength > CardSize.X)
                    ||(LineLength > 0 && LineLength + ScaledSpaceLength + WordLength > CardSize.X)) //GE: This word won't fit.
                    {
                        if (WordLength > CardSize.X)//GE: A single word won't fit, automatic fail
                        {
                            SizeMin = TestedSize;
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
            if (TextHeight > CardSize.Y || SizeMin == TestedSize)
            {
                //Fits = 0;
                SizeMin = TestedSize;
            }
            else
            {
                //Fits = 1;
                SizeMax = TestedSize;
            }
        }
        OptimalSize = Min(OptimalSize, SizeMax);
    }
    
    TTF_CloseFont(ProbeFont);
    
    DeltaTime = clock() - DeltaTime;
    printf("Info: FindOptimalFontSize: You can save %d seconds if you precache your font size as %d!\n", DeltaTime*CLOCKS_PER_SEC, OptimalSize);
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
