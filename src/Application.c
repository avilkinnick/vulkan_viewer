#include "Application.h"

#include <stdlib.h>
#include <string.h>

#include "extensions.h"
#include "layers.h"

static Result createWindow(Application* pApplication);

static VKAPI_ATTR VkBool32 debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT         messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT*    pCallbackData,
    void*                                          pUserData);

static Result createDebugUtilsMessenger(Application* pApplication, VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo);

static Result createInstance(Application* pApplication);

static Result getPhysicalDevice(Application* pApplication);

static Result createDevice(Application* pApplication);

static Result createSurface(Application* pApplication);

static Result createSwapchain(Application* pApplication);

static Result getSwapchainImages(Application* pApplication);

static Result createSwapchainImageViews(Application* pApplication);

static Result createVertShaderModule(Application* pApplication, VkShaderModule* pModule);

static Result createFragShaderModule(Application* pApplication, VkShaderModule* pModule);

static Result createShaderStages(Application* pApplication, VkPipelineShaderStageCreateInfo* pStages);

static Result createGraphicsPipeline(Application* pApplication);

Result createApplication(Application* pApplication)
{
    pApplication->pWindow = NULL;
    pApplication->instance = NULL;
    pApplication->debugUtilsMessenger = NULL;
    pApplication->physicalDevice = NULL;
    pApplication->device = NULL;
    pApplication->surface = NULL;
    pApplication->swapchain = NULL;
    pApplication->pSwapchainImages = NULL;
    pApplication->pSwapchainImageViews = NULL;
    pApplication->pipeline = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printError("Failed to initialize SDL library!");
        return FAIL;
    }

    if (createWindow(pApplication) != SUCCESS)
    {
        printError("Failed to create window!");
        destroyApplication(pApplication);
        return FAIL;
    }

    if (createInstance(pApplication) != SUCCESS)
    {
        destroyApplication(pApplication);
        return FAIL;
    }

    if (getPhysicalDevice(pApplication) != SUCCESS)
    {
        printError("Failed to get physical device!");
        destroyApplication(pApplication);
        return FAIL;
    }

    if (createDevice(pApplication) != SUCCESS)
    {
        printError("Failed to create device!");
        destroyApplication(pApplication);
        return FAIL;
    }

    vkGetDeviceQueue(pApplication->device, 0, 0, &pApplication->queue);

    if (createSurface(pApplication) != SUCCESS)
    {
        printError("Failed to create surface!");
        destroyApplication(pApplication);
        return FAIL;
    }

    if (createSwapchain(pApplication) != SUCCESS)
    {
        printError("Failed to create swapchain!");
        destroyApplication(pApplication);
        return FAIL;
    }

    if (getSwapchainImages(pApplication) != SUCCESS)
    {
        printError("Failed to get swapchain images!");
        destroyApplication(pApplication);
        return FAIL;
    }

    if (createSwapchainImageViews(pApplication) != SUCCESS)
    {
        printError("Failed to create swapchain image views!");
        destroyApplication(pApplication);
        return FAIL;
    }

    if (createGraphicsPipeline(pApplication) != SUCCESS)
    {
        printError("Failed to create graphics pipeline!");
        destroyApplication(pApplication);
        return FAIL;
    }

    return SUCCESS;
}

void destroyApplication(Application* pApplication)
{
    vkDestroyPipeline(pApplication->device, pApplication->pipeline, NULL);

    if (pApplication->pSwapchainImageViews != NULL)
    {
        for (uint32_t i = 0; i < pApplication->swapchainImageCount; ++i)
        {
            vkDestroyImageView(pApplication->device, pApplication->pSwapchainImageViews[i], NULL);
        }
    }

    free(pApplication->pSwapchainImageViews);

    free(pApplication->pSwapchainImages);

    vkDestroySwapchainKHR(pApplication->device, pApplication->swapchain, NULL);

    vkDestroySurfaceKHR(pApplication->instance, pApplication->surface, NULL);

    vkDestroyDevice(pApplication->device, NULL);

    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pApplication->instance, "vkDestroyDebugUtilsMessengerEXT");
    vkDestroyDebugUtilsMessengerEXT(pApplication->instance, pApplication->debugUtilsMessenger, NULL);

    vkDestroyInstance(pApplication->instance, NULL);

    if (pApplication->pWindow != NULL)
    {
        SDL_DestroyWindow(pApplication->pWindow);
    }

    SDL_Quit();
}

Result createWindow(Application* pApplication)
{
    pApplication->pWindow = SDL_CreateWindow("Viewer", 100, 100, 1600, 900, SDL_WINDOW_VULKAN);
    return (pApplication->pWindow == NULL) ? FAIL : SUCCESS;
}

VKAPI_ATTR VkBool32 debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT         messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT*    pCallbackData,
    void*                                          pUserData)
{
    if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        printf("%s\n", pCallbackData->pMessage);
        printf("\n");
    }
    else
    {
        fprintf(stderr, "%s\n", pCallbackData->pMessage);
        fprintf(stderr, "\n");
    }

    return VK_FALSE;
}

Result createDebugUtilsMessenger(Application* pApplication, VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo)
{
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pApplication->instance, "vkCreateDebugUtilsMessengerEXT");
    int result = vkCreateDebugUtilsMessengerEXT(pApplication->instance, pCreateInfo, NULL, &pApplication->debugUtilsMessenger);
    return (result == VK_SUCCESS) ? SUCCESS : FAIL;
}

Result createInstance(Application* pApplication)
{
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

    VkValidationFeatureEnableEXT pEnabledValidationFeatures[4] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                                                                  VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
                                                                  VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
                                                                  VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};

    VkValidationFeaturesEXT validationFeatures;
    validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validationFeatures.pNext = &debugUtilsMessengerCreateInfo;
    validationFeatures.enabledValidationFeatureCount = 4;
    validationFeatures.pEnabledValidationFeatures = pEnabledValidationFeatures;
    validationFeatures.disabledValidationFeatureCount = 0;
    validationFeatures.pDisabledValidationFeatures = NULL;

    uint32_t        availableLayerCount = 0;
    char**          ppAvailableLayers = NULL;
    uint32_t        availableExtensionCount = 0;
    char**          ppAvailableExtensions = NULL;
    uint32_t        requiredLayerCount = 1;
    const char*     ppRequiredLayers[1] = {"VK_LAYER_KHRONOS_validation"};
    unsigned int    requiredExtensionCount = 0;
    const char**    ppRequiredExtensions = NULL;

    if (getAvailableInstanceLayers(&availableLayerCount, &ppAvailableLayers) != SUCCESS)
    {
        printError("Failed to get available instance layers!");
        freeInstanceLayers(&availableLayerCount, &ppAvailableLayers);
        return FAIL;
    }

    if (printAvailableInstanceLayers(availableLayerCount, ppAvailableLayers) != SUCCESS)
    {
        printError("Failed to print available instance layers!");
        freeInstanceLayers(&availableLayerCount, &ppAvailableLayers);
        return FAIL;
    }

    if (checkAvailabilityOfRequiredInstanceLayers(availableLayerCount, ppAvailableLayers, requiredLayerCount, ppRequiredLayers) != SUCCESS)
    {
        printError("Some of required layers are not available!");
        freeInstanceLayers(&availableLayerCount, &ppAvailableLayers);
        return FAIL;
    }

    if (getAvailableInstanceExtensions(&availableExtensionCount, &ppAvailableExtensions) != SUCCESS)
    {
        printError("Failed to get available instance extensions!");
        freeInstanceExtensions(&availableExtensionCount, &ppAvailableExtensions, &requiredExtensionCount, &ppRequiredExtensions);
        freeInstanceLayers(&availableLayerCount, &ppAvailableLayers);
        return FAIL;
    }

    printAvailableInstanceExtensions(availableExtensionCount, ppAvailableExtensions);

    if (getRequiredInstanceExtensions(pApplication->pWindow, &requiredExtensionCount, &ppRequiredExtensions) != SUCCESS)
    {
        printError("Failed to get required extensions!");
        freeInstanceExtensions(&availableExtensionCount, &ppAvailableExtensions, &requiredExtensionCount, &ppRequiredExtensions);
        freeInstanceLayers(&availableLayerCount, &ppAvailableLayers);
        return FAIL;
    }

    printRequiredInstanceExtensions(requiredExtensionCount, ppRequiredExtensions);

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = &validationFeatures;
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledLayerCount = requiredLayerCount;
    createInfo.ppEnabledLayerNames = ppRequiredLayers;
    createInfo.enabledExtensionCount = requiredExtensionCount;
    createInfo.ppEnabledExtensionNames = ppRequiredExtensions;

    int result = vkCreateInstance(&createInfo, NULL, &pApplication->instance);

    freeInstanceExtensions(&availableExtensionCount, &ppAvailableExtensions, &requiredExtensionCount, &ppRequiredExtensions);
    freeInstanceLayers(&availableLayerCount, &ppAvailableLayers);

    if (result != VK_SUCCESS)
    {
        printError("Failed to create instance!");
        return FAIL;
    }

    if (createDebugUtilsMessenger(pApplication, &debugUtilsMessengerCreateInfo) != SUCCESS)
    {
        printError("Failed to create debug utils messenger!");
        return FAIL;
    }

    return SUCCESS;
}

Result getPhysicalDevice(Application* pApplication)
{
    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(pApplication->instance, &physicalDeviceCount, NULL);

    if (physicalDeviceCount < 1)
    {
        printError("There are no physical devices!");
        return FAIL;
    }

    VkPhysicalDevice* pPhysicalDevices = malloc(physicalDeviceCount * sizeof(VkPhysicalDevice));
    if (pPhysicalDevices == NULL)
    {
        printError("Failed to allocate %lu bytes of memory for physical devices!", physicalDeviceCount * sizeof(VkPhysicalDevice));
        return FAIL;
    }

    vkEnumeratePhysicalDevices(pApplication->instance, &physicalDeviceCount, pPhysicalDevices);

    // TODO: change later
    pApplication->physicalDevice = pPhysicalDevices[0];

    free(pPhysicalDevices);

    return SUCCESS;
}

Result createDevice(Application* pApplication)
{
    float priority = 1.0f;

    // TODO: change later
    VkDeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = NULL;
    queueCreateInfo.flags = 0;
    queueCreateInfo.queueFamilyIndex = 0;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &priority;

    uint32_t       availableExtensionCount;
    char**         ppAvailableExtensions;
    uint32_t       requiredExtensionCount = 1;
    const char*    ppRequiredExtensions[1] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    if (getAvailableDeviceExtensions(pApplication->physicalDevice, &availableExtensionCount, &ppAvailableExtensions) != SUCCESS)
    {
        printError("Failed to get available device extensions!");
        freeDeviceExtensions(&availableExtensionCount, &ppAvailableExtensions);
        return FAIL;
    }

    // printAvailableDeviceExtensions(availableExtensionCount, ppAvailableExtensions);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(pApplication->physicalDevice, &features);

    VkDeviceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = NULL;
    createInfo.enabledExtensionCount = requiredExtensionCount;
    createInfo.ppEnabledExtensionNames = ppRequiredExtensions;
    createInfo.pEnabledFeatures = &features;

    int result = vkCreateDevice(pApplication->physicalDevice, &createInfo, NULL, &pApplication->device);

    freeDeviceExtensions(&availableExtensionCount, &ppAvailableExtensions);

    return (result == VK_SUCCESS) ? SUCCESS : FAIL;
}

Result createSurface(Application* pApplication)
{
    SDL_bool result = SDL_Vulkan_CreateSurface(pApplication->pWindow, pApplication->instance, &pApplication->surface);
    return (result == SDL_TRUE) ? SUCCESS : FAIL;
}

Result createSwapchain(Application* pApplication)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pApplication->physicalDevice, pApplication->surface, &surfaceCapabilities);

    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    if ((surfaceCapabilities.maxImageCount > 0) && (imageCount > surfaceCapabilities.maxImageCount))
    {
        imageCount = surfaceCapabilities.maxImageCount;
    }

    uint32_t surfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(pApplication->physicalDevice, pApplication->surface, &surfaceFormatCount, NULL);

    VkSurfaceFormatKHR* pSurfaceFormats = malloc(surfaceFormatCount * sizeof(VkSurfaceFormatKHR));
    if (pSurfaceFormats == NULL)
    {
        printError("Failed to allocate %lu bytes of memory for surface formats!", surfaceFormatCount * sizeof(VkSurfaceFormatKHR));
        return FAIL;
    }

    vkGetPhysicalDeviceSurfaceFormatsKHR(pApplication->physicalDevice, pApplication->surface, &surfaceFormatCount, pSurfaceFormats);

    VkSurfaceFormatKHR surfaceFormat = pSurfaceFormats[0];
    for (uint32_t i = 0; i < surfaceFormatCount; ++i)
    {
        if ((pSurfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB) && (pSurfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
        {
            surfaceFormat = pSurfaceFormats[i];
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
        int width, height;
        SDL_Vulkan_GetDrawableSize(pApplication->pWindow, &width, &height);

        extent.width = width;
        extent.height = height;

        if (width < surfaceCapabilities.minImageExtent.width)
        {
            extent.width = surfaceCapabilities.minImageExtent.width;
        }
        else if (width > surfaceCapabilities.maxImageExtent.width)
        {
            extent.width = surfaceCapabilities.maxImageExtent.width;
        }

        if (height < surfaceCapabilities.minImageExtent.height)
        {
            extent.height = surfaceCapabilities.minImageExtent.height;
        }
        else if (height > surfaceCapabilities.maxImageExtent.height)
        {
            extent.height = surfaceCapabilities.maxImageExtent.height;
        }
    }

    uint32_t queueFamilyIndex = 0;

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pApplication->physicalDevice, pApplication->surface, &presentModeCount, NULL);

    VkPresentModeKHR* pPresentModes = malloc(presentModeCount * sizeof(VkPresentModeKHR));
    if (pPresentModes == NULL)
    {
        printError("Failed to allocate %lu bytes of memory for present modes!");
        return FAIL;
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(pApplication->physicalDevice, pApplication->surface, &presentModeCount, pPresentModes);

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

    VkSwapchainCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.surface = pApplication->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 1;
    createInfo.pQueueFamilyIndices = &queueFamilyIndex;
    createInfo.preTransform = surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    int result = vkCreateSwapchainKHR(pApplication->device, &createInfo, NULL, &pApplication->swapchain);

    pApplication->swapchainImageFormat = surfaceFormat.format;
    pApplication->swapchainExtent = extent;

    return (result == VK_SUCCESS) ? SUCCESS : FAIL;
}

Result getSwapchainImages(Application* pApplication)
{
    vkGetSwapchainImagesKHR(pApplication->device, pApplication->swapchain, &pApplication->swapchainImageCount, NULL);

    pApplication->pSwapchainImages = malloc(pApplication->swapchainImageCount * sizeof(VkImage));
    if (pApplication->pSwapchainImages == NULL)
    {
        printError("Failed to allocate %lu bytes of memory for swapchain images!", pApplication->swapchainImageCount * sizeof(VkImage));
        return FAIL;
    }

    vkGetSwapchainImagesKHR(pApplication->device, pApplication->swapchain, &pApplication->swapchainImageCount, pApplication->pSwapchainImages);

    return SUCCESS;
}

Result createSwapchainImageViews(Application* pApplication)
{
    pApplication->pSwapchainImageViews = malloc(pApplication->swapchainImageCount * sizeof(VkImageView));
    if (pApplication->pSwapchainImageViews == NULL)
    {
        printError("Failed to allocate %lu bytes of memory for swapchain image views!", pApplication->swapchainImageCount * sizeof(VkImageView));
        return FAIL;
    }

    VkImage*        pImages = pApplication->pSwapchainImages;
    VkImageView*    pImageViews = pApplication->pSwapchainImageViews;

    for (uint32_t i = 0; i < pApplication->swapchainImageCount; ++i)
    {
        VkImageViewCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = NULL;
        createInfo.flags = 0;
        createInfo.image = pImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = pApplication->swapchainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(pApplication->device, &createInfo, NULL, &pImageViews[i]) != VK_SUCCESS)
        {
            printError("Failed to create swapchain image view %u!", i);
            return FAIL;
        }
    }

    return SUCCESS;
}

Result createVertShaderModule(Application* pApplication, VkShaderModule* pModule)
{

}

Result createFragShaderModule(Application* pApplication, VkShaderModule* pModule)
{

}

static Result createShaderStages(Application* pApplication, VkPipelineShaderStageCreateInfo* pStages)
{
    VkShaderModule vertShaderModule;

    pStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pStages[0].pNext = NULL;
    pStages[0].flags = 0;
    pStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    pStages[0].module = vertShaderModule;
    pStages[0].pName = "main";
    pStages[0].pSpecializationInfo = NULL;

    VkShaderModule fragShaderModule;

    pStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pStages[1].pNext = NULL;
    pStages[1].flags = 0;
    pStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    pStages[1].module = fragShaderModule;
    pStages[1].pName = "main";
    pStages[1].pSpecializationInfo = NULL;

    return SUCCESS;
}

Result createGraphicsPipeline(Application* pApplication)
{
    // Сначала создаем 2 VkShaderModule
    // Из них создаем 2 VkPipelineShaderStage
    // Потом создаем VkPipelineDynamicState
    // Потом создаем VkPipelineVertexInputState
    // Потом создаем VkPipelineInputAssemblyStateCreateInfo
    // Потом создаем VkPipelineViewportState
    // Потом создаем VkPipelineRasterizationState
    // Потом создаем VkPipelineMultisampleState
    // Потом создаем VkPipelineColorBlendAttachmentState
    // Потом создаем VkPipelineColorBlendState
    // Потом создаем VkPipelineLayout
    // Потом создаем VkAttachmentDescription
    // Потом создаем VkAttachmentReference
    // Потом создаем VkSubpassDescription
    // Потом создаем VkRenderPass
    // Потом создаем VkPipelineLayout
    // Потом создаем VkGraphicsPipeline

    // FILE* pFile = fopen("../shaders/vert.spv", "rb");
    // if (pFile == NULL)
    // {
    //     printError("Failed to open file \"%s\" for reading!");
    //     return FAIL;
    // }

    // fseek(pFile, 0, SEEK_END);

    // size_t codeSize = ftell(pFile);

    // rewind(pFile);

    // char* pCode = malloc(codeSize);
    // fread(pCode, 1, codeSize, pFile);

    // fclose(pFile);

    // VkShaderModuleCreateInfo vertCreateInfo;
    // vertCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    // vertCreateInfo.pNext = NULL;
    // vertCreateInfo.flags = 0;
    // vertCreateInfo.codeSize = codeSize;
    // vertCreateInfo.pCode = (const uint32_t*)pCode;

    // VkShaderModule vertShaderModule;
    // if (vkCreateShaderModule(pApplication->device, &vertCreateInfo, NULL, &vertShaderModule) != VK_SUCCESS)
    // {
    //     printError("Failed to create vert shader module!");
    //     return FAIL;
    // }

    VkPipelineShaderStageCreateInfo pStages[2];

    VkGraphicsPipelineCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.stageCount = 2;
    createInfo.pStages;
    // createInfo.pVertexInputState;
    // createInfo.pInputAssemblyState;
    // createInfo.pTessellationState;
    // createInfo.pViewportState;
    // createInfo.pRasterizationState;
    // createInfo.pMultisampleState;
    // createInfo.pDepthStencilState;
    // createInfo.pColorBlendState;
    // createInfo.pDynamicState;
    // createInfo.layout;
    // createInfo.renderPass;
    // createInfo.subpass;
    // createInfo.basePipelineHandle;
    // createInfo.basePipelineIndex;

    int result = vkCreateGraphicsPipelines(pApplication->device, VK_NULL_HANDLE, 1, &createInfo, NULL, &pApplication->pipeline);
    return (result == VK_SUCCESS) ? SUCCESS : FAIL;
}
