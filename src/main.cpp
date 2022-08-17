#include "GLFW/glfw3.h"

int main()
{
   if (glfwInit() != GLFW_TRUE)
      return -1;

   GLFWwindow *m_Window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
   if (m_Window == nullptr)
      return -1;

   while (!glfwWindowShouldClose(m_Window))
   {
      glfwPollEvents();
   }
}
