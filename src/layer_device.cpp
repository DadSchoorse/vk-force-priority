#include "layer_device.hpp"
#include "layer_instance.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>


#include "vkerror.hpp"

namespace layer
{
#define vk_find_struct(s, t) vk_find_struct_((void* )s, VK_STRUCTURE_TYPE_##t)
    static void* vk_find_struct_(void* s, VkStructureType t)
    {
        VkBaseOutStructure* header;

        for (header = (VkBaseOutStructure*) s; header; header = header->pNext)
        {
            if (header->sType == t)
                return header;
        }

        return nullptr;
    }


    static VkQueueGlobalPriorityEXT get_user_prio()
    {
        static VkQueueGlobalPriorityEXT s_result = (VkQueueGlobalPriorityEXT) 0;

        if (s_result)
            return s_result;

        const char* prio = std::getenv("VK_PRIORITY");

        static const std::unordered_map<std::string, VkQueueGlobalPriorityEXT> s_prios = {
            {"low", VK_QUEUE_GLOBAL_PRIORITY_LOW_EXT},
            {"medium", VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_EXT},
            {"high", VK_QUEUE_GLOBAL_PRIORITY_HIGH_EXT},
            {"realtime", VK_QUEUE_GLOBAL_PRIORITY_REALTIME_EXT},
        };


        if (prio)
        {
            auto search = s_prios.find(prio);
            if (search != s_prios.end())
            {
                s_result = search->second;
            }
            else
            {
                std::cerr << "invalid VK_PRIORITY: " << prio << std::endl;
            }
        }
        else
        {
            std::cerr << "VK_PRIORITY not set, using medium" << std::endl;
        }

        if (!s_result)
            s_result = VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_EXT;

        return s_result;
    }

    LayerDevice::LayerDevice(LayerInstance*               instance,
                             VkPhysicalDevice             phys_dev,
                             const VkDeviceCreateInfo*    pCreateInfo,
                             const VkAllocationCallbacks* pAllocator,
                             VkDevice*                    pDevice)
        : m_instance(instance), m_phys_dev(phys_dev)
    {
        VkLayerDeviceCreateInfo* layerCreateInfo = (VkLayerDeviceCreateInfo*) pCreateInfo->pNext;

        // step through the chain of pNext until we get to the link info
        while (layerCreateInfo
               && (layerCreateInfo->sType != VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO || layerCreateInfo->function != VK_LAYER_LINK_INFO))
        {
            layerCreateInfo = (VkLayerDeviceCreateInfo*) layerCreateInfo->pNext;
        }

        if (layerCreateInfo == nullptr)
            throw VkError(VK_ERROR_INITIALIZATION_FAILED);

        PFN_vkGetInstanceProcAddr gipa = layerCreateInfo->u.pLayerInfo->pfnNextGetInstanceProcAddr;
        PFN_vkGetDeviceProcAddr   gdpa = layerCreateInfo->u.pLayerInfo->pfnNextGetDeviceProcAddr;
        // move chain on for next layer
        layerCreateInfo->u.pLayerInfo = layerCreateInfo->u.pLayerInfo->pNext;

        PFN_vkCreateDevice createFunc = (PFN_vkCreateDevice) gipa(m_instance->instance(), "vkCreateDevice");

        VkDeviceCreateInfo our_info = *pCreateInfo;

        uint32_t ext_count = 0;

        instance->vk().EnumerateDeviceExtensionProperties(phys_dev, nullptr, &ext_count, nullptr);
        std::vector<VkExtensionProperties> available_exts(ext_count);
        instance->vk().EnumerateDeviceExtensionProperties(phys_dev, nullptr, &ext_count, available_exts.data());

        bool supports_prio = false;
        for (VkExtensionProperties properties : available_exts)
        {
            if (properties.extensionName == std::string("VK_EXT_global_priority"))
            {
                supports_prio = true;
                break;
            }
        }

        std::vector<const char*> enabled_exts;
        if (pCreateInfo->enabledExtensionCount)
        {
            enabled_exts = std::vector<const char*>(pCreateInfo->ppEnabledExtensionNames,
                                                    pCreateInfo->ppEnabledExtensionNames + pCreateInfo->enabledExtensionCount);
        }

        std::vector<VkDeviceQueueCreateInfo> queue_infos(pCreateInfo->pQueueCreateInfos,
                                                         pCreateInfo->pQueueCreateInfos + pCreateInfo->queueCreateInfoCount);

        std::vector<VkDeviceQueueGlobalPriorityCreateInfoEXT> prio_infos(pCreateInfo->queueCreateInfoCount);

        if (!supports_prio)
        {
            std::cerr << "VK_EXT_global_priority not supported, can't set priority!" << std::endl;
        }
        else
        {
            bool already_enabled = false;
            for (auto ext : enabled_exts)
            {
                if (!std::strcmp(ext, "VK_EXT_global_priority"))
                {
                    already_enabled = true;
                    break;
                }
            }
            if (!already_enabled)
                enabled_exts.push_back("VK_EXT_global_priority");

            for (size_t i = 0; i < queue_infos.size(); i++)
            {
                auto& queue_info = queue_infos[i];
                if (vk_find_struct(&queue_info, DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_EXT))
                {
                    std::cerr << "Already a priority set for queue family " << queue_info.queueFamilyIndex << std::endl;
                }
                else
                {
                    auto& prio_info = prio_infos[i];
                    prio_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_EXT;
                    prio_info.pNext = queue_info.pNext;
                    prio_info.globalPriority = get_user_prio();

                    queue_info.pNext = (void*) &prio_info;
                }
            }

            our_info.ppEnabledExtensionNames = enabled_exts.data();
            our_info.enabledExtensionCount   = enabled_exts.size();
            our_info.pQueueCreateInfos       = queue_infos.data();
            our_info.queueCreateInfoCount    = queue_infos.size();
        }

        VkResult ret = createFunc(m_phys_dev, &our_info, pAllocator, pDevice);
        if (ret == VK_ERROR_NOT_PERMITTED_EXT)
        {
            std::cerr << "Device creation failed with VK_ERROR_NOT_PERMITTED_EXT, falling back to medium prio." << std::endl;
            for (auto& prio_info : prio_infos)
            {
                prio_info.globalPriority = VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_EXT;
            }

            ret = createFunc(m_phys_dev, &our_info, pAllocator, pDevice);
        }
        if (ret != VK_SUCCESS)
            throw VkError(ret);

        m_device = *pDevice;

        if (pAllocator)
            m_alloc_callbacks = std::make_unique<VkAllocationCallbacks>(*pAllocator);

        fillDispatchTableDevice(m_device, gdpa, &m_funcs);
    }

    LayerDevice::~LayerDevice()
    {
        m_funcs.DestroyDevice(m_device, m_alloc_callbacks.get());
    }

}