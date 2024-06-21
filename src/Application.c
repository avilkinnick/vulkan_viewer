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

static Result createInstance(Application* pApplication);

static Result createDebugUtilsMessenger(Application* pApplication, VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo);

static Result getPhysicalDevice(Application* pApplication);

static Result createDevice(Application* pApplication);

Result createApplication(Application* pApplication)
{
    pApplication->pWindow = NULL;
    pApplication->instance = NULL;
    pApplication->debugUtilsMessenger = NULL;
    pApplication->physicalDevice = NULL;
    pApplication->device = NULL;

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
        printError("Failed to get Vulkan physical device!");
        destroyApplication(pApplication);
        return FAIL;
    }

    if (createDevice(pApplication) != SUCCESS)
    {
        printError("Failed to create Vulkan device!");
        destroyApplication(pApplication);
        return FAIL;
    }

    vkGetDeviceQueue(pApplication->device, 0, 0, &pApplication->queue);

    return SUCCESS;
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

    if (vkCreateInstance(&createInfo, NULL, &pApplication->instance) != VK_SUCCESS)
    {
        printError("Failed to create Vulkan instance!");
        freeInstanceExtensions(&availableExtensionCount, &ppAvailableExtensions, &requiredExtensionCount, &ppRequiredExtensions);
        freeInstanceLayers(&availableLayerCount, &ppAvailableLayers);
        return FAIL;
    }

    freeInstanceExtensions(&availableExtensionCount, &ppAvailableExtensions, &requiredExtensionCount, &ppRequiredExtensions);
    freeInstanceLayers(&availableLayerCount, &ppAvailableLayers);

    if (createDebugUtilsMessenger(pApplication, &debugUtilsMessengerCreateInfo) != SUCCESS)
    {
        printError("Failed to create Vulkan debug utils messenger!");
        return FAIL;
    }

    return SUCCESS;
}

Result createDebugUtilsMessenger(Application* pApplication, VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo)
{
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pApplication->instance, "vkCreateDebugUtilsMessengerEXT");
    int result = vkCreateDebugUtilsMessengerEXT(pApplication->instance, pCreateInfo, NULL, &pApplication->debugUtilsMessenger);
    return (result == VK_SUCCESS) ? SUCCESS : FAIL;
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
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = NULL;
    createInfo.pEnabledFeatures = &features;

    int result = vkCreateDevice(pApplication->physicalDevice, &createInfo, NULL, &pApplication->device);
    return (result == VK_SUCCESS) ? SUCCESS : FAIL;
}

void destroyApplication(Application* pApplication)
{
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
