// VulkanEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <vulkan/vulkan.h>
#include "SDL_vulkan.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <vector>
#include <GLFW/glfw3.h>
#include <iostream>

#undef main

GLFWwindow* window = nullptr;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

VkInstance instance;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDebugUtilsMessengerEXT callback;
VkApplicationInfo applicationInfo;
VkInstanceCreateInfo createInfo;

const std::vector<const char*> validationLayers = 
{
    "VK_LAYER_LUNARG_api_dump",
    "VK_LAYER_LUNARG_standard_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

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

   vkDestroyInstance(instance, nullptr);
   glfwDestroyWindow(window);
   glfwTerminate();
}

void InitVulkan()
{
   CreateVulkanInstance();
   SetupDebugCallback();
   PickPhysicalDevice();
}

bool IsDeviceSuitable(VkPhysicalDevice device) 
{
   VkPhysicalDeviceProperties deviceProperties;
   vkGetPhysicalDeviceProperties(device, &deviceProperties);

   VkPhysicalDeviceFeatures deviceFeatures;
   vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

   //Get features and properties and prioritize devices
   //Can be extended later
   //for now we just need Vulkan support so any device will do.
   return true;
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