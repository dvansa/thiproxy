#include "thiproxy/server.hpp"
#include <iostream>

int main(int argc, char ** argv)
{
	std::cout << "ThiProxy" << std::endl;

	thiproxy::Server server(9000);
	server.start();

	return 0;
}