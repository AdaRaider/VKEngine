// VulkanEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>


int main(void)
{
   GLFWwindow* window;

   int windowWidth = 1024;
   int windowHeight = 768;
   glfwInit();
   glfwWindowHint(GLFW_SAMPLES, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   window = glfwCreateWindow(1024, 768, "Tutorial 14 - Render To Texture", NULL, NULL);

   glfwMakeContextCurrent(window);

   glewExperimental = true; // Needed in core profile
   if (glewInit() != GLEW_OK) {
      fprintf(stderr, "Failed to initialize GLEW\n");
   }

   glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
   glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
   glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


   while (!glfwWindowShouldClose(window))
   {
      glClear(GL_COLOR_BUFFER_BIT);
      glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
      glfwSwapBuffers(window);
      glfwPollEvents();
   }

   return 0;
}