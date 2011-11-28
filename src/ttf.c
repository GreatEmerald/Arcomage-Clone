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
 * Render text in some fashion. I need to test this out.
 * Authors: C-Junkie
 */ 
void SDL_GL_RenderText(char *text, TTF_Font *font, SDL_Color color, SDL_Rect *location)
{

	SDL_Surface *initial;
	SDL_Surface *intermediary;
	SDL_Rect rect;
	int w,h;
	int texture;

	/* Use SDL_TTF to render our text */
	initial = TTF_RenderText_Blended(font, text, color); //GE: TODO - add support for shading

	/* Convert the rendered text to a known format */
	w = nextpoweroftwo(initial->w);
	h = nextpoweroftwo(initial->h);

	

	intermediary = SDL_CreateRGBSurface(0, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);



	SDL_BlitSurface(initial, 0, intermediary, 0);

	

	/* Tell GL about our new texture */
    texture = SurfaceToTexture(intermediary);

    DrawTexture(texture, Size TexSize, SDL_Rect SourceCoords, SizeF DestinationCoords, float ScaleFactor) //GE: TODO

	/* return the deltas in the unused w,h part of the rect */

	location->w = initial->w;

	location->h = initial->h;

	

	/* Clean up */

	SDL_FreeSurface(initial);

	SDL_FreeSurface(intermediary);

	glDeleteTextures(1, &texture);

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
