@ECHO OFF
cd ..\src
dmc frontend.c adapter.c graphics.c BFont.c opengl.c ttf.c input.c ^
  ..\lib\OpenGL32.Lib ..\lib\SDL.lib ..\lib\SDL_ttf.lib ..\lib\SDL_image.lib ^
  ..\..\libarcomage\lib\lua51.lib ^
  ..\..\libarcomage\lib\arcomage.lib ^
  ..\..\libarcomage\lib\phobos.lib ^
  -I..\include -L/SUBSYSTEM:WINDOWS -o..\bin\win32\Arcomage.exe
del *.obj
del *.map
echo Rebuild is complete. The new binaries are stored in /bin/win32.
pause
