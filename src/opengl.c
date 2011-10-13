/**
 * File that contains the OpenGL-specific code and wraps around it. This is
 * specific for this projects.
 * 
 * Authors: GreatEmerald, 2011
 */  

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "adapter.h"

void InitOpenGL()
{
    glEnable( GL_TEXTURE_2D ); //Enable 2D texturing support
 
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f ); //Set the clear colour
     
    glViewport( 0, 0, GetConfig(ResolutionX), GetConfig(ResolutionY) ); //Set the size of the window. 
     
    glClear( GL_COLOR_BUFFER_BIT ); //Clear the screen.
     
    glMatrixMode( GL_PROJECTION ); //Set the output to be a projection (2D plane).
    glLoadIdentity();
     
    glOrtho(0.0f, GetConfig(ResolutionX), GetConfig(ResolutionY), 0.0f, -1.0f, 1.0f); //Set the coordinates to not be wacky - 1 unit is a pixel
     
    glMatrixMode( GL_MODELVIEW ); //Set to show models.
}

GLuint SurfaceToTexture(SDL_Surface surface)
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
}
