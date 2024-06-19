#include "base.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <SDL.h>

void printError(const char* pFormat, ...)
{
    va_list arg;
    va_start(arg, pFormat);
    vfprintf(stderr, pFormat, arg);
    va_end(arg);
    fprintf(stderr, "\n");

    const char* pSDL_Error = SDL_GetError();
    if (strlen(pSDL_Error) > 0)
    {
        fprintf(stderr, "%s\n", pSDL_Error);
    }

    fprintf(stderr, "\n");
}
