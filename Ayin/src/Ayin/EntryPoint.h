#pragma once

//头文件中的错误其实不用管，只要保证在预处理完后的.cpp文件中没有问题就行
//比如这里的报错，缺少声明，但是在具体引用时，引用方能补齐就行（比如Ayin.h中就补齐了）
//因为头文件只参与预处理流程，后续的编译连接只和.cpp有关，这也是为什么只有.cpp被称为翻译模块
//所以只要错误不被带到最终.cpp里就行

#ifdef AYIN_PLATFORM_WINDOWS

extern Ayin::Application* Ayin::CreatApplication();//客户端侧实现该方法


//启动流程由引擎侧控制
int main(int argc, char** argv) {

	Ayin::Application* app = Ayin::CreatApplication();
	app->Run();
	delete app;

}

#endif

