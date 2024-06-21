#ifndef EXTENSIONS_H
#define EXTENSIONS_H

#include <stdint.h>

#include <SDL.h>

#include "base.h"

Result getAvailableInstanceExtensions(uint32_t* pExtensionCount, char*** pppExtensions);

void printAvailableInstanceExtensions(uint32_t extensionCount, char** ppExtensions);

Result getRequiredInstanceExtensions(SDL_Window* pWindow, unsigned int* pExtensionCount, const char*** pppExtensions);

void printRequiredInstanceExtensions(unsigned int extensionCount, const char** ppExtensions);

void freeInstanceExtensions(uint32_t* pAvailableExtensionCount, char*** pppAvailableExtensions, unsigned int* pRequiredExtensionCount, const char*** pppRequiredExtensions);

#endif // EXTENSIONS_H
