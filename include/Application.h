#ifndef APPLICATION_H
#define APPLICATION_H

#include <vulkan/vulkan.h>

#include <SDL.h>
#include <SDL_vulkan.h>

#include "base.h"

typedef struct Application
{
    SDL_Window*                 pWindow;
    VkInstance                  instance;
    VkDebugUtilsMessengerEXT    debugUtilsMessenger;
    VkPhysicalDevice            physicalDevice;
} Application;

Result createApplication(Application* pApplication);

void destroyApplication(Application* pApplication);

#endif // APPLICATION_H
