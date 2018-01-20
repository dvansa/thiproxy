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
#include <memory>


//Example 1: Forwards all HTTP request and prints the request line on console
class SessionControllerForwardLog : public thiproxy::SessionController
{
public:
	SessionControllerForwardLog(){}

	thiproxy::SessionAction on_down_request(const thiproxy::HttpMessage & down_msg) override
	{
		thiproxy::SessionAction res;

		//Forward message
		res.type = thiproxy::SessionAction::ACTION_FORWARD;
		res.message = down_msg;

		//Print request line
		std::cout << ">> HTTP Request <<" << std::endl;
		std::cout << "\t" << down_msg.header.request() << std::endl;

		return res;
	}
};

//Example 2: Forwards all HTTP request and closes connection when requesting github
class SessionControllerCloseOnGithub : public thiproxy::SessionController
{
public:
	SessionControllerCloseOnGithub(){}

	thiproxy::SessionAction on_down_request(const thiproxy::HttpMessage & down_msg) override
	{
		thiproxy::SessionAction res;

		
		if(down_msg.header.url().find("github.") != std::string::npos)
		{
			//Close connection if request has github url
			res.type = thiproxy::SessionAction::ACTION_CLOSE_CONNECTION;
		}
		else
		{
			//Forward all other messages
			res.type = thiproxy::SessionAction::ACTION_FORWARD;
			res.message = down_msg;
		}

		return res;
	}
};

//Example 3: Returns always a simple HTML page
class SessionControllerSimpleHtml : public thiproxy::SessionController
{
public:
	SessionControllerSimpleHtml(){}

	thiproxy::SessionAction on_down_request(const thiproxy::HttpMessage & down_msg) override
	{
		thiproxy::SessionAction res;

		//User message
		res.type = thiproxy::SessionAction::ACTION_USER_MESSAGE;

		//Content : html
		std::string html_str;
		html_str += "<html>";
		html_str += "<head><title>Simple Page</title></head>";
		html_str += "<body>This is a simple HTML page.</body>";
		html_str += "</html>";

		//Header
		std::string header_str = "HTTP/1.1 200 OK\r\n";
		header_str += "Connection:close\r\n";
		header_str += "Content-Type:text/html;\r\n";
		header_str += "Content-Length:" + std::to_string(html_str.size()) + "\r\n";
		header_str += "\r\n";

		//Create response message
		thiproxy::HttpHeader header;
		header.from_buffer(header_str);
		res.message = thiproxy::HttpMessage(thiproxy::HttpMessage::HTTP_RESPONSE, header, thiproxy::HttpBuffer(html_str.begin(), html_str.end()) );

		return res;
	}

};

int main(int argc, char ** argv)
{
	std::cout << "------------------------" << std::endl;
	std::cout << "        ThiProxy" << std::endl;
	std::cout << "------------------------" << std::endl;

	//Help msg
	if(argc > 1 && (std::string(argv[1]) == "help" || std::string(argv[1]) == "--help") )
	{
		std::cout << "usage: thiproxy.exe [0,1,2] <port>" << std::endl;
		return 0;
	}

	std::cout << "Initializing..." << std::endl;

	//Example session controller id [0,1,2]
	int controller_id = 0;
	if(argc > 1 ) 
	{
		controller_id = atoi(argv[1]);
		controller_id = std::min(2, std::max(0,controller_id) );
	}

	std::shared_ptr<thiproxy::SessionController> controller;

	//Select one example controllers
	std::cout << "SessionController : " << std::flush;
	if(controller_id == 0)
	{
		std::cout << "Forward Log" << std::endl;
		controller = std::shared_ptr<thiproxy::SessionController>( new SessionControllerForwardLog());
	}
	else if(controller_id == 1)
	{
		std::cout << "Close on Github" << std::endl;
		controller = std::shared_ptr<thiproxy::SessionController>(new SessionControllerCloseOnGithub());
	}
	else if(controller_id == 2)
	{
		std::cout << "Simple HTML" << std::endl;
		controller = std::shared_ptr<thiproxy::SessionController>(new SessionControllerSimpleHtml());
	}

	//Read port
	int port = 9000;
	if(argc > 2)
		port = atoi(argv[2]);

	std::cout << "Port : " << std::to_string(port) << std::endl;
	
	//Create server
	thiproxy::Server server(port);

	//Assign Session controller
	server.assign_session_controller(controller.get());

	std::cout  << "Serving..." << std::endl;

	//Run
	server.start();

	return 0;
}