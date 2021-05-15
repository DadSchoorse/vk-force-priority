#pragma once
#define VK_NO_PROTOTYPES

#include "vulkan/vulkan.h"
#include "vulkan/vk_layer.h"

#include <string>
#include <iostream>

#ifndef VKASSERT
#define VKASSERT(val) \
    if (val != VK_SUCCESS) \
    { \
        std::cerr << "VKASSERT failed in " + std::string(__func__) + " : " + std::to_string(__LINE__) + "; " + std::to_string(val) << std::endl; \
    }
#endif
