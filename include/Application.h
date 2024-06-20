#ifndef APPLICATION_H
#define APPLICATION_H

#include <vulkan/vulkan.h>

#include <SDL.h>

#include "base.h"

typedef struct Application
{
    SDL_Window*    pWindow;
    uint32_t       availableInstanceLayerCount;
    char**         ppAvailableInstanceLayers;
    uint32_t       availableInstanceExtensionCount;
    char**         ppAvailableInstanceExtensions;
    uint32_t       requiredInstanceLayerCount;
    const char*    ppRequiredInstanceLayers[1];
    uint32_t       requiredInstanceExtensionCount;
    char**         ppRequiredInstanceExtensions;
    VkInstance     instance;
} Application;

Result createApplication(Application* pApplication);

void destroyApplication(Application* pApplication);

#endif // APPLICATION_H
