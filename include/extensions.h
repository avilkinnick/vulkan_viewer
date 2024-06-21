#ifndef EXTENSIONS_H
#define EXTENSIONS_H

#include <stdint.h>

#include <vulkan/vulkan.h>

#include <SDL.h>
#include <SDL_vulkan.h>

#include "base.h"

Result getAvailableInstanceExtensions(uint32_t* pExtensionCount, char*** pppExtensions);

void printAvailableInstanceExtensions(uint32_t extensionCount, char** ppExtensions);

Result getRequiredInstanceExtensions(SDL_Window* pWindow, unsigned int* pExtensionCount, const char*** pppExtensions);

void printRequiredInstanceExtensions(unsigned int extensionCount, const char** ppExtensions);

void freeInstanceExtensions(uint32_t* pAvailableExtensionCount, char*** pppAvailableExtensions, unsigned int* pRequiredExtensionCount, const char*** pppRequiredExtensions);

Result getAvailableDeviceExtensions(VkPhysicalDevice physicalDevice, uint32_t* pExtensionCount, char*** pppExtensions);

void printAvailableDeviceExtensions(uint32_t extensionCount, char** ppExtensions);

void freeDeviceExtensions(uint32_t* pAvailableExtensionCount, char*** pppAvailableExtensions);

#endif // EXTENSIONS_H
