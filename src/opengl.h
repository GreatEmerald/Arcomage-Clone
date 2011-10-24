/**
 * The header file for opengl.c. This is here only for safety reasons.
 */ 

#ifndef _OPENGL_H_
#define _OPENGL_H_ 1

void InitOpenGL();
GLuint SurfaceToTexture(SDL_Surface* surface);
void FreeTextures(GLuint Texture);
void DrawTexture(GLuint Texture, Size TexSize, SDL_Rect SourceCoords, SizeF DestinationCoords, float ScaleFactor);
void DrawRectangle(SizeF DestinationCoords, SizeF DestinationWH, SDL_Colour Colour);
void DrawGradient(SizeF DestinationCoords, SizeF DestinationWH, SDL_Colour ColourA, SDL_Colour ColourB);

#endif
