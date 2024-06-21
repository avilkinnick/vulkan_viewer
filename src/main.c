#include <stdlib.h>

#include "Application.h"

int main(int argc, char* argv[])
{
    Application application;
    if (createApplication(&application) != SUCCESS)
    {
        printError("Failed to create application!");
        return EXIT_FAILURE;
    }

    SDL_bool quit = SDL_FALSE;
    while (quit != SDL_TRUE)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event) == 1)
        {
            switch (event.type)
            {
                case SDL_QUIT: quit = SDL_TRUE; break;
                case SDL_KEYDOWN:
                {
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE: quit = SDL_TRUE; break;
                        default: break;
                    }
                }
                default: break;
            }
        }
    }

    destroyApplication(&application);

    return EXIT_SUCCESS;
}
