#ifndef LAYERS_H
#define LAYERS_H

#include <stdint.h>

#include "base.h"

Result getAvailableInstanceLayers(uint32_t* pLayerCount, char*** pppLayers);

Result printAvailableInstanceLayers(uint32_t layerCount, char** ppLayers);

Result checkAvailabilityOfRequiredInstanceLayers(uint32_t availableLayerCount, char** ppAvailableLayers, uint32_t requiredLayerCount, const char** ppRequiredLayers);

void freeInstanceLayers(uint32_t* pAvailableLayerCount, char*** pppAvailableLayers);

#endif // LAYERS_H
