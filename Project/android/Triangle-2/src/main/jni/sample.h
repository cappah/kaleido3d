//                  Copyright (c) 2016 QUALCOMM Technologies Inc.
//                              All Rights Reserved.

#ifndef __SAMPLE_H__
#define __SAMPLE_H__


// This definition enables the Android extensions
#define VK_USE_PLATFORM_ANDROID_KHR

// This definition allows prototypes of Vulkan API functions,
// rather than dynamically loading entrypoints to the API manually.
#define VK_PROTOTYPES

#include <assert.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "VkSample", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "VkSample", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "VkSample", __VA_ARGS__))


// Helper define for tracking error locations in code
#define VK_CHECK(X) if (!(X)) { LOGE("VK_CHECK Failure"); assert((X));}

struct DepthBuffer {
    VkFormat format;
    VkImage image;
    VkDeviceMemory mem;
    VkImageView view;
};

struct SwapchainBuffer
{
    vk::Image image;
    vk::CommandBuffer cmdBuffer;
    vk::ImageView view;
};

struct Vertices {
    VkBuffer buf;
    VkDeviceMemory mem;
    VkPipelineVertexInputStateCreateInfo vi;
    VkVertexInputBindingDescription      vi_bindings[1];
    VkVertexInputAttributeDescription    vi_attrs[2];
};

class VkSample
{
public:
    VkSample();
    ~VkSample();

    bool Initialize(ANativeWindow* window);

    void DrawFrame();
    
    bool IsInitialized() { return m_initialized; }
    
    bool TearDown();
protected:

    bool CreateInstance();
    bool GetPhysicalDevices();
    void InitSurface();
    void InitDevice();
    void InitSwapchain();
    void InitCommandbuffers();
    void InitSync();
    void InitVertexBuffers();
    void InitLayouts();
    void InitRenderPass();
    void InitPipeline();
    void InitFrameBuffers();

    void BuildCmdBuffer();
    void SetNextBackBuffer();
    void PresentBackBuffer();
    
    VkShaderModule CreateShaderModule(const uint32_t* code, uint32_t size);
    bool GetMemoryTypeFromProperties( uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

    ANativeWindow* m_androidWindow;

    vk::Instance instance;
    vk::PhysicalDevice physicalDevice;
    vk::PhysicalDeviceProperties physicalDeviceProps;
    vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProps;

    vk::Device device;
    vk::Queue queue;

    vk::SurfaceKHR surface;
    vk::SurfaceFormatKHR surfaceFormat;
    vk::SwapchainKHR swapChain;

    VkInstance m_instance;
    VkPhysicalDevice* m_pPhysicalDevices;
    VkPhysicalDevice m_physicalDevice;
    VkPhysicalDeviceProperties m_physicalDeviceProperties;
    VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;
    VkDevice m_device;
    uint32_t m_physicalDeviceCount;
    uint32_t m_queueFamilyIndex;
    VkQueue m_queue;
    VkSurfaceKHR m_surface;
    VkSurfaceFormatKHR m_surfaceFormat;

    VkSwapchainKHR m_swapchain;
    SwapchainBuffer* m_swapchainBuffers;
    uint32_t m_swapchainCurrentIdx;
    uint32_t m_height;
    uint32_t m_width;
    uint32_t m_swapchainImageCount;
    VkSemaphore m_backBufferSemaphore;
    VkSemaphore m_renderCompleteSemaphore;
    VkFramebuffer* m_frameBuffers;

    VkCommandPool m_commandPool;

    DepthBuffer* m_depthBuffers;

    Vertices m_vertices;
    
    VkDescriptorSetLayout m_descriptorLayout;
    VkPipelineLayout  m_pipelineLayout;
    VkRenderPass m_renderPass;
    VkPipeline m_pipeline;
    
    uint64_t GetTime();
    uint64_t m_frameTimeBegin;
    uint64_t m_frameTimeEnd;
    uint64_t m_frameIdx;
    
    bool m_initialized;
    bool m_initBegun;
};

#endif
