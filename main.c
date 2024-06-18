#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vulkan/vulkan.h>

#include <SDL.h>
#include <SDL_vulkan.h>

void printError(const char* pFormat, ...);

typedef struct Application
{
    SDL_Window*                            pWindow;
    VkInstance                             instance;
    VkDebugUtilsMessengerEXT               debugUtilsMessenger;
    VkDevice                               device;
    VkSurfaceKHR                           surface;
    VkSwapchainKHR                         swapchain;

    PFN_vkGetInstanceProcAddr              vkGetInstanceProcAddr;
    PFN_vkCreateDebugUtilsMessengerEXT     vkCreateDebugUtilsMessengerEXT;
    PFN_vkDestroyDebugUtilsMessengerEXT    vkDestroyDebugUtilsMessengerEXT;
} Application;

void destroyApplication(Application* pApplication);

VKAPI_ATTR VkBool32 debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT         messageSeverity,
                                                VkDebugUtilsMessageTypeFlagsEXT                messageTypes,
                                                const VkDebugUtilsMessengerCallbackDataEXT*    pCallbackData,
                                                void*                                          pUserData);

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printError("Failed to initialize SDL library!");
        return EXIT_FAILURE;
    }

    Application application;
    application.pWindow = NULL;
    application.instance = NULL;
    application.debugUtilsMessenger = NULL;
    application.device = NULL;
    application.surface = NULL;
    application.swapchain = NULL;

    application.vkGetInstanceProcAddr = NULL;
    application.vkCreateDebugUtilsMessengerEXT = NULL;
    application.vkDestroyDebugUtilsMessengerEXT = NULL;

    application.pWindow = SDL_CreateWindow("Viewer", 100, 100, 1600, 900, SDL_WINDOW_VULKAN);
    if (application.pWindow == NULL)
    {
        printError("Failed to create SDL window!");
        destroyApplication(&application);
        return EXIT_FAILURE;
    }

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo;
    debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugUtilsMessengerCreateInfo.pNext = NULL;
    debugUtilsMessengerCreateInfo.flags = 0;
    debugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                                    // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugUtilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
                                                | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    debugUtilsMessengerCreateInfo.pfnUserCallback = debugUtilsMessengerCallback;
    debugUtilsMessengerCreateInfo.pUserData = NULL;

    uint32_t apiVersion;
    vkEnumerateInstanceVersion(&apiVersion);

    VkApplicationInfo applicationInfo;
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = NULL;
    applicationInfo.pApplicationName = "Vulkan viewer";
    applicationInfo.applicationVersion = 1;
    applicationInfo.pEngineName = NULL;
    applicationInfo.engineVersion = 0;
    applicationInfo.apiVersion = apiVersion;

    uint32_t requiredLayerCount = 1;
    const char* ppRequiredLayers[1] = {"VK_LAYER_KHRONOS_validation"};

    unsigned int requiredExtensionCount;
    SDL_Vulkan_GetInstanceExtensions(application.pWindow, &requiredExtensionCount, NULL);

    const char** ppRequiredExtensions = malloc((requiredExtensionCount + 2) * sizeof(const char*));
    if (ppRequiredExtensions == NULL)
    {
        printError("Failed to allocate memory for required extensions!");
        destroyApplication(&application);
        return EXIT_FAILURE;
    }

    SDL_Vulkan_GetInstanceExtensions(application.pWindow, &requiredExtensionCount, ppRequiredExtensions);

    ppRequiredExtensions[requiredExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    ppRequiredExtensions[requiredExtensionCount + 1] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    requiredExtensionCount += 2;

    VkInstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = &debugUtilsMessengerCreateInfo;
    instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledLayerCount = requiredLayerCount;
    instanceCreateInfo.ppEnabledLayerNames = ppRequiredLayers;
    instanceCreateInfo.enabledExtensionCount = requiredExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = ppRequiredExtensions;

    if (vkCreateInstance(&instanceCreateInfo, NULL, &application.instance) != VK_SUCCESS)
    {
        printError("Failed to create instance!");
        destroyApplication(&application);
        return EXIT_FAILURE;
    }

    free(ppRequiredExtensions);

    application.vkGetInstanceProcAddr = SDL_Vulkan_GetVkGetInstanceProcAddr();
    if (application.vkGetInstanceProcAddr == NULL)
    {
        printError("Failed to get address of Vulkan procedure \"vkGetInstanceProcAddr\"!");
        destroyApplication(&application);
        return EXIT_FAILURE;
    }

    application.vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)application.vkGetInstanceProcAddr(application.instance, "vkCreateDebugUtilsMessengerEXT");
    if (application.vkCreateDebugUtilsMessengerEXT == NULL)
    {
        printError("Failed to get address of Vulkan procedure \"vkGetInstanceProcAddr\"!");
        destroyApplication(&application);
        return EXIT_FAILURE;
    }

    application.vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)application.vkGetInstanceProcAddr(application.instance, "vkDestroyDebugUtilsMessengerEXT");
    if (application.vkDestroyDebugUtilsMessengerEXT == NULL)
    {
        printError("Failed to get address of Vulkan procedure \"vkDestroyDebugUtilsMessengerEXT\"!");
        destroyApplication(&application);
        return EXIT_FAILURE;
    }

    if (application.vkCreateDebugUtilsMessengerEXT(application.instance, &debugUtilsMessengerCreateInfo, NULL, &application.debugUtilsMessenger) != VK_SUCCESS)
    {
        printError("Failed to create debug utils messenger!");
        destroyApplication(&application);
        return EXIT_FAILURE;
    }

    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(application.instance, &physicalDeviceCount, NULL);

    VkPhysicalDevice* pPhysicalDevices = malloc(physicalDeviceCount * sizeof(VkPhysicalDevice));
    if (pPhysicalDevices == NULL)
    {
        printError("Failed to allocate memory for physical devices!");
        destroyApplication(&application);
        return EXIT_FAILURE;
    }

    vkEnumeratePhysicalDevices(application.instance, &physicalDeviceCount, pPhysicalDevices);

    VkPhysicalDevice physicalDevice = pPhysicalDevices[0];

    free(pPhysicalDevices);

    float pQueuePriorities[1] = {1.0f};

    VkDeviceQueueCreateInfo deviceQueueCreateInfo;
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.pNext = NULL;
    deviceQueueCreateInfo.flags = 0;
    deviceQueueCreateInfo.queueFamilyIndex = 0;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.pQueuePriorities = pQueuePriorities;

    uint32_t deviceRequiredExtensionCount = 1;
    const char* ppDeviceRequiredExtensions[1] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = NULL;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = NULL;
    deviceCreateInfo.enabledExtensionCount = deviceRequiredExtensionCount;
    deviceCreateInfo.ppEnabledExtensionNames = ppDeviceRequiredExtensions;
    deviceCreateInfo.pEnabledFeatures = NULL;

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &application.device) != VK_SUCCESS)
    {
        printError("Failed to create device!");
        destroyApplication(&application);
        return EXIT_FAILURE;
    }

    VkQueue presentQueue;
    vkGetDeviceQueue(application.device, 0, 0, &presentQueue);

    if (SDL_Vulkan_CreateSurface(application.pWindow, application.instance, &application.surface) != SDL_TRUE)
    {
        printError("Failed to create surface!");
        destroyApplication(&application);
        return EXIT_FAILURE;
    }

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, application.surface, &surfaceCapabilities);

    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    if ((surfaceCapabilities.maxImageCount > 0) && (imageCount > surfaceCapabilities.maxImageCount))
    {
        imageCount = surfaceCapabilities.maxImageCount;
    }

    uint32_t surfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, application.surface, &surfaceFormatCount, NULL);

    VkSurfaceFormatKHR* pSurfaceFormats = malloc(surfaceFormatCount * sizeof(VkSurfaceFormatKHR));
    if (pSurfaceFormats == NULL)
    {
        printError("Failed to allocate memory for surface formats!");
        destroyApplication(&application);
        return EXIT_FAILURE;
    }

    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, application.surface, &surfaceFormatCount, pSurfaceFormats);

    VkSurfaceFormatKHR surfaceFormat = pSurfaceFormats[0];
    for (uint32_t i = 0; i < surfaceFormatCount; ++i)
    {
        VkSurfaceFormatKHR* pSurfaceFormat = &pSurfaceFormats[i];

        if ((pSurfaceFormat->format == VK_FORMAT_B8G8R8A8_SRGB) && (pSurfaceFormat->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
        {
            surfaceFormat = *pSurfaceFormat;
            break;
        }
    }

    free(pSurfaceFormats);

    VkExtent2D extent;
    if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        extent = surfaceCapabilities.currentExtent;
    }
    else
    {
        SDL_Vulkan_GetDrawableSize(application.pWindow, (int*)&extent.width, (int*)&extent.height);

        if (extent.width < surfaceCapabilities.minImageExtent.width)
        {
            extent.width = surfaceCapabilities.minImageExtent.width;
        }
        else if (extent.width > surfaceCapabilities.maxImageExtent.width)
        {
            extent.width = surfaceCapabilities.maxImageExtent.width;
        }

        if (extent.height < surfaceCapabilities.minImageExtent.height)
        {
            extent.height = surfaceCapabilities.minImageExtent.height;
        }
        else if (extent.height > surfaceCapabilities.maxImageExtent.height)
        {
            extent.height = surfaceCapabilities.maxImageExtent.height;
        }
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, application.surface, &presentModeCount, NULL);

    VkPresentModeKHR* pPresentModes = malloc(presentModeCount * sizeof(VkPresentModeKHR));
    if (pPresentModes == NULL)
    {
        printError("Failed to allocate memory for present modes!");
        destroyApplication(&application);
        return EXIT_FAILURE;
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, application.surface, &presentModeCount, pPresentModes);

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < presentModeCount; ++i)
    {
        if (pPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    free(pPresentModes);

    VkSwapchainCreateInfoKHR swapchainCreateInfo;
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = NULL;
    swapchainCreateInfo.flags = 0;
    swapchainCreateInfo.surface = application.surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = NULL;
    swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(application.device, &swapchainCreateInfo, NULL, &application.swapchain) != VK_SUCCESS)
    {
        printError("Failed to create swapchain!");
        destroyApplication(&application);
        return EXIT_FAILURE;
    }

    uint32_t swapchainImageCount;
    vkGetSwapchainImagesKHR(application.device, application.swapchain, &swapchainImageCount, NULL);

    VkImage* pSwapchainImages = malloc(swapchainImageCount * sizeof(VkImage));
    if (pSwapchainImages == NULL)
    {
        printError("Failed to allocate memory for swapchain images!");
        destroyApplication(&application);
        return EXIT_FAILURE;
    }

    vkGetSwapchainImagesKHR(application.device, application.swapchain, &swapchainImageCount, pSwapchainImages);

    VkImageView* pSwapchainImageViews = malloc(swapchainImageCount * sizeof(VkImageView));
    if (pSwapchainImageViews == NULL)
    {
        printError("Failed to allocate memory for swapchain image views!");
        free(pSwapchainImages);
        destroyApplication(&application);
        return EXIT_FAILURE;
    }

    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        VkImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = NULL;
        // imageViewCreateInfo.flags;
        // imageViewCreateInfo.image;
        // imageViewCreateInfo.viewType;
        // imageViewCreateInfo.format;
        // imageViewCreateInfo.components;
        // imageViewCreateInfo.subresourceRange;
    }

    free(pSwapchainImageViews);
    free(pSwapchainImages);

    SDL_bool quit = SDL_FALSE;
    while (quit != SDL_TRUE)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event) == 1)
        {
            if (event.type == SDL_QUIT)
            {
                quit = SDL_TRUE;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    quit = SDL_TRUE;
                }
            }
        }
    }

    destroyApplication(&application);

    return EXIT_SUCCESS;
}

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

void destroyApplication(Application* pApplication)
{
    vkDestroySwapchainKHR(pApplication->device, pApplication->swapchain, NULL);
    vkDestroySurfaceKHR(pApplication->instance, pApplication->surface, NULL);
    vkDestroyDevice(pApplication->device, NULL);

    if (pApplication->vkDestroyDebugUtilsMessengerEXT != NULL)
    {
        pApplication->vkDestroyDebugUtilsMessengerEXT(pApplication->instance, pApplication->debugUtilsMessenger, NULL);
    }

    vkDestroyInstance(pApplication->instance, NULL);

    if (pApplication->pWindow != NULL)
    {
        SDL_DestroyWindow(pApplication->pWindow);
    }

    SDL_Quit();
}

VKAPI_ATTR VkBool32 debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT         messageSeverity,
                                                VkDebugUtilsMessageTypeFlagsEXT                messageTypes,
                                                const VkDebugUtilsMessengerCallbackDataEXT*    pCallbackData,
                                                void*                                          pUserData)
{
    printf("%s\n", pCallbackData->pMessage);
    return VK_FALSE;
}
