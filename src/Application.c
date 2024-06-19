#include "Application.h"

Result createApplication(Application* pApplication)
{
    pApplication->pWindow = NULL;
    pApplication->instance = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printError("Failed to initialize SDL library!");
        return FAIL;
    }

    if (createWindow(&pApplication->pWindow) != SUCCESS)
    {
        printError("Failed to create window!");
        destroyApplication(pApplication);
        return FAIL;
    }

    if (createInstance(&pApplication->instance) != SUCCESS)
    {
        printError("Failed to create Vulkan instance!");
        destroyApplication(pApplication);
        return FAIL;
    }

    return SUCCESS;
}

Result createWindow(SDL_Window** ppWindow)
{
    *ppWindow = SDL_CreateWindow("Viewer", 100, 100, 1600, 900, SDL_WINDOW_VULKAN);
    return (*ppWindow == NULL) ? FAIL : SUCCESS;
}

Result createInstance(VkInstance* pInstance)
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

    uint32_t availableLayerCount;
    vkEnumerateInstanceLayerProperties(&availableLayerCount, NULL);

    VkLayerProperties* pAvailableLayersProperties = malloc(availableLayerCount * sizeof(VkLayerProperties));
    if (pAvailableLayersProperties == NULL)
    {
        printError("Failed to allocate %lu bytes of memory for available layers properties!", availableLayerCount * sizeof(VkLayerProperties));
        return FAIL;
    }

    vkEnumerateInstanceLayerProperties(&availableLayerCount, pAvailableLayersProperties);

    printf("Available layers:\n");
    for (uint32_t i = 0; i < availableLayerCount; ++i)
    {
        VkLayerProperties* pProperties = &pAvailableLayersProperties[i];
        uint32_t variant = VK_API_VERSION_VARIANT(pProperties->specVersion);
        uint32_t major = VK_API_VERSION_MAJOR(pProperties->specVersion);
        uint32_t minor = VK_API_VERSION_MINOR(pProperties->specVersion);
        uint32_t patch = VK_API_VERSION_PATCH(pProperties->specVersion);

        printf("Layer %u:\n", i);
        printf("    layerName: %s\n", pProperties->layerName);
        printf("    specVersion: %u.%u.%u.%u\n", variant, major, minor, patch);
        printf("    implementationVersion: %u\n", pProperties->implementationVersion);
        printf("    description: %s\n", pProperties->description);
        printf("\n");
    }

    const char** ppAvailableLayers = malloc(availableLayerCount * sizeof(const char*));
    if (ppAvailableLayers == NULL)
    {
        printError("Failed to allocate %lu bytes of memory for available layers!", availableLayerCount * sizeof(const char*));
        free(pAvailableLayersProperties);
        return FAIL;
    }

    // If the pNext chain of VkInstanceCreateInfo includes a VkDebugUtilsMessengerCreateInfoEXT structure,
    // the list of enabled extensions in ppEnabledExtensionNames must contain VK_EXT_debug_utils

    // If flags has the VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR bit set,
    // the list of enabled extensions in ppEnabledExtensionNames must contain VK_KHR_portability_enumeration

    // Each pNext member of any structure (including this one) in the pNext chain must be either NULL or a pointer to a valid
    // instance of VkDebugReportCallbackCreateInfoEXT, VkDebugUtilsMessengerCreateInfoEXT, VkDirectDriverLoadingListLUNARG,
    // VkExportMetalObjectCreateInfoEXT, VkLayerSettingsCreateInfoEXT, VkValidationFeaturesEXT, or VkValidationFlagsEXT

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    // createInfo.pNext;
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    createInfo.pApplicationInfo = &applicationInfo;
    // createInfo.enabledLayerCount;
    // createInfo.ppEnabledLayerNames;
    // createInfo.enabledExtensionCount;
    // createInfo.ppEnabledExtensionNames;

    // int result = vkCreateInstance(&createInfo, NULL, pInstance);
    int result = SUCCESS;

    free(pAvailableLayersProperties);

    return (result == VK_SUCCESS) ? SUCCESS : FAIL;
}

void destroyApplication(Application* pApplication)
{
    vkDestroyInstance(pApplication->instance, NULL);

    if (pApplication->pWindow != NULL)
    {
        SDL_DestroyWindow(pApplication->pWindow);
    }

    SDL_Quit();
}
