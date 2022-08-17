#include "GLFW/glfw3.h"
#include "Common/interface/RefCntAutoPtr.hpp"

#if PLATFORM_LINUX
// Remove xlib symbols
#ifdef Bool
#undef Bool
#endif
#ifdef True
#undef True
#endif
#ifdef False
#undef False
#endif

#include "Platforms/Linux/interface/LinuxNativeWindow.h"
#define GLFW_EXPOSE_NATIVE_X11
#endif

#if VULKAN_SUPPORTED
#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"
#endif

#if GL_SUPPORTED
#include "Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#endif

#include "GLFW/glfw3native.h"

using namespace Diligent;

int main()
{
   if (glfwInit() != GLFW_TRUE)
      return -1;

   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   GLFWwindow *m_Window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
   if (m_Window == nullptr)
      return -1;

   LinuxNativeWindow nativeWin;
   nativeWin.WindowId = glfwGetX11Window(m_Window);
   nativeWin.pDisplay = glfwGetX11Display();

   RefCntAutoPtr<IRenderDevice> m_Device;
   RefCntAutoPtr<IDeviceContext> m_Context;
   RefCntAutoPtr<ISwapChain> m_SwapChain;

   // Initial vulkan setup
   IEngineFactoryVk *vulkEngine = GetEngineFactoryVk();
   EngineVkCreateInfo EngineCI;
   vulkEngine->CreateDeviceAndContextsVk(EngineCI, &m_Device, &m_Context);
   vulkEngine->CreateSwapChainVk(m_Device, m_Context, SwapChainDesc(), nativeWin, &m_SwapChain);

   if (m_Device && m_Context && m_SwapChain && vulkEngine)
   {
      std::cout << "Successfully initialized vulkan backend." << std::endl;
   }
   else
   {
      std::cerr << "Failed to init vulkan backend." << std::endl;
      abort();
   }

   while (!glfwWindowShouldClose(m_Window))
   {
      glfwPollEvents();

      auto *pRTV = m_SwapChain->GetCurrentBackBufferRTV();
      auto *pDSV = m_SwapChain->GetDepthBufferDSV();
      m_Context->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

      const float ClearColor[] = {1.0f, 0.0f, 0.0f, 1.0f};
      m_Context->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

      m_SwapChain->Present();
   }
}
   }
}
