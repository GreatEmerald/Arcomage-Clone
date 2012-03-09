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
    
    Fonts[Font_Description] = TTF_OpenFont(GetFilePath("fonts/FreeSans.ttf"), FindOptimalFontSize()); //GE: Make sure D is initialised first here.
    Fonts[Font_Message] = TTF_OpenFont(GetFilePath("fonts/FreeSansBold.ttf"), (int)(GetDrawScale()*2*20));
    Fonts[Font_Title] = TTF_OpenFont(GetFilePath("fonts/FreeSans.ttf"), (int)(GetDrawScale()*2*10));
    Fonts[Font_Name] = TTF_OpenFont(GetFilePath("fonts/FreeMono.ttf"), (int)(GetDrawScale()*2*11));//7
    if (Fonts[Font_Description] == NULL)
        FatalError(TTF_GetError());
    
    NumberFonts[Numbers_Big] = TTF_OpenFont(GetFilePath("fonts/FreeMonoBold.ttf"), (int)(GetDrawScale()*2*27));//17
    NumberFonts[Numbers_Medium] = TTF_OpenFont(GetFilePath("fonts/FreeMonoBold.ttf"), (int)(GetDrawScale()*2*16));//10
    NumberFonts[Numbers_Small] = TTF_OpenFont(GetFilePath("fonts/FreeMono.ttf"), (int)(GetDrawScale()*2*11));//7
    
    PrecacheCards();
    
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
    int i;
    
    for (i=0; i<Font_Count; i++)
        TTF_CloseFont(Fonts[i]);
    for (i=0; i<Numbers_Count; i++)
        TTF_CloseFont(NumberFonts[i]);
    TTF_Quit();
}

/**
 * Render a single line. Do not use for formatted text!
 * Authors: C-Junkie, GreatEmerald
 */ 
void DrawTextLine(char* text, SizeF location)
{
	SDL_Rect rect = {0, 0, 0, 0};
	int w,h;
	GLuint texture;
    Size TextureSize;

    texture = TextToTexture(Fonts[Font_Description], text);
    
    TTF_SizeText(Fonts[Font_Description], text, &w, &h);
    rect.w = w; rect.h = h;
    TextureSize.X = w; TextureSize.Y= h;
    DrawTexture(texture, TextureSize, rect, location, 1.0);

	/* Clean up */
	glDeleteTextures(1, &texture);
}

void DrawCustomTextCentred(char* text, int FontType, SizeF BoxLocation, SizeF BoxSize)
{
	SDL_Rect rect = {0, 0, 0, 0};
	int w,h;
	GLuint texture;
    Size TextureSize;
    SizeF RelativeSize;

    SDL_Color Colour = {255, 255, 255};
    texture = TextToTextureColour(Fonts[FontType], text, Colour);
    
    TTF_SizeText(Fonts[Font_Description], text, &w, &h);
    rect.w = w; rect.h = h;
    TextureSize.X = w; TextureSize.Y= h;
    RelativeSize.X = w/(float)GetConfig(ResolutionX); RelativeSize.Y = h/(float)GetConfig(ResolutionY);
    BoxLocation = CentreOnX(BoxLocation, RelativeSize, BoxSize);
    
    DrawTexture(texture, TextureSize, rect, BoxLocation, 1.0);

	/* Clean up */
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
    PrecacheNumbers();
    //GEm: Make sure you precache player names later on
}

void PrecacheTitleText()
{
    char*** CardTitles = GetCardTitleWords();
    int Pool, Card;
    
    for (Pool = 0; Pool < CardDescriptions.NumPools; Pool++)
    {
        for (Card = 0; Card < CardDescriptions.NumSentences[Pool]; Card++)
        {
            CardCache[Pool][Card].TitleTexture.Texture = TextToTexture(Fonts[Font_Title], CardTitles[Pool][Card]);
            TTF_SizeText(Fonts[Font_Title], CardTitles[Pool][Card], &(CardCache[Pool][Card].TitleTexture.TextureSize.X), &(CardCache[Pool][Card].TitleTexture.TextureSize.Y));
        }
        free(CardTitles[Pool]);
    }
    free(CardTitles);
}

void PrecacheDescriptionText()
{
    Size CardSize = {(int)(GetDrawScale()*2*88), (int)(GetDrawScale()*2*53)};
    int Pool, Sentence, Line, Word;
    int SpaceLength;
    int LineHeight; TTF_SizeText(Fonts[Font_Description], " ", &SpaceLength, &LineHeight);
    int WordLength, LineLength;
    int LastLineEnd;
    int i;
    int TextSize = 0;
    char* CurrentLine;
    GLuint CurrentTexture;
    Size TextureSize;
    
    for (Pool = 0; Pool < CardDescriptions.NumPools; Pool++)
    {
        for (Sentence = 0; Sentence < CardDescriptions.NumSentences[Pool]; Sentence++)
        {
            for (Line = 0; Line < CardDescriptions.NumLines[Pool][Sentence]; Line++)
            {
                LineLength = 0;
                LastLineEnd = 0;
                
                for (Word = 0; Word < CardDescriptions.NumWords[Pool][Sentence][Line]; Word++)
                {
                    TTF_SizeText(Fonts[Font_Description], CardDescriptions.Text[Pool][Sentence][Line][Word], &WordLength, NULL); //GE: Set the current word length.
                    
                    if ((LineLength == 0 && LineLength + WordLength > CardSize.X)
                    ||(LineLength > 0 && LineLength + SpaceLength + WordLength > CardSize.X) //GEm: Next word won't fit,
                    ||(Word+1 == CardDescriptions.NumWords[Pool][Sentence][Line])) //GEm: or there are no more words left
                    {
                        //GE: This line is full, write to cache.
                        for (i=0; i < Word+1-LastLineEnd; i++)
                        {
                            TextSize += strlen(CardDescriptions.Text[Pool][Sentence][Line][i]);
                            if (i > 0)
                                TextSize++;
                        }
                        CurrentLine = (char*) malloc((TextSize+1)*sizeof(char)); TextSize = 0;
                        strcpy(CurrentLine, CardDescriptions.Text[Pool][Sentence][Line][0]);
                        for (i=1; i < Word+1-LastLineEnd; i++)
                        {
                            strcat(CurrentLine, " ");
                            strcat(CurrentLine, CardDescriptions.Text[Pool][Sentence][Line][i]);
                        }
                        CurrentTexture = TextToTexture(Fonts[Font_Description], CurrentLine);
                        TTF_SizeText(Fonts[Font_Description], CurrentLine, &(TextureSize.X), &(TextureSize.Y));
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

void PrecachePriceText()
{
    int Pool, Card, BrickCost, GemCost, RecruitCost;
    //char* ReadableNumber = (char*) malloc(4*sizeof(char)); //GE: Prices should never go over 999
    char ReadableNumber[4];
    GLuint ZeroTexture = TextToTexture(Fonts[Font_Title], "0"); //GE: Small optimisation - 0 is very common, so use a common texture for that
    Size ZeroSize; TTF_SizeText(Fonts[Font_Title], "0", &(ZeroSize.X), &(ZeroSize.Y));
    
    for (Pool = 0; Pool < CardDescriptions.NumPools; Pool++)
    {
        for (Card = 0; Card < CardDescriptions.NumSentences[Pool]; Card++)
        {
            GetCardPrice(Pool, Card, &BrickCost, &GemCost, &RecruitCost);
            
            if (BrickCost)
            {
                sprintf(ReadableNumber, "%d", BrickCost); //GE: Convert int to char*
                CardCache[Pool][Card].PriceTexture[0].Texture = TextToTexture(Fonts[Font_Title], ReadableNumber);
                TTF_SizeText(Fonts[Font_Title], ReadableNumber, &(CardCache[Pool][Card].PriceTexture[0].TextureSize.X), &(CardCache[Pool][Card].PriceTexture[0].TextureSize.Y));
            }
            else
            {
                CardCache[Pool][Card].PriceTexture[0].Texture = ZeroTexture;
                CardCache[Pool][Card].PriceTexture[0].TextureSize = ZeroSize;
            }
            
            if (GemCost)
            {
                sprintf(ReadableNumber, "%d", GemCost);
                CardCache[Pool][Card].PriceTexture[1].Texture = TextToTexture(Fonts[Font_Title], ReadableNumber);
                TTF_SizeText(Fonts[Font_Title], ReadableNumber, &(CardCache[Pool][Card].PriceTexture[1].TextureSize.X), &(CardCache[Pool][Card].PriceTexture[1].TextureSize.Y));
            }
            else
            {
                CardCache[Pool][Card].PriceTexture[1].Texture = ZeroTexture;
                CardCache[Pool][Card].PriceTexture[1].TextureSize = ZeroSize;
            }
            
            if (RecruitCost)
            {
                sprintf(ReadableNumber, "%d", RecruitCost);
                CardCache[Pool][Card].PriceTexture[2].Texture = TextToTexture(Fonts[Font_Title], ReadableNumber);
                TTF_SizeText(Fonts[Font_Title], ReadableNumber, &(CardCache[Pool][Card].PriceTexture[2].TextureSize.X), &(CardCache[Pool][Card].PriceTexture[2].TextureSize.Y));
            }
            else
            {
                CardCache[Pool][Card].PriceTexture[2].Texture = ZeroTexture;
                CardCache[Pool][Card].PriceTexture[2].TextureSize = ZeroSize;
            }
        }
    }
}

void PrecacheNumbers()
{
    int i, n;
    char ReadableNumber[2]; //GEm: Has to be a string, even if it's a single char - needs \0
    SDL_Color Colour = {200, 200, 0};
    
    for (n=0; n<Numbers_Count; n++)
    {
        for (i=0; i<10; i++) //GEm: Numbers match their positions, NC[0]=0, NC[9]=9
        {
            sprintf(ReadableNumber, "%d", i);
            if (n == Numbers_Medium)
                NumberCache[n][i].Texture = TextToTexture(NumberFonts[n], ReadableNumber);
            else
                NumberCache[n][i].Texture = TextToTextureColour(NumberFonts[n], ReadableNumber, Colour);
            TTF_SizeText(NumberFonts[n], ReadableNumber, &(NumberCache[n][i].TextureSize.X), &(NumberCache[n][i].TextureSize.Y)); //GEm: We are the knights who say NI!
        }
    }
}

void PrecachePlayerNames()
{
    int NumPlayers = 2; //GEm: TODO implement variable amount of players!
    int i;
    SDL_Color Colour = {200, 200, 0};
    
    for (i=0; i<NumPlayers; i++)
    {
        NameCache[i].Texture = TextToTextureColour(Fonts[Font_Name], GetPlayerName(i), Colour);
        TTF_SizeText(Fonts[Font_Name], GetPlayerName(i), &(NameCache[i].TextureSize.X), &(NameCache[i].TextureSize.Y));
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

/**
 * Find the lower of the two values (type int).
 */ 
int Min(int A, int B)
{
    if (A <= B)
        return A;
    else
        return B;
}

/**
 * Convert text string into OpenGL texture. Returns its handle.
 */ 
GLuint TextToTextureColour(TTF_Font* Font, char* Text, SDL_Color Colour)
{
    SDL_Surface* Initial;
	GLuint Texture;

	Initial = TTF_RenderText_Blended(Font, Text, Colour);
    Texture = SurfaceToTexture(Initial);
    
	SDL_FreeSurface(Initial);
    return Texture;
}

/**
 * Convert text string into OpenGL texture. Returns its handle.
 */ 
GLuint TextToTexture(TTF_Font* Font, char* Text)
{
    SDL_Color Colour = {0, 0, 0};
    return TextToTextureColour(Font, Text, Colour);
}

/**
 * Centre a box in another box. Ignores Y.
 * Parameters:
 * Destination - The (top) left edge of the bounding box.
 * ObjectSize - The length of the texture you wish to draw.
 * BoundingBox - The size of the bounding box you want to fit things in.
 */ 
SizeF CentreOnX(SizeF Destination, SizeF ObjectSize, SizeF BoundingBox)
{
    Destination.X += (BoundingBox.X - ObjectSize.X)/2.0;
    return Destination;
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
