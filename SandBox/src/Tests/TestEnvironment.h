#pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ranges>
#include <string>

namespace SandBoxTests {
	inline constexpr const char* LayerSelectionVariable = "AYIN_SANDBOX_TEST_LAYER";
	inline constexpr const char* AutoExitVariable = "AYIN_SANDBOX_TEST_AUTO_EXIT";

	inline std::string ReadEnvironmentVariable(const char* name) {
		char* value = nullptr;
		std::size_t valueSize = 0;
		if (_dupenv_s(&value, &valueSize, name) != 0 || value == nullptr) {
			// 这是 Microsoft CRT 提供的环境变量读取函数，参数是 buffer，bufferSize，环境变量名称
			// _dupenv_s 需要修改 value 本身，让它指向新分配的内存。所以 &value
			return {};
		}

		std::string result{ value };
		std::free(value);
		return result;
	}


	// 把从环境变量读取到的字符串转换成统一、便于比较的格式
	inline std::string NormalizeName(std::string name) {

		// 删除左侧空白
		name.erase(name.begin(), std::ranges::find_if(name, [](const unsigned char character) {
			return !std::isspace(character);
		}));

		// 删除右侧空白
		name.erase(std::ranges::find_if(name | std::views::reverse, [](const unsigned char character) {
			return !std::isspace(character);
		}).base(), name.end());

		// 全部转成小写
		std::ranges::transform(name, name.begin(), [](const unsigned char character) {
			return static_cast<char>(std::tolower(character));
		});
		return name;
	}

	inline bool IsTruthyEnvironmentVariable(const char* name) {
		const std::string value = NormalizeName(ReadEnvironmentVariable(name));
		return !value.empty() && value != "0" && value != "false" && value != "off";
	}
}
