#pragma once

#include <memory>

#include "vkdispatch.hpp"

namespace layer
{
    class LayerInstance;

    class LayerDevice
    {
    public:

        LayerDevice(LayerInstance*               instance,
                    VkPhysicalDevice             phys_dev,
                    const VkDeviceCreateInfo*    pCreateInfo,
                    const VkAllocationCallbacks* pAllocator,
                    VkDevice*                    pDevice);
        ~LayerDevice();
        LayerDevice(const LayerDevice& other) = delete;
        LayerDevice& operator=(const LayerDevice& other) = delete;
        LayerDevice(LayerDevice&& other)  = delete;
        LayerDevice& operator=(LayerDevice&& other) = delete;

        LayerInstance* instance();
        VkPhysicalDevice phys_dev() const;
        const DeviceDispatch& vk() const;
        VkDevice device() const;


    private:

        LayerInstance*                         m_instance;
        VkPhysicalDevice                       m_phys_dev;
        VkDevice                               m_device;
        DeviceDispatch                         m_funcs;
        std::unique_ptr<VkAllocationCallbacks> m_alloc_callbacks;

    };

    inline LayerInstance* LayerDevice::instance()
    {
        return m_instance;
    }

    inline VkPhysicalDevice LayerDevice::phys_dev() const
    {
        return m_phys_dev;
    }

    inline const DeviceDispatch& LayerDevice::vk() const
    {
        return m_funcs;
    }

    inline VkDevice LayerDevice::device() const
    {
        return m_device;
    }

}
