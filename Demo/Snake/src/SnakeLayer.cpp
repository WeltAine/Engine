#include <AyinPch.h>

#include "SnakeLayer.h"

#include <algorithm>

namespace {

    // 这一组常量描述棋盘尺寸和移动节奏。放在匿名命名空间中，只在本 cpp 文件内可见。
    constexpr int s_BoardWidth = 24;
    constexpr int s_BoardHeight = 18;
    constexpr float s_CellSize = 0.58f;
    constexpr float s_CellGap = 0.035f;
    constexpr float s_MoveInterval = 0.12f;
    constexpr glm::vec3 s_ZeroRotation = { 0.0f, 0.0f, 0.0f };

    // 这一组颜色控制整体视觉风格。所有绘制都通过 Renderer2D::DrawQuad 完成。
    constexpr glm::vec4 s_BackgroundColor = { 0.025f, 0.032f, 0.045f, 1.0f };
    constexpr glm::vec4 s_BoardColor = { 0.055f, 0.075f, 0.095f, 1.0f };
    constexpr glm::vec4 s_BoardAccentColor = { 0.073f, 0.098f, 0.120f, 1.0f };
    constexpr glm::vec4 s_BorderColor = { 0.170f, 0.260f, 0.240f, 1.0f };
    constexpr glm::vec4 s_SnakeHeadColor = { 0.310f, 0.960f, 0.470f, 1.0f };
    constexpr glm::vec4 s_SnakeBodyColor = { 0.080f, 0.720f, 0.370f, 1.0f };
    constexpr glm::vec4 s_SnakeBodyAltColor = { 0.100f, 0.620f, 0.340f, 1.0f };
    constexpr glm::vec4 s_SnakeEyeColor = { 0.020f, 0.050f, 0.035f, 1.0f };
    constexpr glm::vec4 s_FoodColor = { 1.000f, 0.240f, 0.220f, 1.0f };
    constexpr glm::vec4 s_FoodHighlightColor = { 1.000f, 0.660f, 0.420f, 1.0f };

    glm::vec3 QuadSize(float width, float height) {

        // Renderer2D 的 DrawQuad 需要 vec3 尺寸；2D 游戏中 z 尺寸固定为 1 即可。
        return { width, height, 1.0f };

    }

}

SnakeLayer::SnakeLayer()
    : Ayin::Layer("SnakeLayer"),
    m_CameraController(Ayin::CameraProp{ .Type = Ayin::Camera::CameraType::Orthogonal, .Height = 12.2f }),
    m_RandomEngine(std::random_device{}())
{

    ResetGame();

}

void SnakeLayer::OnAttach() {

    AYIN_PROFILE_FUNCTION();

}

void SnakeLayer::OnDetach() {

    AYIN_PROFILE_FUNCTION();

}

void SnakeLayer::OnUpdate(Ayin::Timestep deltaTime) {

    AYIN_PROFILE_FUNCTION();

    
    // 1. 输入阶段：先读取玩家输入，更新缓存方向、暂停或重开状态。
    HandleInput();

    // 2. 规则阶段：用固定时间步推进蛇移动，避免不同帧率下移动速度不同。
    if (m_GameState == GameState::Playing) {
        m_MoveAccumulator += deltaTime.GetSeconds();

        while (m_MoveAccumulator >= s_MoveInterval && m_GameState == GameState::Playing) {
            m_MoveAccumulator -= s_MoveInterval;
            StepGame();
        }
    }

    // 3. 清屏阶段：先清掉上一帧画面，再开始本帧 2D 批渲染。
    Ayin::RenderCommand::SetClearColor(s_BackgroundColor);
    Ayin::RenderCommand::Clear();
    Ayin::Renderer2D::ResetStatistics();
    Ayin::Renderer2D::BeginScene(m_CameraController.GetCamera());

    // 4. 绘制阶段：绘制顺序从背景到前景，z 值也按背景更远、蛇和食物更近来设置。
    RenderBoard();
    RenderFood();
    RenderSnake();

    // 5. 提交阶段：EndScene 会把前面 DrawQuad 收集的批渲染数据真正提交给 GPU。
    Ayin::Renderer2D::EndScene();

}

void SnakeLayer::OnImGuiRender() {

    Ayin::Renderer2D::Statistics statistics = Ayin::Renderer2D::GetStatistics();

    ImGui::Begin("Snake Demo");
    ImGui::Text("State: %s", GetGameStateName(m_GameState));
    ImGui::Text("Score: %d", m_Score);
    ImGui::Text("Best: %d", m_BestScore);
    ImGui::Separator();
    ImGui::TextUnformatted("Move: WASD / Arrow Keys");
    ImGui::TextUnformatted("Pause: Space");
    ImGui::TextUnformatted("Restart: R");
    ImGui::Separator();
    ImGui::Text("Snake Length: %d", static_cast<int>(m_Snake.size()));
    ImGui::Text("Draw Calls: %d", statistics.DrawCalls);
    ImGui::Text("Quads: %d", statistics.QuadCount);

    if (m_GameState == GameState::Paused) {
        ImGui::TextColored(ImVec4{ 1.0f, 0.86f, 0.36f, 1.0f }, "Paused. Press Space to continue.");
    }
    else if (m_GameState == GameState::GameOver) {
        ImGui::TextColored(ImVec4{ 1.0f, 0.25f, 0.22f, 1.0f }, "Game Over. Press R to restart.");
    }
    else if (m_GameState == GameState::Won) {
        ImGui::TextColored(ImVec4{ 0.30f, 1.0f, 0.45f, 1.0f }, "Board cleared. Press R to play again.");
    }

    ImGui::End();

}

void SnakeLayer::OnEvent(Ayin::Event& event) {

    // 游戏本身使用轮询输入；事件这里只交给相机控制器处理窗口尺寸变化等相机相关事件。
    m_CameraController.OnEvent(event);

}

void SnakeLayer::ResetGame() {

    // 初始蛇身放在棋盘中心，头朝右，后续元素依次是身体和尾巴。
    m_Snake.clear();

    const glm::ivec2 center = { s_BoardWidth / 2, s_BoardHeight / 2 };
    m_Snake.push_back(center);
    m_Snake.push_back({ center.x - 1, center.y });
    m_Snake.push_back({ center.x - 2, center.y });
    m_Snake.push_back({ center.x - 3, center.y });

    m_Direction = Direction::Right;
    m_QueuedDirection = Direction::Right;
    m_GameState = GameState::Playing;
    m_MoveAccumulator = 0.0f;
    m_Score = 0;
    m_WasPausePressed = false;
    m_WasResetPressed = false;

    SpawnFood();

}

void SnakeLayer::HandleInput() {

    // 方向键和 WASD 都映射到同一套方向。TryQueueDirection 会阻止蛇直接反向掉头。
    if (Ayin::Input::IsKeyPressed(AYIN_KEY_W) || Ayin::Input::IsKeyPressed(AYIN_KEY_UP)) {
        TryQueueDirection(Direction::Up);
    }
    else if (Ayin::Input::IsKeyPressed(AYIN_KEY_S) || Ayin::Input::IsKeyPressed(AYIN_KEY_DOWN)) {
        TryQueueDirection(Direction::Down);
    }
    else if (Ayin::Input::IsKeyPressed(AYIN_KEY_A) || Ayin::Input::IsKeyPressed(AYIN_KEY_LEFT)) {
        TryQueueDirection(Direction::Left);
    }
    else if (Ayin::Input::IsKeyPressed(AYIN_KEY_D) || Ayin::Input::IsKeyPressed(AYIN_KEY_RIGHT)) {
        TryQueueDirection(Direction::Right);
    }

    // 空格暂停使用“边沿触发”：只有从未按下变成按下的那一帧才切换状态。
    const bool pausePressed = Ayin::Input::IsKeyPressed(AYIN_KEY_SPACE);
    if (pausePressed && !m_WasPausePressed) {
        if (m_GameState == GameState::Playing) {
            m_GameState = GameState::Paused;
        }
        else if (m_GameState == GameState::Paused) {
            m_GameState = GameState::Playing;
            m_MoveAccumulator = 0.0f;
        }
    }
    m_WasPausePressed = pausePressed;

    // R 键重开也使用边沿触发，避免按住 R 时每一帧都 ResetGame。
    const bool resetPressed = Ayin::Input::IsKeyPressed(AYIN_KEY_R);
    if (resetPressed && !m_WasResetPressed) {
        ResetGame();
    }
    m_WasResetPressed = resetPressed;

}

void SnakeLayer::StepGame() {

    // 本步开始时才让缓存方向生效，保证一帧内多次输入不会立刻多走几格。
    m_Direction = m_QueuedDirection;

    const glm::ivec2 newHead = m_Snake.front() + DirectionToVector(m_Direction);
    if (newHead.x < 0 || newHead.x >= s_BoardWidth || newHead.y < 0 || newHead.y >= s_BoardHeight) {
        m_GameState = GameState::GameOver;
        return;
    }

    // 如果没吃到食物，本步蛇尾会移动，所以检测自撞时可以排除最后一节尾巴。
    const bool willGrow = IsSameCell(newHead, m_Food);
    const size_t collisionLength = willGrow ? m_Snake.size() : m_Snake.size() - 1;
    if (IsSnakeCell(newHead, collisionLength)) {
        m_GameState = GameState::GameOver;
        return;
    }

    // 队列变化代表蛇移动：新头插入 front，没吃到食物时再移除 back 作为尾巴前进。
    m_Snake.insert(m_Snake.begin(), newHead);

    if (willGrow) {
        m_Score++;
        m_BestScore = std::max(m_BestScore, m_Score);
        SpawnFood();
    }
    else {
        m_Snake.pop_back();
    }

}

void SnakeLayer::SpawnFood() {

    // 蛇占满整个棋盘时已经没有可生成食物的位置，游戏胜利。
    if (m_Snake.size() >= static_cast<size_t>(s_BoardWidth * s_BoardHeight)) {
        m_GameState = GameState::Won;
        return;
    }

    std::uniform_int_distribution<int> randomX(0, s_BoardWidth - 1);
    std::uniform_int_distribution<int> randomY(0, s_BoardHeight - 1);

    do {
        m_Food = { randomX(m_RandomEngine), randomY(m_RandomEngine) };
    } while (IsSnakeCell(m_Food, m_Snake.size()));

}

void SnakeLayer::RenderBoard() {

    const float boardWidth = static_cast<float>(s_BoardWidth) * s_CellSize;
    const float boardHeight = static_cast<float>(s_BoardHeight) * s_CellSize;
    const float borderThickness = 0.16f;

    // 先画比棋盘略大的边框，再画棋盘底色。
    Ayin::Renderer2D::DrawQuad(
        { 0.0f, 0.0f, -4.0f },
        s_ZeroRotation,
        QuadSize(boardWidth + borderThickness * 2.0f, boardHeight + borderThickness * 2.0f),
        s_BorderColor);

    Ayin::Renderer2D::DrawQuad(
        { 0.0f, 0.0f, -3.0f },
        s_ZeroRotation,
        QuadSize(boardWidth, boardHeight),
        s_BoardColor);

    // 用深浅交替的小方块表现棋盘格，每一个格子都是一个 DrawQuad。
    for (int y = 0; y < s_BoardHeight; y++) {
        for (int x = 0; x < s_BoardWidth; x++) {
            const glm::vec4 color = ((x + y) % 2 == 0) ? s_BoardColor : s_BoardAccentColor;
            Ayin::Renderer2D::DrawQuad(
                CellToWorld({ x, y }, -2.0f),
                s_ZeroRotation,
                QuadSize(s_CellSize - s_CellGap, s_CellSize - s_CellGap),
                color);
        }
    }

}

void SnakeLayer::RenderSnake() {

    // m_Snake[0] 是蛇头，其余元素是身体。身体用交替颜色让长度更容易观察。
    for (size_t index = 0; index < m_Snake.size(); index++) {
        const bool isHead = index == 0;
        const glm::vec4 color = isHead ? s_SnakeHeadColor : (index % 2 == 0 ? s_SnakeBodyColor : s_SnakeBodyAltColor);
        const float size = isHead ? s_CellSize * 0.88f : s_CellSize * 0.80f;

        Ayin::Renderer2D::DrawQuad(
            CellToWorld(m_Snake[index], isHead ? 0.0f : -0.5f),
            s_ZeroRotation,
            QuadSize(size, size),
            color);
    }

    if (m_Snake.empty()) {
        return;
    }

    // 眼睛的位置由“前进方向”和“侧方向”共同决定，因此蛇转向后眼睛也会朝向对应方向。
    const glm::vec3 headPosition = CellToWorld(m_Snake.front(), 0.5f);
    const glm::ivec2 facing = DirectionToVector(m_Direction);
    const glm::vec2 side = { -static_cast<float>(facing.y), static_cast<float>(facing.x) };
    const glm::vec2 forward = { static_cast<float>(facing.x), static_cast<float>(facing.y) };

    for (float sideSign : { -1.0f, 1.0f }) {
        const glm::vec2 offset = forward * (s_CellSize * 0.18f) + side * (sideSign * s_CellSize * 0.18f);
        Ayin::Renderer2D::DrawQuad(
            { headPosition.x + offset.x, headPosition.y + offset.y, headPosition.z },
            s_ZeroRotation,
            QuadSize(s_CellSize * 0.12f, s_CellSize * 0.12f),
            s_SnakeEyeColor);
    }

}

void SnakeLayer::RenderFood() {

    // 食物主体和高光都是小方块，依然只使用 Renderer2D 的 DrawQuad。
    Ayin::Renderer2D::DrawQuad(
        CellToWorld(m_Food, -0.2f),
        s_ZeroRotation,
        QuadSize(s_CellSize * 0.78f, s_CellSize * 0.78f),
        s_FoodColor);

    const glm::vec3 foodPosition = CellToWorld(m_Food, 0.2f);
    Ayin::Renderer2D::DrawQuad(
        { foodPosition.x + s_CellSize * 0.12f, foodPosition.y + s_CellSize * 0.12f, foodPosition.z },
        s_ZeroRotation,
        QuadSize(s_CellSize * 0.24f, s_CellSize * 0.24f),
        s_FoodHighlightColor);

}

bool SnakeLayer::TryQueueDirection(Direction direction) {

    if (AreOpposite(direction, m_Direction)) {
        return false;
    }

    m_QueuedDirection = direction;
    return true;

}

bool SnakeLayer::IsSnakeCell(const glm::ivec2& cell, size_t snakeLength) const {

    // snakeLength 让调用者可以只检测前 N 节，例如移动时排除即将离开的尾巴。
    const size_t count = std::min(snakeLength, m_Snake.size());
    for (size_t index = 0; index < count; index++) {
        if (IsSameCell(cell, m_Snake[index])) {
            return true;
        }
    }

    return false;

}

glm::ivec2 SnakeLayer::DirectionToVector(Direction direction) const {

    switch (direction) {
        case Direction::Up:
            return { 0, 1 };
        case Direction::Down:
            return { 0, -1 };
        case Direction::Left:
            return { -1, 0 };
        case Direction::Right:
            return { 1, 0 };
    }

    return { 1, 0 };

}

glm::vec3 SnakeLayer::CellToWorld(const glm::ivec2& cell, float z) const {

    // 棋盘坐标以左下角为 (0,0)；世界坐标以棋盘中心为原点，方便相机居中观察。
    return {
        (static_cast<float>(cell.x) - static_cast<float>(s_BoardWidth) * 0.5f + 0.5f) * s_CellSize,
        (static_cast<float>(cell.y) - static_cast<float>(s_BoardHeight) * 0.5f + 0.5f) * s_CellSize,
        z
    };

}

bool SnakeLayer::IsSameCell(const glm::ivec2& first, const glm::ivec2& second) {

    return first.x == second.x && first.y == second.y;

}

bool SnakeLayer::AreOpposite(Direction first, Direction second) {

    return (first == Direction::Up && second == Direction::Down)
        || (first == Direction::Down && second == Direction::Up)
        || (first == Direction::Left && second == Direction::Right)
        || (first == Direction::Right && second == Direction::Left);

}

const char* SnakeLayer::GetGameStateName(GameState state) {

    switch (state) {
        case GameState::Playing:
            return "Playing";
        case GameState::Paused:
            return "Paused";
        case GameState::GameOver:
            return "Game Over";
        case GameState::Won:
            return "Won";
    }

    return "Unknown";

}
