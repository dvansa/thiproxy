#include "thiproxy/server.hpp"

namespace thiproxy
{

	Server::Server(int port) : _port(port) , _socket(_io_service)
	{
	}

	void Server::start()
	{
		_endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), _port);
		_acceptor = boost::make_shared<boost::asio::ip::tcp::acceptor>(boost::asio::ip::tcp::acceptor(_io_service,_endpoint));
		

		_acceptor->async_accept(_socket, boost::bind(&Server::callback_accept, this, boost::asio::placeholders::error));
		
		_running = true;
		while(_running)
		{
			_io_service.run_one();
		}
	}

	void Server::stop()
	{
		_running = false;
	}

	void Server::callback_accept(const boost::system::error_code & error)
	{
		std::cout << "Accepted" << std::endl;
		
		_sessions.push_back(boost::make_shared<thiproxy::Session>(_socket));
		_sessions.back()->start();

		//Accept next
		_acceptor->async_accept(_socket, boost::bind(&Server::callback_accept, this, boost::asio::placeholders::error));
	}
}

