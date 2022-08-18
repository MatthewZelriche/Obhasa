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

#include "Common/interface/BasicMath.hpp"
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

   // Describe vertex attrib layout.
   LayoutElement elems;
   elems.NumComponents = 4;
   elems.ValueType = VT_FLOAT32;

   GraphicsPipelineStateCreateInfo PSOCreateInfo;
   PSOCreateInfo.PSODesc.Name = "Basic pipeline";
   PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
   PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = m_SwapChain->GetDesc().ColorBufferFormat;
   PSOCreateInfo.GraphicsPipeline.DSVFormat = m_SwapChain->GetDesc().DepthBufferFormat;
   PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
   PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;
   PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
   PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FillMode = FILL_MODE_SOLID;
   PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
   PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = &elems;
   PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = 1;

   // Create shader programs.
   RefCntAutoPtr<IShaderSourceInputStreamFactory> shaderSourceFact;
   vulkEngine->CreateDefaultShaderSourceStreamFactory(nullptr, &shaderSourceFact);
   ShaderCreateInfo shaderCreateInfo;
   shaderCreateInfo.pShaderSourceStreamFactory = shaderSourceFact;
   shaderCreateInfo.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
   shaderCreateInfo.UseCombinedTextureSamplers = True;

   RefCntAutoPtr<IShader> vertShaderProgram;
   {
      shaderCreateInfo.Desc.Name = "Vertex Shader";
      shaderCreateInfo.Desc.ShaderType = SHADER_TYPE_VERTEX;
      shaderCreateInfo.EntryPoint = "main";
      shaderCreateInfo.FilePath = "Resources/Shaders/vert.hlsl";
      m_Device->CreateShader(shaderCreateInfo, &vertShaderProgram);
   }
   PSOCreateInfo.pVS = vertShaderProgram;

   RefCntAutoPtr<IShader> fragShaderProgram;
   {
      shaderCreateInfo.Desc.Name = "Fragment Shader";
      shaderCreateInfo.Desc.ShaderType = SHADER_TYPE_PIXEL;
      shaderCreateInfo.EntryPoint = "main";
      shaderCreateInfo.FilePath = "Resources/Shaders/frag.hlsl";
      m_Device->CreateShader(shaderCreateInfo, &fragShaderProgram);
   }
   PSOCreateInfo.pPS = fragShaderProgram;

   // Create vertex buffer
   RefCntAutoPtr<IBuffer> vertBuf;
   float4 pos[3];
   pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
   pos[1] = float4(0.0, 0.5, 0.0, 1.0);
   pos[2] = float4(0.5, -0.5, 0.0, 1.0);
   BufferDesc vertBufDesc;
   vertBufDesc.Name = "Vertex Buffer";
   vertBufDesc.Usage = USAGE_IMMUTABLE;
   vertBufDesc.BindFlags = BIND_VERTEX_BUFFER;
   vertBufDesc.Size = sizeof(pos);
   BufferData vertBufData;
   vertBufData.DataSize = vertBufDesc.Size;
   vertBufData.pData = pos;
   m_Device->CreateBuffer(vertBufDesc, &vertBufData, &vertBuf);

   RefCntAutoPtr<IPipelineState> PSO;
   m_Device->CreateGraphicsPipelineState(PSOCreateInfo, &PSO);

   while (!glfwWindowShouldClose(m_Window))
   {
      glfwPollEvents();

      auto *pRTV = m_SwapChain->GetCurrentBackBufferRTV();
      auto *pDSV = m_SwapChain->GetDepthBufferDSV();
      m_Context->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

      const float ClearColor[] = {1.0f, 0.0f, 0.0f, 1.0f};
      m_Context->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
      m_Context->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

      m_Context->SetVertexBuffers(0, 1, &vertBuf, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
      m_Context->SetPipelineState(PSO);

      DrawAttribs drawAttrs;
      drawAttrs.NumVertices = 3;
      m_Context->Draw(drawAttrs);

      m_SwapChain->Present();
   }
}