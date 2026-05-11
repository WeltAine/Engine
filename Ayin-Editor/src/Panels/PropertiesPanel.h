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
// GetCursorPos()								光标位置
// GetCursorPosX() / Y()						光标 X / Y 分量
// GetCursorScreenPos()							光标位置
// GetCursorStartPos()							本行起始光标位置
// SetCursorPos(ImVec2)							设光标位置
// SetCursorScreenPos(ImVec2)					设光标位置
// SetCursorPosX(float)							设光标 X
// 
// -- -
// 
// 区域 / 尺寸
//			API										说明
// 
// GetContentRegionAvail()						内容区剩余可用空间(宽, 高)
// GetContentRegionMax()						内容区右下角
// GetWindowContentRegionMin() / Max()			整个窗口内容区的边界（包含已有内容占用的部分）
// GetWindowWidth() / Height()					窗口整体宽高
// GetWindowPos() / Size()						窗口位置和尺寸
// GetFrameHeight()								标准控件框架高度
// GetFrameHeightWithSpacing()					框架高 + ItemSpacing.y
// 
// -- -
// 
// 上一个控件的反查
//			API										说明
// 
// GetItemRectMin() / Max()						上一个控件的包围盒（屏幕坐标）
// GetItemRectSize()							上一个控件的尺寸
// IsItemHovered()								鼠标悬停在上一控件上？
// IsItemClicked()								上一控件被点击？
// IsItemActive()								上一控件正在被按着（拖拽中）？
// IsItemFocused()								上一控件有键盘焦点？
// IsItemVisible()								上一控件在可视范围内？
// 
// -- -
// 
// ID 管理
//			API										说明
// 
// PushID(int / str / ptr)						推一个新层级到 ID 栈
// PopID()										弹出一个层级
// GetID(str)									根据当前 ID 栈 + 字符串算一个唯一 ID
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
// IsMouseDown(btn)								某鼠标键按下中？
// IsKeyDown(key)								某键盘键按下中？
// IsKeyPressed(key)							某键本帧刚按下？
