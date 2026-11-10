#include "commandparser.hpp"

namespace mychat {
	void CommandParser::register_command(std::string name, std::function<void(const std::vector<std::string>&)> handler) {
		m_commands[name] = handler;
	}

	bool CommandParser::parse(const std::string& line) {
		if (line.empty()) return false;

        std::stringstream ss(line);
        std::string command_name;
        ss >> command_name;

        std::vector<std::string> args;
        std::string arg;
        while (ss >> arg) {
            args.push_back(arg);
        }

        auto it = m_commands.find(command_name);
        if (it != m_commands.end()) {
            it->second(args);
            return true;
        }

        return false;
	}
}