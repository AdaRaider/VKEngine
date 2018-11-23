
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

#undef main

GLFWwindow* window = nullptr;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

VkInstance instance;
VkDevice device;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkPhysicalDeviceFeatures deviceFeatures = {};
VkDebugUtilsMessengerEXT callback;
VkApplicationInfo applicationInfo;
VkInstanceCreateInfo createInfo;

VkQueue presentQueue;
VkQueue graphicsQueue;

const std::vector<const char*> validationLayers = 
{
    "VK_LAYER_LUNARG_api_dump",
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
      //DestroyDebugUtilsMessengerEXT(instance, callback, nullptr);
   }

   vkDestroySurfaceKHR(instance, surface, nullptr);
   vkDestroyDevice(device, nullptr);
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

void InitVulkan()
{
   CreateVulkanInstance();
   SetupDebugCallback();
   CreateSurface();
   PickPhysicalDevice();
   CreateLogicalDevice();
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