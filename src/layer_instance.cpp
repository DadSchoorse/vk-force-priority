#include "layer_instance.hpp"

#include <cstring>


#include "vkerror.hpp"

namespace layer
{

    LayerInstance::LayerInstance(const VkInstanceCreateInfo*  pCreateInfo,
                                 const VkAllocationCallbacks* pAllocator,
                                 VkInstance*                  pInstance)
    {
        VkLayerInstanceCreateInfo* layerCreateInfo = (VkLayerInstanceCreateInfo*) pCreateInfo->pNext;

        // step through the chain of pNext until we get to the link info
        while (layerCreateInfo
               && (layerCreateInfo->sType != VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO || layerCreateInfo->function != VK_LAYER_LINK_INFO))
        {
            layerCreateInfo = (VkLayerInstanceCreateInfo*) layerCreateInfo->pNext;
        }

        if (layerCreateInfo == nullptr)
            throw VkError(VK_ERROR_INITIALIZATION_FAILED);

        PFN_vkGetInstanceProcAddr gpa = layerCreateInfo->u.pLayerInfo->pfnNextGetInstanceProcAddr;
        // move chain on for next layer
        layerCreateInfo->u.pLayerInfo = layerCreateInfo->u.pLayerInfo->pNext;

        PFN_vkCreateInstance createFunc = (PFN_vkCreateInstance) gpa(VK_NULL_HANDLE, "vkCreateInstance");

        VkResult ret = createFunc(pCreateInfo, pAllocator, pInstance);
        if (ret != VK_SUCCESS)
            throw VkError(ret);

        m_instance = *pInstance;

        if (pAllocator)
            m_alloc_callbacks = std::make_unique<VkAllocationCallbacks>(*pAllocator);

        fillDispatchTableInstance(m_instance, gpa, &m_funcs);
    }

    LayerInstance::~LayerInstance()
    {
        m_funcs.DestroyInstance(m_instance, m_alloc_callbacks.get());
    }

}