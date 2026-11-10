#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>

namespace mychat {
	class CommandParser {
	private:
		std::unordered_map <std::string, std::function<void(const std::vector<std::string>&)>> m_commands;
	public:
		void register_command(std::string name, std::function<void(const std::vector<std::string>&)>);

		bool parse(const std::string& line);
	};
}