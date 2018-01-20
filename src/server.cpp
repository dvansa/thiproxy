/*
* server.hpp implementations.
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
		//Clean finished sessions
		clean_sessions();

		boost::shared_ptr<Session> new_session = boost::make_shared<Session>(_socket);
		_sessions.insert(new_session);

		new_session->set_controller(_session_controller);
		new_session->start();

		std::cout << "[Session Started] Current sessions " << _sessions.size() << std::endl;

		//Accept next
		_acceptor->async_accept(_socket, boost::bind(&Server::callback_accept, this, boost::asio::placeholders::error));
	}

	void Server::clean_sessions()
	{
		std::set<boost::shared_ptr<Session>> _to_remove;
		for(auto& s : _sessions)
		{
			if(s->is_closed())
				_to_remove.insert(s);
		}

		if(_to_remove.size())
		{
			std::cout << "Cleaning " << _to_remove.size() << " sessions." << std::endl;
		}

		for(auto& rs : _to_remove)
			_sessions.erase(rs);
	}


}

