#ifndef APPLICATION_H
#define APPLICATION_H

#include <vulkan/vulkan.h>

#include <SDL.h>

#include "base.h"

typedef struct Application
{
    SDL_Window*    pWindow;
    VkInstance     instance;
} Application;

Result createApplication(Application* pApplication);

Result createWindow(SDL_Window** ppWindow);

Result createInstance(VkInstance* pInstance);

void destroyApplication(Application* pApplication);

#endif // APPLICATION_H
