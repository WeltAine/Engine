#pragma once

//Ayin 程序使用（客户端侧使用，如SandBox）


#include "Ayin/Application.h"
#include "Ayin/Core/Timestep.h"
#include "Ayin/Log.h"//这个要写在EntryPoint前面，不要破坏依赖顺序，否则dll可以生成，但是sandBox在运行是会报错，显示没有Log与其内容？？？！！！，奇怪的事情
#include "Ayin/Layer.h"

//#include "Ayin/ImGui/ImGuiLayer.h"

#include "Ayin/Input.h"
#include "Ayin/KayCodes.h"
#include "Ayin/MouseButtonCodes.h"
#include "Ayin/CameraController.h"

// ---渲染器--------------------------
#include "Ayin/Renderer/Renderer.h"
#include "Ayin/Renderer/RenderCommand.h"

#include "Ayin/Renderer/Shader.h"
#include "Ayin/Renderer/Texture.h"

#include "Ayin/Renderer/Buffer.h"
#include "Ayin/Renderer/VertexArray.h"
// -----------------------------------


// ---数学----------------------------
#include <glm/gtc/type_ptr.hpp>
// -----------------------------------

// ---ImGui---------------------------
#include <ImGui/imgui.h>
// -----------------------------------
