Arcomage Clone Frontend (arcomage) 1.alpha
Revision 2012.01.13, GreatEmerald

This is the OpenGL graphical frontend for the Arcomage Clone project. It handles
all input, rendering and sound operations in the game.

The frontend is written in the C programming language.

=======
RUNNING
=======

In order to play the game, the user has to either obtain the precompiled
binaries or compile the code manually. For the latter, check the "Building"
section of this document to generate binaries before you proceed to run them.

Once you have the binaries, make sure that the 'lua' directory is in the same
directory as the binary file (the 'lua' directory is part of libarcomage).
Currently, the binary expects the lua directory to be in the same directory,
but that will most likely change in the future to make it more flexible.

Similarly, make sure that the 'data' directory is in the same directory as the
binary file. Alternatively, you may indicate another place for the data
directory by editing the lua/Configuration.lua file.

Using symlinks (junctions on Windows) is also a possible option.

If you are using Linux, you will have to install libSDL-1_2, libSDL_image-1_2
and SDL_ttf to run the game (or their equivalents in your Linux distribution;
see the building section for hints). You also need something that provides
OpenGL, that's usually Mesa.

Due to the fact that the DMD compiler does not support dynamic libraries, you
don't need libarcomage to run the game, as it is included in the binary during
the compilation of the program.

After that, simply run the binary and enjoy!

========
BUILDING
========

Building the Arcomage Clone OpenGL frontend is actully a pretty complex
procedure that involves downloading a few extra programs that you wouldn't
normally use for your every day C compiling needs. Since I believe that it is
important for people to know the motives of using such software, in this section
I will detail why I chose that over anything else.

First off, the short form of what you need to build, depending on the operating
system:

Windows:
  Mandatory:
    DMC (Digital Mars C/C++ compiler): http://www.digitalmars.com/download/freecompiler.html
  Recommended:
    DMC Basic Utilities: http://www.digitalmars.com/download/freecompiler.html
    DMC COFFImpLib: http://ftp.digitalmars.com/coffimplib.zip

NOTE: You cannot compile this project with MinGW or Visual C++ of any version.
Explanation is given below.

Linux:
  Mandatory:
    GCC 4.4+ (GNU Compiler Collection; gcc, gcc-4.4 or gcc-4.5)
    GNU Make (make)
    Embedded GNU C Library (libc6, glibc)
    SDL 1.2 (Simple DirectMedia Layer, 1.2.14; libsdl1.2-dev on Debian and
      libSDL-devel on openSUSE)
    SDL image (libsdl-image-dev on Debian and libSDL_image-devel on openSUSE)
    SDL TTF (2.0.10 preferred; SDL_ttf-devel on openSUSE)
    OpenGL (Mesa-devel on openSUSE)

Firstly I want to explain the system here, and what led to requirements like
that. You will have to delve a bit to the history of the project, so bear with
me.
You might ask why this frontend was written in C in the first place, instead of
something more recent such as C++. The answer is two-fold. On one hand, it's
because the original creator of the Arcomage Clone project, STiCK, had the game
written in C before I took over the development. On the other hand, D2, which is
my language of choice, didn't have any working SDL bindings at the time of
writing.
Why is D my primary language of choice? Well, when I started working on this
project, I was primarily an UnrealScript coder (in case you're wondering,
UnrealScript is the programming language of Unreal Engine games, which is a mix
of C++ and Java). So to me, C felt and still feels archaic. I never had to use
pointers in UnrealScript, and I never needed it, so why should I care about them
here? Or about memory allocation? Plus, without doing all kinds of archaic
tricks, you can't create a lot of things in C, and that includes dynamic arrays
(for example, if you want to use a different amount of cards than is set by
default, you need dynamic arrays, period).
So, being familiar with UnrealScript, after looking into quite a few of
different languages, I have found that D is the most fitting one for me, because
all the programming methods and the syntax is familiar to me.

So that's why I am using D in this project - it handles everything that C can't,
such as dynamic arrays and string manipulation, as well as garbage collection.
That's why the main library is written in D.

Now you might wonder why you can't compile the frontend with any well-known
compiler on Windows, such as MinGW and VC++. It also has to do with D, or,
rather, Microsoft. The static libraries that the D compiler produces is in the
OMF (Object Module Format), which is an open format with well-defined
specifications. However, basically every other compiler out there uses COFF
(Common Object File Format), a format by Microsoft, which is poorly defined and
prone to changes depending on VC++ (case in point: the format changed when
MS released VC++ 6). Since COFF is a lot more popular, all compilers use that.
But they don't recognise OMF, thus you can't use them. Thankfully, DMC, the C
compiler made by the same company that created D, uses OMF as well, and so can
do the job just as well.
Linux doesn't have such a problem, however, because it uses different library
formats altogether (ELF), which are open and thus cause no such conflicts.
That's why you can simply use GCC for compiling the code there.

OMF/COFF issue is also why the tools are recommended there. Basic Utilities
include ImpLib, which is used for building OMF .lib files from DLLs (note: for
some insane reason, when using ImpLib, the output file goes first. See more
here: http://www.digitalmars.com/ctg/implib.html). You wouldn't normally need
this, however, unless you wish to add more libraries to the project. COFFImpLib
is a different utility, and it's very useful - it converts COFF .lib files to
OMF .lib files. That's the usual thing that you'd want to do if you want to add
a library to the project. More info about it here:
http://www.digitalmars.com/ctg/coffimplib.html

Now about Linux. If you don't know why you need GCC, GNU Make and libc6, then,
well, you're missing out. SDL is the part that is responsible for all the
rendering, so it's naturally required. SDL_image can import PNGs, SDL_ttf
handles font rendering, and OpenGL is required to import the OpenGL-specific
part of SDL.

Right now I only have the x86_64 (AMD64) Linux binaries precompiled, but if you
need to, you should be able to compile in a 32-bit environment as well.
In that case you'll have to change the src/Makefile to include -m32 instead of
-m64. And make sure you copy the binaries to a new directory and let me know :)

=========
STRUCTURE
=========

You probably noticed that the project has several directories. Here's the
current directory tree:

  +-- bin
  +-- data
  |   +-- fonts
  +-- include
  +-- lib
  +-- src
  +-- utils
  |-- COPYING.TXT
  |-- README.TXT

I will go through what each folder contains. First off, the bin folder contains
the precompiled binaries that are needed in order to run the game. Right now, it
contains only the 64-bit Linux binaries, but it will contain Windows binaries
in the future as well.

data is the folder that contains resources for the program. At the moment they
are standalone, but might become integrated into a resource file later on. The
folder contains .wav, .ico and .png files for the players. The subdirectory
fonts includes the GNU FreeFonts, used to render fonts in the game.

include contains the include header files for the libraries used by C. It's also
utterly useless for Linux users, because there includes are provided in the
library packages themselves.

lib contains the needed static libraries for compiling the project. The static
libary created from the D source code also goes in here. Again, it's utterly
useless for Linux users.

src is the C source code. That's the crucial (and the most interesting) part of
the frontend.

utils contains utilities for building the code. Included are scripts for quick
compilation on both Linux and Windows.

COPYING.TXT is a text file that contains GPL licensing information.

README.TXT is this file, in case you haven't noticed :)
