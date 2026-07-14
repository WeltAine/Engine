#include <AyinPch.h>

#include <Ayin.h>

#include "SnakeLayer.h"

#include "Ayin/Core/EntryPoint.h"

class SnakeApplication : public Ayin::Application {

public:
    SnakeApplication() {

        // Application 管理整个程序生命周期；SnakeLayer 承载贪吃蛇的输入、更新和渲染逻辑。
        PushLayer(new SnakeLayer{});

    }

    ~SnakeApplication() override = default;

};

Ayin::Application* Ayin::CreatApplication() {

    // 引擎入口点会调用这个函数，由客户端项目决定具体创建哪个 Application。
    return new SnakeApplication();

}
