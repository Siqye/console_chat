#include "server/server.hpp"
#include "cmd_parser/commandparser.hpp"

using namespace mychat;

int main() {
	CommandParser cp;


	Server server;
	server.pollConnection();

	server.poll();

	return 0;
}