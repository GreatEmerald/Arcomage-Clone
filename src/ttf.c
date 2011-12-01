/**
 * Utility functions related to TrueType font rendering.
 * Authors: GreatEmerald, 2011
 */

#include <SDL_ttf.h>

/**
 * A shortened initialisation function for TTF.
 */
void InitTTF()
{
    if (TTF_Init()) //GE: Abusing the fact that -1 is considered true
        FatalError(TTF_GetError());
}

/**
 * Render a single line. Do not use for formatted text!
 * Authors: C-Junkie, GreatEmerald
 */ 
void RenderLine(char* text, TTF_Font* font, SDL_Color color, SDL_Rect location)
{

	SDL_Surface *initial;
	SDL_Surface *intermediary;
	SDL_Rect rect;
	int w,h;
	int texture;

	/* Use SDL_TTF to render our text */
	initial = TTF_RenderText_Blended(font, text, color); //GE: TODO - add support for shading

	/* Convert the rendered text to a known format */
    //GE: Probably not necessary when we have support for non-power of 2 textures.
	w = nextpoweroftwo(initial->w);
	h = nextpoweroftwo(initial->h);

	intermediary = SDL_CreateRGBSurface(0, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

	SDL_BlitSurface(initial, 0, intermediary, 0);

	/* Tell GL about our new texture */
    texture = SurfaceToTexture(intermediary);
    DrawTexture(texture, Size TexSize, SDL_Rect SourceCoords, SizeF DestinationCoords, float ScaleFactor) //GE: TODO

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
 * words in 90px (9*10). It then finds out how many lines said string
 * would take up if it was bound to a rectangle. If it's more than is
 * allowed, then it scales the size twice. If it fits, it scales up and
 * repeats until the size is correct. Perhaps trial and error are not
 * necessary and this can be done using a formula?
 */
int FindOptimalFontSize(SDL_Rect CardSize)
{
    int NumSentences;
    int* NumWords;
    char*** Words = GetCardDescriptionWords(&NumSentences, &NumWords);
    int Sentence, Word;
    int** WordLengthList;
    TTF_Font* ProbeFont;
    
    TTF_OpenFont(GetFilePath("fonts/FreeSans.ttf"), 90); //GE: Probe with 9*10, because it gives us good enough resolution (we won't support over 8000x6000 anyway)
    WordLengthList = (int**) malloc(sizeof(int*)*NumSentences); //GE: Set WordLengthList.length = NumSentences;
    
    for (Sentence = 0; Sentence < NumSentences; Sentence++)
    {
        WordLengthList[Sentence] = (int*) malloc(sizeof(int)*NumWords[Sentence]); //GE: Set WordLengthList[Sentence].length = NumWords[Sentence];
        for (Word = 0; Word < NumWords[Sentence]; Word++)
        {
            TTF_SizeText(ProbeFont, Words[Sentence][Word], &WordLengthList[Sentence][Word], NULL); //GE: Populate the word length list.
        }
        //Words[Sentence] means a sentence
    }
}

/**
 * Utility functions for RenderText.
 * Authors: C-Junkie
 */ 
int round(double x)
{
	return (int)(x + 0.5);
}

/**
 * ditto
 */ 
int nextpoweroftwo(int x)
{
	double logbase2 = log(x) / log(2);
	return round(pow(2,ceil(logbase2)));
}
