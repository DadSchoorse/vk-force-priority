#include <cstring>
#include <mutex>
#include <unordered_map>
#include <utility>

#include "layer_device.hpp"
#include "layer_instance.hpp"
#include "vkerror.hpp"


namespace layer
{
    std::mutex g_lock;

    std::unordered_map<void*, std::unique_ptr<LayerInstance>> g_instance_map;
    std::unordered_map<void*, std::unique_ptr<LayerDevice>>   g_device_map;

    template<typename DispatchableType>
    void* getKey(DispatchableType inst)
    {
        return *(void**) inst;
    }

    VkResult VKAPI_CALL layer_CreateInstance(const VkInstanceCreateInfo*  pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator,
                                             VkInstance*                  pInstance)
    {
        std::lock_guard lock(g_lock);
        try
        {
            auto instance = std::make_unique<LayerInstance>(pCreateInfo, pAllocator, pInstance);
            g_instance_map.insert({getKey(*pInstance), std::move(instance)});
            return VK_SUCCESS;
        }
        catch (VkError e)
        {
            return e.result();
        }
    }

    void VKAPI_CALL layer_DestroyInstance(VkInstance                   instance,
                                          const VkAllocationCallbacks* pAllocator)
    {
        (void) pAllocator;
        if (!instance)
            return;

        std::lock_guard lock(g_lock);
        g_instance_map.erase(getKey(instance));
    }

    VkResult VKAPI_CALL layer_CreateDevice(VkPhysicalDevice             physicalDevice,
                                           const VkDeviceCreateInfo*    pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator,
                                           VkDevice*                    pDevice)
    {
        std::lock_guard lock(g_lock);
        try
        {
            LayerInstance* instance = g_instance_map[getKey(physicalDevice)].get();
            auto device = std::make_unique<LayerDevice>(instance, physicalDevice, pCreateInfo, pAllocator, pDevice);
            g_device_map.insert({getKey(*pDevice), std::move(device)});
            return VK_SUCCESS;
        }
        catch (VkError e)
        {
            return e.result();
        }
    }

    void VKAPI_CALL layer_DestroyDevice(VkDevice                     device,
                                        const VkAllocationCallbacks* pAllocator)
    {
        (void) pAllocator;
        if (!device)
            return;

        std::lock_guard lock(g_lock);
        g_device_map.erase(getKey(device));
    }

    static PFN_vkVoidFunction internalGetProcAddr(const char* name);

    extern "C" VK_LAYER_EXPORT PFN_vkVoidFunction VKAPI_CALL layer_GetDeviceProcAddr(VkDevice    device_,
                                                                                     const char* pName)
    {
        PFN_vkVoidFunction result = internalGetProcAddr(pName);
        if (result)
            return result;

        if (!device_)
            return nullptr;

        std::lock_guard lock(g_lock);
        LayerDevice* device = g_device_map[getKey(device_)].get();
        return device->vk().GetDeviceProcAddr(device_, pName);
    }

    extern "C" VK_LAYER_EXPORT PFN_vkVoidFunction VKAPI_CALL layer_GetInstanceProcAddr(VkInstance  instance_,
                                                                                       const char* pName)
    {
        PFN_vkVoidFunction result = internalGetProcAddr(pName);
        if (result)
            return result;

        if (!instance_)
            return nullptr;

        std::lock_guard lock(g_lock);
        LayerInstance* instance = g_instance_map[getKey(instance_)].get();
        return instance->vk().GetInstanceProcAddr(instance_, pName);
    }

    static PFN_vkVoidFunction internalGetProcAddr(const char* name)
    {
#define GETPROCADDR(func) \
    if (!std::strcmp(name, "vk" #func)) \
        return (PFN_vkVoidFunction) &layer_##func;

        // instance functions
        GETPROCADDR(CreateInstance);
        GETPROCADDR(DestroyInstance);
        GETPROCADDR(GetInstanceProcAddr);

        // device functions
        GETPROCADDR(CreateDevice);
        GETPROCADDR(DestroyDevice);
        GETPROCADDR(GetDeviceProcAddr);

#undef GETPROCADDR

        return nullptr;
    }

}
