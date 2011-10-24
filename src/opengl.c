/**
 * File that contains the OpenGL-specific code and wraps around it. This is
 * specific for this projects.
 * 
 * Authors: GreatEmerald, 2011
 */  

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <stdio.h>
#include "graphics.h"
#include "adapter.h"

void InitOpenGL()
{
    glEnable( GL_TEXTURE_2D ); //Enable 2D texturing support
 
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f ); //Set the clear colour
     
    glViewport( 0, 0, GetConfig(ResolutionX), GetConfig(ResolutionY) ); //Set the size of the window. 
     
    glClear( GL_COLOR_BUFFER_BIT ); //Clear the screen.
     
    glMatrixMode( GL_PROJECTION ); //Set the output to be a projection (2D plane).
    glLoadIdentity();
     
    glOrtho(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f); //Set the coordinates to not be wacky - 1 unit is a pixel
     
    glMatrixMode( GL_MODELVIEW ); //Set to show models.
}

GLuint SurfaceToTexture(SDL_Surface* surface)
{
    GLint  nOfColors;
    GLuint texture;
    GLenum texture_format;
    
    // get the number of channels in the SDL surface
    nOfColors = surface->format->BytesPerPixel;
    if (nOfColors == 4)     // contains an alpha channel
    {
            if (surface->format->Rmask == 0x000000ff)
                    texture_format = GL_RGBA;
            else
                    texture_format = GL_BGRA;
    }
    else if (nOfColors == 3)     // no alpha channel
    {
            if (surface->format->Rmask == 0x000000ff)
                    texture_format = GL_RGB;
            else
                    texture_format = GL_BGR;
    }
    else
    {
            printf("Warning: The image is not Truecolour. This will probably break.\n");
    }
     
    // Have OpenGL generate a texture object handle for us
    glGenTextures( 1, &texture );
 
    // Bind the texture object
    glBindTexture( GL_TEXTURE_2D, texture );
 
    // Set the texture's stretching properties
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
 
    // Edit the texture object's image data using the information SDL_Surface gives us
    glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
                      texture_format, GL_UNSIGNED_BYTE, surface->pixels );
    
    
    return texture;
}

void FreeTextures(GLuint Texture)
{
    if (glIsTexture(Texture))
        glDeleteTextures(1, &Texture);
}

/**
 * Draw an OpenGL texture on the screen.
 * \param Texture The OpenGL texture handler.
 * \param TexSize Two ints that show the size of the texture in the handler. You get this by looking into TextureCoordinates[].
 * \param SourceCoords A SDL_Rect that defines what part of the texture to draw.
 * \param DestinationCoords Two floats that define, in percentage, where the texture is to be positioned on the screen. The values indicate the top left pixel.
 * \param ScaleFactor A float that indicates (in percentage) the amount of scaling to use. For the original data scaled only by the chosen resolution, use 1.0.
 */ 
void DrawTexture(GLuint Texture, Size TexSize, SDL_Rect SourceCoords, SizeF DestinationCoords, float ScaleFactor)
{
    int ResX = GetConfig(ResolutionX);
    int ResY = GetConfig(ResolutionY);
    float DestinationW = (float)SourceCoords.w * ScaleFactor;
    float DestinationH = (float)SourceCoords.h * ScaleFactor;
    
    if (!glIsTexture(Texture))
        printf("Warning: DrawTexture: This texture is not a valid OpenGL texture!\n");
    printf("Info: DrawTexture: Called with (%d, {%d, %d}, {%d, %d, %d, %d}, {%f, %f}, %f)\n", Texture, TexSize.X, TexSize.Y, SourceCoords.x, SourceCoords.y, SourceCoords.w, SourceCoords.h, DestinationCoords.X, DestinationCoords.Y, ScaleFactor);
    
    // Bind the texture to which subsequent calls refer to
    glBindTexture( GL_TEXTURE_2D, Texture );
     
    glBegin( GL_QUADS );
        //Top-left vertex (corner)
        glTexCoord2f( (float)SourceCoords.x/(float)TexSize.X, (float)SourceCoords.y/(float)TexSize.Y );
        glVertex2f( (float)DestinationCoords.X, (float)DestinationCoords.Y);
        printf("Info: DrawTexture: Drawing glTexCoord2f(%f, %f); glVertex2f(%f, %f)\n", (float)SourceCoords.x/(float)TexSize.X, (float)SourceCoords.y/(float)TexSize.Y, (float)DestinationCoords.X, (float)DestinationCoords.Y);
     
        //Top-right vertex (corner)
        glTexCoord2f( (float)SourceCoords.x+(float)SourceCoords.w/(float)TexSize.X, (float)SourceCoords.y/(float)TexSize.Y );
        glVertex2f( (float)DestinationCoords.X+DestinationW/(float)ResX, (float)DestinationCoords.Y);
        printf("Info: DrawTexture: Drawing glTexCoord2f(%f, %f); glVertex2f(%f, %f)\n", (float)SourceCoords.x+(float)SourceCoords.w/(float)TexSize.X, (float)SourceCoords.y/(float)TexSize.Y, (float)DestinationCoords.X+DestinationW/(float)ResX, (float)DestinationCoords.Y);
     
        //Bottom-right vertex (corner)
        glTexCoord2f( (float)SourceCoords.x+(float)SourceCoords.w/(float)TexSize.X, (float)SourceCoords.y+(float)SourceCoords.h/(float)TexSize.Y );
        glVertex2f( (float)DestinationCoords.X+DestinationW/(float)ResX, (float)DestinationCoords.Y+DestinationH/(float)ResY);
        printf("Info: DrawTexture: Drawing glTexCoord2f(%f, %f); glVertex2f(%f, %f)\n", (float)SourceCoords.x+(float)SourceCoords.w/(float)TexSize.X, (float)SourceCoords.y+(float)SourceCoords.h/(float)TexSize.Y, (float)DestinationCoords.X+DestinationW/(float)ResX, (float)DestinationCoords.Y+DestinationH/(float)ResY);
     
        //Bottom-left vertex (corner)
        glTexCoord2f( (float)SourceCoords.x/(float)TexSize.X, (float)SourceCoords.y+(float)SourceCoords.h/(float)TexSize.Y );
        glVertex2f( (float)DestinationCoords.X, (float)DestinationCoords.Y+DestinationH/(float)ResY);
        printf("Info: DrawTexture: Drawing glTexCoord2f(%f, %f); glVertex2f(%f, %f)\n", (float)SourceCoords.x/(float)TexSize.X, (float)SourceCoords.y+(float)SourceCoords.h/(float)TexSize.Y, (float)DestinationCoords.X, (float)DestinationCoords.Y+DestinationH/(float)ResY);
    glEnd();
}

void DrawRectangle(SizeF DestinationCoords, SizeF DestinationWH, SDL_Colour Colour)
{
    glDisable(GL_TEXTURE_2D); //We need a solid colour, thus texturing support is irrelevant. Also, this does not affect things we have already rendered.
    glColor4f((float)Colour.r/255.0, (float)Colour.g/255.0, (float)Colour.b/255.0, (float)Colour.unused/255.0);
    glRectf(DestinationCoords.X, DestinationCoords.Y, DestinationCoords.X+DestinationWH.X, DestinationCoords.Y+DestinationWH.Y);
    glEnable(GL_TEXTURE_2D);
}

void DrawGradient(SizeF DestinationCoords, SizeF DestinationWH, SDL_Colour ColourA, SDL_Colour ColourB)
{
    glDisable(GL_TEXTURE_2D); //We need a solid colour, thus texturing support is irrelevant. Also, this does not affect things we have already rendered.
    glBegin(GL_POLYGON);
        glColor4f((float)ColourA.r/255.0, (float)ColourA.g/255.0, (float)ColourA.b/255.0, (float)ColourA.unused/255.0);
        glVertex2f(DestinationCoords.X, DestinationCoords.Y);
        glVertex2f(DestinationCoords.X+DestinationWH.X, DestinationCoords.Y);
        glColor4f((float)ColourB.r/255.0, (float)ColourB.g/255.0, (float)ColourB.b/255.0, (float)ColourB.unused/255.0);
        glVertex2f(DestinationCoords.X+DestinationWH.X, DestinationCoords.Y+DestinationWH.Y);
        glVertex2f(DestinationCoords.X, DestinationCoords.Y+DestinationWH.Y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
