#include "server/server.hpp"
#include "cmd_parser/commandparser.hpp"

using namespace mychat;

int main() {
	
	Server server;
	server.pollConnection();

	return 0;
}