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
    VkDevice                    device;
    VkQueue                     queue;
    VkSurfaceKHR                surface;
    VkSwapchainKHR              swapchain;
    VkFormat                    swapchainImageFormat;
    VkExtent2D                  swapchainExtent;
    uint32_t                    swapchainImageCount;
    VkImage*                    pSwapchainImages;
    VkImageView*                pSwapchainImageViews;
    VkPipelineLayout            pipelineLayout;
    VkRenderPass                renderPass;
    VkPipeline                  pipeline;
} Application;

Result createApplication(Application* pApplication);

void destroyApplication(Application* pApplication);

#endif // APPLICATION_H
