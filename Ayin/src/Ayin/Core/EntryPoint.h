#pragma once

//头文件中的错误其实不用管，只要保证在预处理完后的.cpp文件中没有问题就行
//比如这里的报错，缺少声明，但是在具体引用时，引用方能补齐就行（比如Ayin.h中就补齐了）
//因为头文件只参与预处理流程，后续的编译连接只和.cpp有关，这也是为什么只有.cpp被称为翻译模块
//所以只要错误不被带到最终.cpp里就行

//我承认上述仅为猜测，因为当时VS2022有错误，比如Ayin哪里，但之后就没有了。我现在也不确定到底是那样了？？！！
//不过既然.h中的内容也是要加入.cpp的，并且其中也会包含C++语法，那么至少存在语法层面上的约束，这些错误不能犯
//不如我在Application.h中将Core.h注释掉，会导致宏被识别为类，进而导致一系列语法错误

#ifdef AYIN_PLATFORM_WINDOWS

extern Ayin::Application* Ayin::CreatApplication();//客户端侧实现该方法


//启动流程由引擎侧控制
int main(int argc, char** argv) {

	Ayin::Log::Init();
	//Ayin::Log::GetCoreLogger()->warn("Initialized Log");
	//Ayin::Log::GetClientLogger()->info("Hello");

	AYIN_CORE_WARN("Initialized Log");
	AYIN_INFO("Hello");


	Ayin::Application* app = Ayin::CreatApplication();
	app->Run();
	delete app;

}

#endif

