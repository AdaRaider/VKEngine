// VulkanEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <vulkan/vulkan.h>
#include "SDL_vulkan.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <vector>
#undef main

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

//The window we'll be rendering to
SDL_Window* window = NULL;
//The surface contained by the window
SDL_Surface* screenSurface = NULL;

VkInstance instance;
VkApplicationInfo applicationInfo;
VkInstanceCreateInfo createInfo;

void Update()
{

}

void Render()
{

}

bool InitSDL()
{
   //Initialize SDL
   if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
   {
      printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
      return false;
   }
   //Create window
   window = SDL_CreateWindow("VKEngine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
   if (window == NULL)
   {
      printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
   }
   return true;
}

void CreateVulkanInstance()
{
   applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   applicationInfo.pApplicationName = "Vulkan Engine";
   applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
   applicationInfo.pEngineName = "Vulkan Engine";
   applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
   applicationInfo.apiVersion = VK_API_VERSION_1_0;

   unsigned int count = 0;
   std::vector<const char*> extensions;
   if (!SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data()))
   {
      //Handle error.
   }

   createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   createInfo.pApplicationInfo = &applicationInfo;
   createInfo.enabledExtensionCount = count;
   createInfo.ppEnabledExtensionNames = extensions.data();
   createInfo.enabledLayerCount = 0;

   if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) 
   {
      throw std::runtime_error("Failed to create instance!");
   }
}

void InitVulkan()
{
   CreateVulkanInstance();
}

int main()
{
   if(InitSDL())
   {      
      InitVulkan();
      while (true)
      {
         Update();
         Render();
      }         
   }
   return 0;
}