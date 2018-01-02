/*
* Session
* Handles server single connetion
* Basic proxy functionallity forwarding HTTP requests and keeping an established connection
* for HTTP "CONNECT" requests (https)
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
#ifndef THIPROXY_SESSION
#define THIPROXY_SESSION

//std
#include <string>
#include <iostream>
#include <exception>

//boost
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/array.hpp>

//thiproxy
#include "thiproxy/http.hpp"

#define THIPROXY_SESSION_BUFFER_LEN 4096

namespace thiproxy
{
	//
	// Proxy Session.
	// Starts on accepted connection.
	// Bridges browser HTTP requests (DOWN connection) to remote destination (UP connection)
	// Also, establishes plain TCP bridge for HTTP CONNECT requests (https://)
	class Session : public boost::enable_shared_from_this<Session>
	{
	public:

		//Status
		enum
		{
			STATUS_READ_HEADER, //Reading HTTP header from DOWN
			STATUS_CONNECTING, // Connecting to UP
			STATUS_CONNECTED, // Connected
			STATUS_READ_HEADER_UP // Reading HTTP header from UP
		};

		//Create session from a connected socket to DOWN
		Session(boost::asio::ip::tcp::socket & socket);

		//Start session
		void start();

		//End session closing socket connections
		void finish(const std::string & reason = "");

		//Check connection status
		bool is_closed() const { return _con_closed; }

	private:

		//Socket callbacks
		void callback_down_read(const boost::system::error_code & error, size_t len);
		void callback_down_write(const boost::system::error_code & error, size_t len);

		void callback_up_read(const boost::system::error_code & error, size_t len);
		void callback_up_write(const boost::system::error_code & error, size_t len);

		void callback_resolve(const boost::system::error_code & error, boost::asio::ip::tcp::resolver::iterator iterator);

		void callback_connect(const boost::system::error_code & error);

		//Socket callbacks for HTTP tunneling session
		void callback_tunnel_down_read(const boost::system::error_code & error, size_t len);
		void callback_tunnel_down_write(const std::vector<char> write_buffer ,std::size_t write_size, const boost::system::error_code & error, size_t len);

		void callback_tunnel_up_read(const boost::system::error_code & error, size_t len);
		void callback_tunnel_up_write(const std::vector<char> write_buffer ,std::size_t write_size, const boost::system::error_code & error, size_t len);

		//Session status
		int _status;

		//Sockets
		boost::asio::ip::tcp::socket _down_socket; // browser <-> proxy
		boost::asio::ip::tcp::socket _up_socket; // proxy <-> remote

		//Socket I/O buffers
		boost::array<char, THIPROXY_SESSION_BUFFER_LEN> _down_buffer;
		boost::array<char, THIPROXY_SESSION_BUFFER_LEN> _up_buffer;

		//Send/Recv buffers
		std::string _buffer_headers;
		std::string _buffer_request;

		//HTTP body sending
		std::size_t _body_len;
		std::size_t _body_io;

		//HTTP Headers
		HttpHeader _http_header;
		HttpHeader _http_header_req;

		//Connection info
		bool _con_closed;
		bool _con_persistent;
		bool _con_tunnel; //http tunnel (https)

		//Host resolver
		boost::asio::ip::tcp::resolver _address_resolver;
	};
}

#endif