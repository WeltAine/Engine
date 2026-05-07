-- 针对解决方案级文件实现 solution_items 命令
require('vstudio') -- 显式加载vstudio模块，否则premake.vstudio.sln2005在beta版中为nil

-- 创建premakeAPI，注册solution_items命令
premake.api.register {
	name = "solution_items",	--API名称
	scope = "workspace",		--作用域，表示该命令只能在workspace块内使用
	kind = "string",			--元素类型，表示列表中的每一项都是字符串
	list = true,				--标记为列表类型，旧版用"list:string"，beta版拆分为kind+list
}

--重写vstudio.sln2005.projects函数，在生成解决方案文件时添加solution_items
premake.override(premake.vstudio.sln2005, "projects", function(base, wks)
	if wks.solution_items and #wks.solution_items > 0 then
		local solution_folder_GUID = "{2150E333-8FDC-42A3-9474-1A3956D46DE8}" -- See https://www.codeproject.com/Reference/720512/List-of-Visual-Studio-Project-Type-GUIDs
		premake.push("Project(\"" .. solution_folder_GUID .. "\") = \"Solution Items\", \"Solution Items\", \"{" .. os.uuid("Solution Items:" .. wks.name) .. "}\"")
		premake.push("ProjectSection(SolutionItems) = preProject")

		for _, path in ipairs(wks.solution_items) do
			premake.w(path .. " = " .. path)
			--让这个文件出现在 VS 解决方案资源管理器里，方便你双击打开编辑。
			-- 你会在编辑器中看到一个虚假的文件夹
			-- Solution Items
			-- 		|
			--  	—— .editorconfig
		end

		premake.pop("EndProjectSection")
		premake.pop("EndProject")
	end
	--生成的内容大概是这样的：
	--Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Solution Items", "Solution Items", "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}"
    --ProjectSection(SolutionItems) = preProject
    --    .editorconfig = .editorconfig
    --EndProjectSection
    --EndProject
	base(wks)
end)

-- push和pop函数用于写入sln文件，push表示开始一个新的块，pop表示结束当前块（用缩进来区分块）。通过调用base(wks)来继续执行原来的projects函数，以确保生成的sln文件包含所有项目和solution_items。
