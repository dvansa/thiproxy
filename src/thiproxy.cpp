/*
* thiproxy main program.
* Initializes and runs thin proxy server on specific port (default 9000).
*
*
* Copyright (C) 2018 Daniel van Sabben Alsina
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#include "thiproxy/server.hpp"
#include <iostream>

int main(int argc, char ** argv)
{
	std::cout << "------------------------" << std::endl;
	std::cout << "        ThiProxy" << std::endl;
	std::cout << "------------------------" << std::endl;

	std::cout << "Initializing..." << std::endl;

	int port = 9000;
	if(argc > 1)
		port = atoi(argv[1]);
	
	thiproxy::Server server(port);

	std::cout  << "Serving..." << std::endl;

	server.start();

	return 0;
}