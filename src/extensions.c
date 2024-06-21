#include "extensions.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vulkan/vulkan.h>

#include <SDL.h>
#include <SDL_vulkan.h>

Result getAvailableInstanceExtensions(uint32_t* pExtensionCount, char*** pppExtensions)
{
    vkEnumerateInstanceExtensionProperties(NULL, pExtensionCount, NULL);

    if (*pExtensionCount > 0)
    {
        VkExtensionProperties* pProperties = malloc(*pExtensionCount * sizeof(VkExtensionProperties));
        if (pProperties == NULL)
        {
            printError("Failed to allocate %lu bytes of memory for available instance extensions properties!", *pExtensionCount * sizeof(VkExtensionProperties));
            return FAIL;
        }

        vkEnumerateInstanceExtensionProperties(NULL, pExtensionCount, pProperties);

        *pppExtensions = calloc(*pExtensionCount, sizeof(char*));
        if (*pppExtensions == NULL)
        {
            printError("Failed to allocate %lu bytes of memory for available instance extensions names!", *pExtensionCount * sizeof(char*));
            free(pProperties);
            return FAIL;
        }

        char** ppExtensions = *pppExtensions;

        for (uint32_t i = 0; i < *pExtensionCount; ++i)
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

void printAvailableInstanceExtensions(uint32_t extensionCount, char** ppExtensions)
{
    printf("Available extension[%u]:\n", extensionCount);
    for (uint32_t i = 0; i < extensionCount; ++i)
    {
        printf("    %s\n", ppExtensions[i]);
    }
    printf("\n");
}

Result getRequiredInstanceExtensions(SDL_Window* pWindow, unsigned int* pExtensionCount, const char*** pppExtensions)
{
    SDL_Vulkan_GetInstanceExtensions(pWindow, pExtensionCount, NULL);

    *pppExtensions = calloc(*pExtensionCount + 3, sizeof(char*));
    if (*pppExtensions == NULL)
    {
        printError("Failed to allocate %lu bytes of memory for required instance extensions names!", *pExtensionCount * sizeof(char*));
        return FAIL;
    }

    const char** ppExtensions = *pppExtensions;

    SDL_Vulkan_GetInstanceExtensions(pWindow, pExtensionCount, *pppExtensions);

    ppExtensions[(*pExtensionCount)++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    ppExtensions[(*pExtensionCount)++] = VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME;
    ppExtensions[(*pExtensionCount)++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;

    return SUCCESS;
}

void printRequiredInstanceExtensions(unsigned int extensionCount, const char** ppExtensions)
{
    printf("Required extension[%u]:\n", extensionCount);
    for (unsigned int i = 0; i < extensionCount; ++i)
    {
        printf("    %s\n", ppExtensions[i]);
    }
    printf("\n");
}

void freeInstanceExtensions(uint32_t* pAvailableExtensionCount, char*** pppAvailableExtensions, unsigned int* pRequiredExtensionCount, const char*** pppRequiredExtensions)
{
    free(*pppRequiredExtensions);
    *pppRequiredExtensions = NULL;

    *pRequiredExtensionCount = 0;

    for (uint32_t i = 0; i < *pAvailableExtensionCount; ++i)
    {
        free((*pppAvailableExtensions)[i]);
    }

    free(*pppAvailableExtensions);
    *pppAvailableExtensions = NULL;

    *pAvailableExtensionCount = 0;
}
