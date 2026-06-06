#pragma once

#include <Ayin.h>

#include "Ayin/Scene/ComponentRegistry.h"

namespace Ayin {

	class PropertiesPanel {

	public:

		PropertiesPanel() = default;

		inline void SetContext(const Entity& entity) { m_SelectedEntity = entity; }

		inline const Entity& GetSelectedEntity() const { return m_SelectedEntity; }

		void OnImGuiRender();

	private:

		void DrawComponentHeader(const ComponentDescriptor& desc);

		void DrawAddComponentSearch();

	private:

		Entity m_SelectedEntity;

	};

}

// 光标 / 布局位置
//			API										说明
// 
// GetCursorPos()										光标位置
// GetCursorPosX() / Y()								光标 X / Y 分量
// GetCursorScreenPos()								光标位置
// GetCursorStartPos()									本行起始光标位置
// SetCursorPos(ImVec2)								设光标位置
// SetCursorScreenPos(ImVec2)							设光标位置
// SetCursorPosX(float)								设光标 X
// 
// -- -
// 
// 区域 / 尺寸
//			API										说明
// 
// GetContentRegionAvail()								内容区剩余可用空间(宽, 高)
// GetContentRegionMax()								内容区右下角
// GetWindowContentRegionMin() / Max()					整个窗口内容区的边界（包含已有内容占用的部分）
// GetWindowWidth() / Height()							窗口整体宽高
// GetWindowPos() / Size()								窗口位置和尺寸
// GetFrameHeight()									标准控件框架高度。比如这些控件通常都算 frame：ImGui::Button(...)ImGui::RadioButton(...)ImGui::Checkbox(...)ImGui::InputFloat(...)ImGui::SliderFloat(...)它们的高度大致由当前字体和样式内边距决定：FrameHeight = FontSize + FramePadding.y * 2所以它不是提前知道“某个具体按钮最终有多宽”，而是根据当前 ImGui 样式知道“标准控件高度应该是多少”。但宽度一般要自己算，因为按钮文字不同。
// GetFrameHeightWithSpacing()							框架高 + ItemSpacing.y
// CalcItemWidth()										计算当前上下文下单个控件的推荐宽度
// PushMultiItemsWidths(int count, float total_width)		为接下来的 N 个控件预先分配相等的宽度，并将这些宽度压入宽度栈(必须为每个控件调用一次ImGui::PopItemWidth())
// -- -
// 
// 上一个控件的反查
//			API										说明
// 
// GetItemRectMin() / Max()								上一个控件的包围盒（屏幕坐标）
// GetItemRectSize()									上一个控件的尺寸
// IsItemHovered()										鼠标悬停在上一控件上？
// IsItemClicked()										上一控件被点击？
// IsItemActive()										上一控件正在被按着（拖拽中）？
// IsItemFocused()										上一控件有键盘焦点？
// IsItemVisible()										上一控件在可视范围内？
// 
// -- -
// 
// ID 管理
//			API										说明
// 
// PushID(int / str / ptr)								推一个新层级到 ID 栈
// PopID()											弹出一个层级
// GetID(str)											根据当前 ID 栈 + 字符串算一个唯一 ID
// 
// -- -
// 
// 样式 / 颜色
//			API										说明
// 
// GetStyle()									全局样式表（颜色、间距、圆角…）
// GetColorU32(ImGuiCol_xxx)					把枚举颜色转成 32 位色值
// PushStyleColor(col, color)					临时改某种颜色
// PopStyleColor()								恢复颜色
// PushStyleVar(var, val)						临时改样式参数（如 ItemSpacing）
// PopStyleVar()								恢复样式参数
// 
// -- -
// 
// 绘图（DrawList）
//			API										说明
// 
// GetWindowDrawList()							当前窗口的 DrawList
// GetForegroundDrawList()						最顶层的 DrawList（覆盖在所有东西上面）
// GetBackgroundDrawList()						最底层的 DrawList
// DrawList 常用画法：AddLine、AddRect、AddRectFilled、AddCircle、AddTriangle、AddText、AddImage。
// 
// -- -
// 
// 常用 Is * 查询
//			API										说明
// 
// IsWindowHovered()							当前窗口被鼠标悬停？
// IsWindowFocused()							当前窗口有焦点？
// IsMouseDown(btn)							某鼠标键按下中？
// IsKeyDown(key)								某键盘键按下中？
// IsKeyPressed(key)							某键本帧刚按下？
//
// -- -
//
// 全局
// GImGui										ImGui 的全局上下文指针，指向所有全局状态（IO、样式、字体等）
// GImGui->Font									当前正在使用的字体对象
// GImGui->Font->FontSize						获取当前激活字体的字号大小（单位像素），FontSize：该字体的字号（如 16.0f 表示 16 像素高）
//GImGui->Style.FramePadding.y					框架控件（按钮、输入框、滑块等）的内边距（内容与边框之间的距离）（直接读取样式中的预定义值）
//
// -- -
// 
// 样式
// PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 })			修改 ImGui 的全局样式变量，并将修改压入样式栈


