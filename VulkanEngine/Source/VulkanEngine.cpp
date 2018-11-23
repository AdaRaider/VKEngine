
#include <vulkan/vulkan.h>
#include "SDL_vulkan.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <vector>
#include <GLFW/glfw3.h>
#include <iostream>
#include <optional>
#include <set>
#include <algorithm>
#include <cstdlib>

#undef main

GLFWwindow* window = nullptr;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

VkInstance instance;
VkDevice device;
VkSwapchainKHR swapChain;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkPhysicalDeviceFeatures deviceFeatures = {};
VkDebugUtilsMessengerEXT callback;
VkApplicationInfo applicationInfo;
VkInstanceCreateInfo createInfo;

VkQueue presentQueue;
VkQueue graphicsQueue;

const std::vector<const char*> validationLayers = 
{
    //"VK_LAYER_LUNARG_api_dump",
    "VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = 
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VkSurfaceKHR surface;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices
{
   std::optional<uint32_t> graphicsFamily;
   std::optional<uint32_t> presentFamily;

   bool IsComplete()
   {
      return graphicsFamily.has_value() && presentFamily.has_value();
   }
};

struct SwapChainSuppportDetails
{
   VkSurfaceCapabilitiesKHR capabilities;
   std::vector <VkSurfaceFormatKHR> formats;
   std::vector<VkPresentModeKHR> presentModes;
};



void Update()
{

}

void Render()
{

}

bool InitWindow()
{
   glfwInit();
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
   window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Vulkan Engine", nullptr, nullptr);
   return true;
}

bool CheckValidationLayerSupport()
{
   uint32_t layerCount;
   vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

   std::vector<VkLayerProperties> availableLayers(layerCount);
   vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

   for (const char* layerName : validationLayers) 
   {
      bool layerFound = false;

      for (const auto& layerProperties : availableLayers) 
      {
         if (strcmp(layerName, layerProperties.layerName) == 0) 
         {
            layerFound = true;
            break;
         }
      }

      if (!layerFound) 
      {
         return false;
      }
   }

   return true;
}

std::vector<const char*> GetRequiredExtensions()
{
   uint32_t glfwRequiredExtCount = 0;
   const char** glfwExtensions;
   glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwRequiredExtCount);
   std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwRequiredExtCount);
   if (enableValidationLayers)
   {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
   }
   return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
   VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
   VkDebugUtilsMessageTypeFlagsEXT messageType,
   const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
   void* pUserData) 
{
   std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
   return VK_FALSE;
}

//Get the vkCreateDebugUtilsMessengerEXT func and return the result of calling the function
VkResult CreateDebugUtilsMessengerEXT(
   VkInstance instance, 
   const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
   const VkAllocationCallbacks* pAllocator, 
   VkDebugUtilsMessengerEXT* pCallback) 
{
   auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
   if (func != nullptr) 
   {
      return func(instance, pCreateInfo, pAllocator, pCallback);
   }
   else 
   {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
   }
}

//Get the vkDestroyDebugUtilsMessengerEXT func and return the result of calling the function
void DestroyDebugUtilsMessengerEXT(
   VkInstance instance, 
   VkDebugUtilsMessengerEXT callback, 
   const VkAllocationCallbacks* pAllocator) 
{
   auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
   if (func != nullptr) 
   {
      func(instance, callback, pAllocator);
   }
}

void SetupDebugCallback()
{
   if(!enableValidationLayers) return;

   VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
   createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
   createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
   createInfo.pfnUserCallback = debugCallback;
   createInfo.pUserData = nullptr; 

   if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS) 
   {
      throw std::runtime_error("Failed to set up debug callback!");
   }
}

void CreateVulkanInstance()
{
   if (enableValidationLayers && !CheckValidationLayerSupport())
   {
      throw std::runtime_error("validation layers requested, but not available!");
   }

   applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   applicationInfo.pApplicationName = "Vulkan Engine";
   applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
   applicationInfo.pEngineName = "Vulkan Engine";
   applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
   applicationInfo.apiVersion = VK_API_VERSION_1_0;

   auto extensions = GetRequiredExtensions();
   createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
   createInfo.ppEnabledExtensionNames = extensions.data();

   if (enableValidationLayers)
   {
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
   }
   else
   {
      createInfo.enabledLayerCount = 0;
   }

   if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) 
   {
      throw std::runtime_error("Failed to create instance!");
   }
}

void Cleanup() 
{
   if (enableValidationLayers) 
   {
      DestroyDebugUtilsMessengerEXT(instance, callback, nullptr);
   }

   vkDestroySurfaceKHR(instance, surface, nullptr);
   vkDestroyDevice(device, nullptr);
   vkDestroySwapchainKHR(device, swapChain, nullptr);
   vkDestroyInstance(instance, nullptr);
   glfwDestroyWindow(window);
   glfwTerminate();
}

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
{
   QueueFamilyIndices indices;

   uint32_t queueFamilyCount = 0;
   vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
   std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
   vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

   int i = 0;
   for (const auto& queueFamily : queueFamilies)
   {
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

      if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
         indices.graphicsFamily = i;
      if (queueFamily.queueCount > 0 && presentSupport)
         indices.presentFamily = i;
      if (indices.IsComplete())
         break;
      i++;
   }

   return indices;
}

SwapChainSuppportDetails querySwapChainSupport(VkPhysicalDevice device)
{
   SwapChainSuppportDetails details;

   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

   uint32_t formatCount;
   vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

   if (formatCount != 0)
   {
      details.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(
         device, 
         surface, 
         &formatCount,
         details.formats.data());
   }

   uint32_t presentModeCount;
   vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

   if (presentModeCount != 0)
   {
      details.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(
         device, 
         surface, 
         &presentModeCount, 
         details.presentModes.data());
   }
   
   return details;
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats) 
{
   if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) 
   {
      return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
   }

   for (const auto& availableFormat : availableFormats) 
   {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
         return availableFormat;
   }

   return availableFormats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(std::vector<VkPresentModeKHR> const& availablePresentModes)
{
   VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

   for (auto const& availablePresentMode : availablePresentModes) 
   {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
      {
         return availablePresentMode;
      }
      else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) 
      {
         bestMode = availablePresentMode;
      }
   }
   return bestMode;
}

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
{
   if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
   {
      return capabilities.currentExtent;
   }
   else
   {
      VkExtent2D actualExtent = { SCREEN_WIDTH, SCREEN_HEIGHT };

      actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
      actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

      return actualExtent;
   }
}

bool CheckDeviceExtensionSupport(VkPhysicalDevice device) 
{
   uint32_t extensionCount;
   vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

   std::vector<VkExtensionProperties> availableExtensions(extensionCount);
   vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

   std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

   for (const auto& extension : availableExtensions) 
   {
      requiredExtensions.erase(extension.extensionName);
   }

   return requiredExtensions.empty();
}

bool IsDeviceSuitable(VkPhysicalDevice device) 
{
   bool swapChainAdequate = false;
   QueueFamilyIndices indices = FindQueueFamilies(device);
   bool extensionsSupported = CheckDeviceExtensionSupport(device);

   if (extensionsSupported)
   {
      SwapChainSuppportDetails swapChainSupport = querySwapChainSupport(device);
      swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
   }

   return indices.IsComplete() && extensionsSupported && swapChainAdequate;
}

void PickPhysicalDevice()
{
   uint32_t deviceCount = 0;
   vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
   if (deviceCount == 0) 
   {
      throw std::runtime_error("Failed to find GPUs with Vulkan support!");
   }
   std::vector<VkPhysicalDevice> devices(deviceCount);

   vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
   for (const auto& device : devices) 
   {
      if (IsDeviceSuitable(device)) 
      {
         physicalDevice = device;
         break;
      }
   }

   if (physicalDevice == VK_NULL_HANDLE) 
   {
      throw std::runtime_error("Failed to find a suitable GPU!");
   }
}

void CreateLogicalDevice()
{
   QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
   VkDeviceQueueCreateInfo queueCreateInfo = {};
     
   std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
   std::set<uint32_t> uniqueQueueFamilies = 
   { 
      indices.graphicsFamily.value(), 
      indices.presentFamily.value() 
   };

   float queuePriority = 1.0f;
   for (uint32_t queueFamily : uniqueQueueFamilies) 
   {
      VkDeviceQueueCreateInfo queueCreateInfo = {};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
   }

   VkDeviceCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   createInfo.pEnabledFeatures = &deviceFeatures;
   createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
   createInfo.pQueueCreateInfos = queueCreateInfos.data();
   createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
   createInfo.ppEnabledExtensionNames = deviceExtensions.data();

   if (enableValidationLayers) 
   {
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
   }
   else 
   {
      createInfo.enabledLayerCount = 0;
   }

   if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) 
   {
      throw std::runtime_error("failed to create logical device!");
   }   

   vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
   vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
}

void CreateSurface()
{
   if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) 
   {
      throw std::runtime_error("Failed to create window surface!");
   }
}

void CreateSwapChain()
{
   SwapChainSuppportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
   VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
   VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
   VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

   uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
   bool hasSwapChainCapablities = swapChainSupport.capabilities.maxImageCount > 0;
   bool imageCountExceedsMax = imageCount > swapChainSupport.capabilities.maxImageCount;
   
   if (hasSwapChainCapablities && imageCountExceedsMax) 
   {
      imageCount = swapChainSupport.capabilities.maxImageCount;
   }

   VkSwapchainCreateInfoKHR createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
   createInfo.surface = surface;
   createInfo.minImageCount = imageCount;
   createInfo.imageFormat = surfaceFormat.format;
   createInfo.imageColorSpace = surfaceFormat.colorSpace;
   //createInfo.imageExtent = extent;
   createInfo.imageArrayLayers = 1;
   /* The imageUsage bit field specifies what kind of operations we'll use the images 
   in the swap chain for. We're going to render directly to them, 
   which means that they're used as color attachment. 
   It is also possible that you'll render images to a separate image first 
   to perform operations like post-processing. In that case you may use a value 
   like VK_IMAGE_USAGE_TRANSFER_DST_BIT instead and use a memory operation to transfer 
   the rendered image to a swap chain image.*/
   createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


   QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
   uint32_t queueFamilyIndices[] = 
   {  indices.graphicsFamily.value(), 
      indices.presentFamily.value() 
   };

   if (indices.graphicsFamily != indices.presentFamily) 
   {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
   }
   else 
   {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0; // Optional
      createInfo.pQueueFamilyIndices = nullptr; // Optional
   }

   /*We can specify that a certain transform should be applied to images 
   in the swap chain if it is supported, 
   like a 90 degree clockwise rotation or horizontal flip. 
   To specify that you do not want any transformation, 
   simply specify the current transformation.*/
   createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
   createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
   createInfo.presentMode = presentMode;
   createInfo.clipped = VK_TRUE;
   /*With Vulkan it's possible that your swap chain becomes invalid or unoptimized 
   while your application is running, for example because the window was resized. 
   In that case the swap chain actually needs to be recreated from scratch and a 
   reference to the old one must be specified in this field. */
   createInfo.oldSwapchain = VK_NULL_HANDLE;

   _putenv("DISABLE_VK_LAYER_VALVE_steam_overlay_1=1");
   if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) 
   {
      throw std::runtime_error("Failed to create swap chain!");
   }
}

void InitVulkan()
{
   CreateVulkanInstance();
   SetupDebugCallback();
   CreateSurface();
   PickPhysicalDevice();
   CreateLogicalDevice();
   CreateSwapChain();
}

int main()
{
   if(InitWindow())
   {      
      InitVulkan();
      while (!glfwWindowShouldClose(window)) 
      {
         Update();
         Render();

         glfwPollEvents();
      }   
      Cleanup();
   }
   return 0;
}