/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/* TinyGL implementation of SDL OpenGL support */

#include "../../SDL_internal.h"

#ifndef _SDL_amigaopengl_h
#define _SDL_amigaopengl_h

/* OpenGL functions */
extern int AMIGA_GL_LoadLibrary(_THIS, const char *path);
extern void *AMIGA_GL_GetProcAddress(_THIS, const char *proc);
extern void AMIGA_GL_UnloadLibrary(_THIS);
extern SDL_GLContext AMIGA_GL_CreateContext(_THIS, SDL_Window * window);
extern int AMIGA_GL_MakeCurrent(_THIS, SDL_Window * window, SDL_GLContext context);
extern void AMIGA_GL_GetDrawableSize(_THIS, SDL_Window * window, int *w, int *h);
extern int AMIGA_GL_SetSwapInterval(_THIS, int interval);
extern int AMIGA_GL_GetSwapInterval(_THIS);
extern int AMIGA_GL_SwapWindow(_THIS, SDL_Window * window);
extern void AMIGA_GL_DeleteContext(_THIS, SDL_GLContext context);

/* Non-SDL functions */
extern int AMIGA_GL_ResizeContext(_THIS, SDL_Window *window);

#endif /* _SDL_amigaopengl_h */
