#pragma once

#include <memory>

#include "vkdispatch.hpp"

namespace layer
{
    class LayerInstance
    {
    public:

        LayerInstance(const VkInstanceCreateInfo*  pCreateInfo,
                      const VkAllocationCallbacks* pAllocator,
                      VkInstance*                  pInstance);
        ~LayerInstance();
        LayerInstance(const LayerInstance& other) = delete;
        LayerInstance& operator=(const LayerInstance& other) = delete;
        LayerInstance(LayerInstance&& other)  = delete;
        LayerInstance& operator=(LayerInstance&& other) = delete;

        VkInstance instance() const;
        const InstanceDispatch& vk() const;

    private:

        VkInstance       m_instance;
        InstanceDispatch m_funcs;
        std::unique_ptr<VkAllocationCallbacks> m_alloc_callbacks;

    };

    inline VkInstance LayerInstance::instance() const
    {
        return m_instance;
    }

    inline const InstanceDispatch& LayerInstance::vk() const
    {
        return m_funcs;
    }
}
