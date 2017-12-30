#ifndef THIPROXY_SERVER
#define THIPROXY_SERVER

//std
#include <string>
#include <iostream>

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
	class Server
	{
	public:
		Server(int port = 80);

		void start();
		void stop();

	private:

		void callback_accept(const boost::system::error_code & error);

		bool _running;
		int _port;

		boost::asio::io_service _io_service;
		boost::asio::ip::tcp::endpoint _endpoint;
		boost::shared_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
		
		boost::asio::ip::tcp::socket _socket;
		std::vector<boost::shared_ptr<thiproxy::Session>> _sessions;
	};
}

#endif