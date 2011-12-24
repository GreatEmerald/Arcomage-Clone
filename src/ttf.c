/**
 * Utility functions related to TrueType font rendering.
 * Authors: GreatEmerald, 2011
 */
 
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <SDL_ttf.h>
#include "graphics.h"
#include <SDL_opengl.h>
#include "opengl.h"
#include "adapter.h"
#include "ttf.h"

struct S_CardDescriptions
{
    char***** Text;
    int NumPools;
    int* NumSentences;
    int** NumLines;
    int*** NumWords;
};

TTF_Font* CurrentFont; //GE: The font that is currently loaded. Only one right now.
GLuint** FontCache; //GE: An array of textures for quick rendering. Must match the CardDB[][] in D!
struct S_CardDescriptions CardDescriptions;

/**
 * A shortened initialisation function for TTF.
 */
void InitTTF()
{
    int a, b, c, d;
    
    if (TTF_Init()) //GE: Abusing the fact that -1 is considered true
        FatalError(TTF_GetError());
    
    CardDescriptions.Text = GetCardDescriptionWords(&(CardDescriptions.NumPools), &(CardDescriptions.NumSentences), &(CardDescriptions.NumLines), &(CardDescriptions.NumWords));
    
    CurrentFont = TTF_OpenFont(GetFilePath("fonts/FreeSans.ttf"), FindOptimalFontSize()); //GE: Make sure D is initialised first here.
    if (CurrentFont == NULL)
        FatalError(TTF_GetError());
    
    //GE: <insert card precaching here>
    //GE: Deallocate the card descriptions, since we initialised them in this function as well.
    for (a=0; a<CardDescriptions.NumPools; a++)
    {
        for (b=0; b<CardDescriptions.NumSentences[a]; b++)
        {
            for (c=0; c<CardDescriptions.NumLines[a][b]; c++)
                free(CardDescriptions.Text[a][b][c]);
            free(CardDescriptions.Text[a][b]);
            free(CardDescriptions.NumWords[a][b]);
        }
        free(CardDescriptions.Text[a]);
        free(CardDescriptions.NumLines[a]);
        free(CardDescriptions.NumWords[a]);
    }
    free(CardDescriptions.Text);
    free(CardDescriptions.NumSentences);
    free(CardDescriptions.NumLines);
    free(CardDescriptions.NumWords);
    CardDescriptions.Text = NULL;
    CardDescriptions.NumSentences = NULL;
    CardDescriptions.NumLines = NULL;
    CardDescriptions.NumWords = NULL;
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
	GLuint texture;
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

void DrawTextCentred(char* Text, SizeF Destination, SizeF BoundingBox)
{
    int TextWidth;
    TTF_SizeText(CurrentFont, text, &TextWidth, NULL);
    Destination.X += (Destination.X - TextWidth/(float)GetConfig(ResolutionX))/2.0
    DrawTextLine(Text, Destination);
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
    /*time_t DeltaTime = time(NULL);
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
    return OptimalSize;*/
    return 10;
}

void PrecacheFonts()
{
    PrecacheTitleText();
    PrecacheDescriptionText();
    PrecachePriceText();
}

void PrecacheDescriptionText()
{
    Size CardSize = {(int)(GetDrawScale()*2*88), (int)(GetDrawScale()*2*53)};
    int Pool, Sentence, Line, Word;
    int SpaceLength;
    int LineHeight; TTF_SizeText(CurrentFont, " ", &SpaceLength, &LineHeight);
    int WordLength, LineLength;
    int LastLineEnd;
    int i;
    int TextSize;
    char* CurrentLine;
    GLuint CurrentTexture;
    Size TextureSize;
    
    for (Pool = 0; Pool < CardDescriptions.NumPools; Pool++)
    {
        for (Sentence = 0; Sentence < CardDescriptions.NumSentences; Sentence++)
        {
            for (Line = 0; Sentence < CardDescriptions.NumLines; Line++)
            {
                LineLength = 0;
                LastLineEnd = 0;
                
                for (Word = 0; Word < NumWords[Sentence]; Word++)
                {
                    TTF_SizeText(CurrentFont, CardDescriptions.Text[Pool][Sentence][Line][Word], &WordLength, NULL); //GE: Set the current word length.
                    
                    if ((LineLength == 0 && LineLength + WordLength > CardSize.X)
                    ||(LineLength > 0 && LineLength + SpaceLength + WordLength > CardSize.X)) //GE: Next word won't fit.
                    {
                        //GE: This line is full, write to cache.
                        for (i=0; i < Word-LastLineEnd; i++)
                            TextSize += strlen(CardDescriptions.Text[Pool][Sentence][Line][i]);
                        CurrentLine = (char*) malloc((TextSize+1)*sizeof(char));
                        strcpy(CurrentLine, CardDescriptions.Text[Pool][Sentence][Line][0]);
                        for (i=1; i < Word-LastLineEnd; i++)
                        {
                            strcat(CurrentLine, " ");
                            strcat(CurrentLine, CardDescriptions.Text[Pool][Sentence][Line][i]);
                        }
                        CurrentTexture = TextToTexture(CurrentFont, CurrentLine);
                        TTF_SizeText(CurrentFont, CurrentLine, &(TextureSize.X), &(TextureSize.Y));
                        free(CurrentLine);
                        
                        CardCache[Pool][Sentence].DescriptionNum++;
                        CardCache[Pool][Sentence].DescriptionTextures = (OpenGLTexture*)realloc(CardCache[Pool][Sentence].DescriptionTextures, CardCache[Pool][Sentence].DescriptionNum*sizeof(OpenGLTexture));
                        //GE: Remember to free this! Also, using the fact that when the pointer to realloc is NULL (and we set it that way when we malloced CardCache), realloc redirects to malloc.
                        CardCache[Pool][Sentence].DescriptionTextures[CardCache[Pool][Sentence].DescriptionNum-1].Texture = CurrentTexture;
                        CardCache[Pool][Sentence].DescriptionTextures[CardCache[Pool][Sentence].DescriptionNum-1].TextureSize = TextureSize;
                            
                        LastLineEnd = Word;
                        LineLength = WordLength;
                    }
                    else if (LineLength == 0)
                        LineLength += WordLength;
                    else
                        LineLength += SpaceLength + WordLength;
                }
            }
        }
    }
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

/**
 * Concatenate multiple strings.
 * This must be called with a (char*)NULL as the last argument so that
 * it would know when to stop, and the resulting string must be freed
 * afterwards.
 * Authors: comp.lang.c FAQ list
 */ 
char *vstrcat(const char *first, ...)
{
	size_t len;
	char *retbuf;
	va_list argp;
	char *p;

	if(first == NULL)
		return NULL;

	len = strlen(first);

	va_start(argp, first);

	while((p = va_arg(argp, char *)) != NULL)
		len += strlen(p);

	va_end(argp);

	retbuf = malloc(len + 1);	/* +1 for trailing \0 */

	if(retbuf == NULL)
		return NULL;		/* error */

	(void)strcpy(retbuf, first);

	va_start(argp, first);		/* restart; 2nd scan */

	while((p = va_arg(argp, char *)) != NULL)
		(void)strcat(retbuf, p);

	va_end(argp);

	return retbuf;
}
