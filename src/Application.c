#include "Application.h"

#include <stdlib.h>
#include <string.h>

static Result createWindow(Application* pApplication);

static Result getAvailableInstanceLayers(Application* pApplication);

static Result printAvailableInstanceLayers(Application* pApplication);

static Result getAvailableInstanceExtensions(Application* pApplication);

static void printAvailableInstanceExtensions(Application* pApplication);

static Result checkAvailabilityOfRequiredLayers(Application* pApplication);

static VKAPI_ATTR VkBool32 debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT         messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT*    pCallbackData,
    void*                                          pUserData);

static Result createInstance(Application* pApplication);

Result createApplication(Application* pApplication)
{
    pApplication->pWindow = NULL;
    pApplication->availableInstanceLayerCount = 0;
    pApplication->ppAvailableInstanceLayers = NULL;
    pApplication->availableInstanceExtensionCount = 0;
    pApplication->ppAvailableInstanceExtensions = NULL;
    pApplication->requiredInstanceLayerCount = 1;
    pApplication->ppRequiredInstanceLayers[0] = "VK_LAYER_KHRONOS_validation";
    pApplication->requiredInstanceExtensionCount = 0;
    pApplication->ppRequiredInstanceExtensions = NULL;
    pApplication->instance = NULL;

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

    if (getAvailableInstanceLayers(pApplication) != SUCCESS)
    {
        printError("Failed to get available instance layers!");
        destroyApplication(pApplication);
        return FAIL;
    }

    if (printAvailableInstanceLayers(pApplication) != SUCCESS)
    {
        printError("Failed to print available instance layers!");
        destroyApplication(pApplication);
        return FAIL;
    }

    if (getAvailableInstanceExtensions(pApplication) != SUCCESS)
    {
        printError("Failed to get available instance extensions!");
        destroyApplication(pApplication);
        return FAIL;
    }

    printAvailableInstanceExtensions(pApplication);

    if (createInstance(pApplication) != SUCCESS)
    {
        printError("Failed to create Vulkan instance!");
        destroyApplication(pApplication);
        return FAIL;
    }

    return SUCCESS;
}

Result createWindow(Application* pApplication)
{
    pApplication->pWindow = SDL_CreateWindow("Viewer", 100, 100, 1600, 900, SDL_WINDOW_VULKAN);
    return (pApplication->pWindow == NULL) ? FAIL : SUCCESS;
}

Result getAvailableInstanceLayers(Application* pApplication)
{
    vkEnumerateInstanceLayerProperties(&pApplication->availableInstanceLayerCount, NULL);

    uint32_t layerCount = pApplication->availableInstanceLayerCount;

    if (layerCount > 0)
    {
        VkLayerProperties* pProperties = malloc(layerCount * sizeof(VkLayerProperties));
        if (pProperties == NULL)
        {
            printError("Failed to allocate %lu bytes of memory for available instance layers properties!", layerCount * sizeof(VkLayerProperties));
            return FAIL;
        }

        vkEnumerateInstanceLayerProperties(&pApplication->availableInstanceLayerCount, pProperties);

        pApplication->ppAvailableInstanceLayers = calloc(layerCount, sizeof(char*));
        if (pApplication->ppAvailableInstanceLayers == NULL)
        {
            printError("Failed to allocate %lu bytes of memory for available instance layers names!", layerCount * sizeof(char*));
            free(pProperties);
            return FAIL;
        }

        char** ppLayers = pApplication->ppAvailableInstanceLayers;

        for (uint32_t i = 0; i < layerCount; ++i)
        {
            const char* pLayerName = pProperties[i].layerName;

            ppLayers[i] = malloc((strlen(pLayerName) + 1) * sizeof(char));
            if (ppLayers[i] == NULL)
            {
                printError("Failed to allocate %lu bytes of memory for name of layer \"%s\"!", (strlen(pLayerName) + 1) * sizeof(char), pLayerName);
                free(pProperties);
                return FAIL;
            }

            strlcpy(ppLayers[i], pLayerName, (strlen(pLayerName) + 1) * sizeof(char));
        }

        free(pProperties);
    }

    return SUCCESS;
}

static Result printAvailableInstanceLayers(Application* pApplication)
{
    uint32_t layerCount = pApplication->availableInstanceLayerCount;

    printf("Available layers[%u]:\n", layerCount);
    for (uint32_t i = 0; i < layerCount; ++i)
    {
        const char* pLayerName = pApplication->ppAvailableInstanceLayers[i];

        uint32_t extensionCount;
        vkEnumerateInstanceExtensionProperties(pLayerName, &extensionCount, NULL);

        if (extensionCount == 0)
        {
            printf("    %s\n", pLayerName);
        }
        else
        {
            VkExtensionProperties* pProperties = malloc(extensionCount * sizeof(VkExtensionProperties));
            if (pProperties == NULL)
            {
                printError("Failed to allocate %lu bytes of memory for extensions properties of layer \"%s\"!", extensionCount * sizeof(VkExtensionProperties), pLayerName);
                return FAIL;
            }

            vkEnumerateInstanceExtensionProperties(pLayerName, &extensionCount, pProperties);

            printf("    %s[%u]:\n", pLayerName, extensionCount);
            for (uint32_t j = 0; j < extensionCount; ++j)
            {
                printf("        %s\n", pProperties[j].extensionName);
            }

            free(pProperties);
        }
    }
    printf("\n");

    return SUCCESS;
}

static Result getAvailableInstanceExtensions(Application* pApplication)
{
    vkEnumerateInstanceExtensionProperties(NULL, &pApplication->availableInstanceExtensionCount, NULL);

    uint32_t extensionCount = pApplication->availableInstanceExtensionCount;

    if (extensionCount > 0)
    {
        VkExtensionProperties* pProperties = malloc(extensionCount * sizeof(VkExtensionProperties));
        if (pProperties == NULL)
        {
            printError("Failed to allocate %lu bytes of memory for available instance extensions properties!", extensionCount * sizeof(VkExtensionProperties));
            return FAIL;
        }

        vkEnumerateInstanceExtensionProperties(NULL, &pApplication->availableInstanceExtensionCount, pProperties);

        pApplication->ppAvailableInstanceExtensions = calloc(extensionCount, sizeof(char*));
        if (pApplication->ppAvailableInstanceExtensions == NULL)
        {
            printError("Failed to allocate %lu bytes of memory for available instance extensions names!", extensionCount * sizeof(char*));
            free(pProperties);
            return FAIL;
        }

        char** ppExtensions = pApplication->ppAvailableInstanceExtensions;

        for (uint32_t i = 0; i < extensionCount; ++i)
        {
            const char* pExtensionName = pProperties[i].extensionName;

            ppExtensions[i] = malloc((strlen(pExtensionName) + 1) * sizeof(char));
            if (ppExtensions[i] == NULL)
            {
                printError("Failed to allocate %lu bytes of memory for name of extension \"%s\"!", (strlen(pExtensionName) + 1) * sizeof(char), pExtensionName);
                free(pProperties);
                return FAIL;
            }

            strlcpy(ppExtensions[i], pExtensionName, (strlen(pExtensionName) + 1) * sizeof(char));
        }

        free(pProperties);
    }

    return SUCCESS;
}

static void printAvailableInstanceExtensions(Application* pApplication)
{
    printf("Available extension[%u]:\n", pApplication->availableInstanceExtensionCount);
    for (uint32_t i = 0; i < pApplication->availableInstanceExtensionCount; ++i)
    {
        printf("    %s\n", pApplication->ppAvailableInstanceExtensions[i]);
    }
    printf("\n");
}

static VKAPI_ATTR VkBool32 debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT         messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT*    pCallbackData,
    void*                                          pUserData)
{
    if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        printf("%s\n", pCallbackData->pMessage);
    }
    else
    {
        fprintf(stderr, "%s\n", pCallbackData->pMessage);
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

    // If the pEnabledValidationFeatures array contains VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
    // then it must also contain VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT

    // If the pEnabledValidationFeatures array contains VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
    // then it must not contain VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT

    const VkValidationFeatureEnableEXT pEnabledValidationFeatures[4] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
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

    // If the pNext chain of VkInstanceCreateInfo includes a VkDebugUtilsMessengerCreateInfoEXT structure,
    // the list of enabled extensions in ppEnabledExtensionNames must contain VK_EXT_debug_utils

    // If flags has the VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR bit set,
    // the list of enabled extensions in ppEnabledExtensionNames must contain VK_KHR_portability_enumeration

    // Each pNext member of any structure (including this one) in the pNext chain must be either NULL or a pointer to a valid
    // instance of VkDebugReportCallbackCreateInfoEXT, VkDebugUtilsMessengerCreateInfoEXT, VkDirectDriverLoadingListLUNARG,
    // VkExportMetalObjectCreateInfoEXT, VkLayerSettingsCreateInfoEXT, VkValidationFeaturesEXT, or VkValidationFlagsEXT

    // The sType value of each struct in the pNext chain must be unique, with the exception of structures of type
    // VkDebugUtilsMessengerCreateInfoEXT, VkExportMetalObjectCreateInfoEXT, or VkLayerSettingsCreateInfoEXT

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = &validationFeatures;
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = NULL;
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = NULL;

    int result = vkCreateInstance(&createInfo, NULL, &pApplication->instance);

    return (result == VK_SUCCESS) ? SUCCESS : FAIL;
}

void destroyApplication(Application* pApplication)
{
    vkDestroyInstance(pApplication->instance, NULL);

    if (pApplication->ppRequiredInstanceExtensions != NULL)
    {
        for (uint32_t i = 0; i < pApplication->requiredInstanceExtensionCount; ++i)
        {
            free(pApplication->ppRequiredInstanceExtensions[i]);
        }

        free(pApplication->ppRequiredInstanceExtensions);
    }

    if (pApplication->ppAvailableInstanceExtensions != NULL)
    {
        for (uint32_t i = 0; i < pApplication->availableInstanceExtensionCount; ++i)
        {
            free(pApplication->ppAvailableInstanceExtensions[i]);
        }

        free(pApplication->ppAvailableInstanceExtensions);
    }

    if (pApplication->ppAvailableInstanceLayers != NULL)
    {
        for (uint32_t i = 0; i < pApplication->availableInstanceLayerCount; ++i)
        {
            free(pApplication->ppAvailableInstanceLayers[i]);
        }

        free(pApplication->ppAvailableInstanceLayers);
    }

    if (pApplication->pWindow != NULL)
    {
        SDL_DestroyWindow(pApplication->pWindow);
    }

    SDL_Quit();
}
