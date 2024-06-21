#include "layers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vulkan/vulkan.h>

#include <SDL.h>

Result getAvailableInstanceLayers(uint32_t* pLayerCount, char*** pppLayers)
{
    vkEnumerateInstanceLayerProperties(pLayerCount, NULL);

    if (*pLayerCount > 0)
    {
        VkLayerProperties* pProperties = malloc(*pLayerCount * sizeof(VkLayerProperties));
        if (pProperties == NULL)
        {
            printError("Failed to allocate %lu bytes of memory for available instance layers properties!", *pLayerCount * sizeof(VkLayerProperties));
            return FAIL;
        }

        vkEnumerateInstanceLayerProperties(pLayerCount, pProperties);

        *pppLayers = calloc(*pLayerCount, sizeof(char*));
        if (*pppLayers == NULL)
        {
            printError("Failed to allocate %lu bytes of memory for available instance layers names!", *pLayerCount * sizeof(char*));
            free(pProperties);
            return FAIL;
        }

        char** ppLayers = *pppLayers;

        for (uint32_t i = 0; i < *pLayerCount; ++i)
        {
            const char* pLayerName = pProperties[i].layerName;

            ppLayers[i] = malloc((strlen(pLayerName) + 1) * sizeof(char));
            if (ppLayers[i] == NULL)
            {
                printError("Failed to allocate %lu bytes of memory for name of instance layer \"%s\"!", (strlen(pLayerName) + 1) * sizeof(char), pLayerName);
                free(pProperties);
                return FAIL;
            }

            strlcpy(ppLayers[i], pLayerName, (strlen(pLayerName) + 1) * sizeof(char));
        }

        free(pProperties);
    }

    return SUCCESS;
}

Result printAvailableInstanceLayers(uint32_t layerCount, char** ppLayers)
{
    printf("Available instance layers[%u]:\n", layerCount);
    for (uint32_t i = 0; i < layerCount; ++i)
    {
        uint32_t extensionCount;
        vkEnumerateInstanceExtensionProperties(ppLayers[i], &extensionCount, NULL);

        if (extensionCount == 0)
        {
            printf("    %s\n", ppLayers[i]);
        }
        else
        {
            VkExtensionProperties* pProperties = malloc(extensionCount * sizeof(VkExtensionProperties));
            if (pProperties == NULL)
            {
                printError("Failed to allocate %lu bytes of memory for extensions properties of instance layer \"%s\"!", extensionCount * sizeof(VkExtensionProperties), ppLayers[i]);
                return FAIL;
            }

            vkEnumerateInstanceExtensionProperties(ppLayers[i], &extensionCount, pProperties);

            printf("    %s[%u]:\n", ppLayers[i], extensionCount);
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

Result checkAvailabilityOfRequiredInstanceLayers(uint32_t availableLayerCount, char** ppAvailableLayers, uint32_t requiredLayerCount, const char** ppRequiredLayers)
{
    for (uint32_t i = 0; i < requiredLayerCount; ++i)
    {
        SDL_bool found = SDL_FALSE;
        for (uint32_t j = 0; j < availableLayerCount; ++j)
        {
            if (strcmp(ppRequiredLayers[i], ppAvailableLayers[j]) == 0)
            {
                found = SDL_TRUE;
                break;
            }
        }

        if (found != SDL_TRUE)
        {
            printError("Instance layer %s is not available!", ppRequiredLayers[i]);
            return FAIL;
        }
    }

    return SUCCESS;
}

void freeInstanceLayers(uint32_t* pAvailableLayerCount, char*** pppAvailableLayers)
{
    for (uint32_t i = 0; i < *pAvailableLayerCount; ++i)
    {
        free((*pppAvailableLayers)[i]);
    }

    free(*pppAvailableLayers);
    *pppAvailableLayers = NULL;

    *pAvailableLayerCount = 0;
}
