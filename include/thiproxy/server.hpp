/*
* Thiproxy server.
* Proxy server. Listens to fixed port and starts Sessions for
* each incoming connection.
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
#ifndef THIPROXY_SERVER
#define THIPROXY_SERVER

//std
#include <string>
#include <iostream>
#include <set>

//boost
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>

//thiproxy
#include "thiproxy/session.hpp"

namespace thiproxy
{
	//Proxy server
	class Server
	{
	public:
		// Setup server on specific port
		Server(int port = 9000);

		//Starts server (blocking)
		void start();

		//Stops server
		void stop();

	private:

		//Connection accepted calback
		void callback_accept(const boost::system::error_code & error);

		//Clean finished sessions
		void clean_sessions();

		bool _running;
		int _port;

		boost::asio::io_service _io_service;
		boost::asio::ip::tcp::endpoint _endpoint;
		boost::shared_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
		
		boost::asio::ip::tcp::socket _socket;
		std::set<boost::shared_ptr<thiproxy::Session>> _sessions;
	};
}

#endif