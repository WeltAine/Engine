#pragma once

#include <Ayin.h>

#include <random>
#include <vector>

class SnakeLayer : public Ayin::Layer {

public:
    SnakeLayer();
    ~SnakeLayer() override = default;

    void OnAttach() override;                         // Layer 被加入层栈时调用，适合初始化资源
    void OnDetach() override;                         // Layer 从层栈移除时调用，适合释放资源
    void OnUpdate(Ayin::Timestep deltaTime) override; // 每帧调用：处理输入、推进游戏、发起渲染
    void OnImGuiRender() override;                    // 每帧绘制 ImGui 调试/说明窗口
    void OnEvent(Ayin::Event& event) override;        // 接收窗口、鼠标、键盘等事件

private:
    // 蛇当前移动方向。棋盘坐标中 x 向右增长，y 向上增长。
    enum class Direction {
        Up,
        Down,
        Left,
        Right
    };

    // 游戏状态决定 OnUpdate 是否继续推进蛇移动，也决定 ImGui 显示的提示。
    enum class GameState {
        Playing,
        Paused,
        GameOver,
        Won
    };

private:
    void ResetGame();                                    // 重置蛇、分数、状态和第一颗食物
    void HandleInput();                                  // 轮询键盘输入，更新方向/暂停/重开
    void StepGame();                                     // 按固定步长推进一格，是游戏规则的核心
    void SpawnFood();                                    // 在非蛇身位置随机生成食物
    void RenderBoard();                                  // 使用 Renderer2D 绘制棋盘背景和格子
    void RenderSnake();                                  // 使用 Renderer2D 绘制蛇身、蛇头和眼睛
    void RenderFood();                                   // 使用 Renderer2D 绘制食物和高光

    bool TryQueueDirection(Direction direction);         // 尝试缓存下一步方向，禁止直接掉头
    bool IsSnakeCell(const glm::ivec2& cell, size_t snakeLength) const; // 判断某个格子是否被蛇身占用
    glm::ivec2 DirectionToVector(Direction direction) const;            // 把方向转换成棋盘坐标增量
    glm::vec3 CellToWorld(const glm::ivec2& cell, float z = -1.0f) const;// 把棋盘格子坐标转换到世界坐标

    static bool IsSameCell(const glm::ivec2& first, const glm::ivec2& second); // 判断两个棋盘坐标是否相同
    static bool AreOpposite(Direction first, Direction second);                // 判断两个方向是否相反
    static const char* GetGameStateName(GameState state);                     // 把状态转换成 ImGui 可显示文本

private:
    Ayin::CameraController m_CameraController; // 正交相机控制器；这里主要使用其相机矩阵和窗口尺寸响应

    std::vector<glm::ivec2> m_Snake;           // 蛇身格子队列：front 是蛇头，back 是蛇尾
    glm::ivec2 m_Food{ 0, 0 };                 // 食物所在棋盘格子

    Direction m_Direction = Direction::Right;       // 当前已经生效的移动方向
    Direction m_QueuedDirection = Direction::Right;  // 玩家输入后缓存的下一步方向
    GameState m_GameState = GameState::Playing;     // 当前游戏状态

    std::mt19937 m_RandomEngine;               // 随机数引擎，用于生成食物位置
    float m_MoveAccumulator = 0.0f;             // 移动时间累加器，让蛇按固定间隔移动，而不是每帧移动
    int m_Score = 0;                            // 当前分数，吃到一个食物加 1
    int m_BestScore = 0;                        // 本次运行中的最高分
    bool m_WasPausePressed = false;             // 上一帧空格是否按下，用于把长按转换成单次触发
    bool m_WasResetPressed = false;             // 上一帧 R 是否按下，用于把长按转换成单次触发

};
