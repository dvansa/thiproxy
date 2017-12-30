#include "thiproxy/session.hpp"

namespace thiproxy
{

	Session::Session(boost::asio::ip::tcp::socket & socket) : _down_socket(std::move(socket)),
															  _up_socket(_down_socket.get_io_service()),
															  _address_resolver(_down_socket.get_io_service()),
															  _status(STATUS_READ_HEADER),
															  _con_closed(false),
															  _con_persistent(false),
															  _con_tunnel(false)
	{
		
	}

	void Session::start()
	{
		_con_closed = false;
		_con_persistent = false;
		_con_tunnel = false;
		_buffer_request.clear();
		_buffer_headers.clear();

		_status = STATUS_READ_HEADER;

		async_read(_down_socket, boost::asio::buffer(_down_buffer), boost::asio::transfer_at_least(1),
					    boost::bind(&Session::callback_down_read,
						shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
	}

	void Session::finish(const std::string & reason)
	{
		std::cout << "[Finished]" << reason << " --- Req: " << _http_header_req.uri() <<  std::endl;
		_con_closed = true;
		_down_socket.close();
		_up_socket.close();
	}

	void Session::callback_down_read(const boost::system::error_code & error, size_t len)
	{
		if(error)
		{
			finish("Down Read");
			return;
		}

		if(_status == STATUS_READ_HEADER)
		{
			_buffer_headers += std::string(_down_buffer.data(),len);

			if(_buffer_headers.find("\r\n\r\n") == std::string::npos)
			{
				async_read(_down_socket, boost::asio::buffer(_down_buffer), boost::asio::transfer_at_least(1),
					    boost::bind(&Session::callback_down_read,
						shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
			}
			else
			{
			
				//std::cout << "HTTP: " << std::endl << _buffer_headers << std::endl;
				std::string uri;
				std::string port;
				try
				{
					_http_header_req.from_buffer(_buffer_headers);
					uri = _http_header_req.uri();
					port = std::to_string(_http_header_req.uri_port());

					if(_http_header_req.action() == "CONNECT")
					{
						_con_tunnel = true;
					}


				}
				catch(const HttpHeader::ExceptionParseError &)
				{
					//std::cout << "HttpHeader parse error" << std::endl;
					finish();
				}

				boost::asio::ip::tcp::resolver::query q(uri, port);
				_address_resolver.async_resolve(q,boost::bind(&Session::callback_resolve, shared_from_this(),
													boost::asio::placeholders::error,
													boost::asio::placeholders::iterator));
			}

			
			//std::cout << "Readed bytes" <<len << std::endl;
		}
		
	}

	void Session::callback_resolve(const boost::system::error_code & error, boost::asio::ip::tcp::resolver::iterator iterator)
	{
		if(error)
		{
			finish("Resolve");
			return;
		}

		//std::cout << "[Resolved]" << std::endl;

		_status = STATUS_CONNECTING;
		_up_socket.async_connect(*iterator, boost::bind(&Session::callback_connect, shared_from_this(),boost::asio::placeholders::error) );
	}

	void Session::callback_connect(const boost::system::error_code & error)
	{
		if(error)
		{
			finish("Connect");
			return;
		}

		_status = STATUS_CONNECTED;

		if(_con_tunnel)
		{
			_status = STATUS_CONNECTED;

			std::string res_http_ok = "HTTP/1.1 200 OK\r\n\r\n";
			boost::asio::async_write(_down_socket, boost::asio::buffer(res_http_ok),
			boost::bind(&Session::callback_tunnel_down_write, shared_from_this(),
									std::vector<char>(),
									0,
									boost::asio::placeholders::error,
									boost::asio::placeholders::bytes_transferred));


			//Start reading from down/up
			async_read(_down_socket, boost::asio::buffer(_down_buffer), boost::asio::transfer_at_least(1),
						    boost::bind(&Session::callback_tunnel_down_read,
							shared_from_this(), 
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred));

			async_read(_up_socket, boost::asio::buffer(_up_buffer), boost::asio::transfer_at_least(1),
						    boost::bind(&Session::callback_tunnel_up_read,
							shared_from_this(), 
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred));

			return;
		}

		_buffer_request = _buffer_headers;
		_buffer_headers.clear();

		boost::asio::async_write(_up_socket, boost::asio::buffer(_buffer_request),
		boost::bind(&Session::callback_up_write, shared_from_this(),
								boost::asio::placeholders::error,
								boost::asio::placeholders::bytes_transferred));

		//std::cout << "[Connected]" << std::endl;
		//std::cout << "[Write UP] " << std::endl << _buffer_request << std::endl;
	}

	void Session::callback_up_write(const boost::system::error_code & error, size_t len)
	{
		if(error)
		{
			finish("Up write");
			return;
		}

		_status = STATUS_READ_HEADER_UP;

		boost::asio::async_read(_up_socket, boost::asio::buffer(_up_buffer),
		boost::bind(&Session::callback_up_read, shared_from_this(),
								boost::asio::placeholders::error,
								boost::asio::placeholders::bytes_transferred));

		//std::cout << "[UP Write]" << std::endl;
	}

	void Session::callback_up_read(const boost::system::error_code & error, size_t len)
	{
		

		if(_status == STATUS_READ_HEADER_UP)
		{
			if(error && error != boost::asio::error::eof)
			{
				std::cout << error.message() << std::endl;
				finish("Up Read");
				return;
			}

			_buffer_headers += std::string(_up_buffer.data(),len);

			//std::cout << "[UP Read] " << len << std::string(_up_buffer.data(),len) << std::endl;
			if(_buffer_headers.find("\r\n\r\n") == std::string::npos)
			{
				async_read(_up_socket, boost::asio::buffer(_up_buffer), boost::asio::transfer_at_least(1),
					    boost::bind(&Session::callback_up_read,
						shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
			}
			else
			{
				//std::cout << "[UP HTTP]: " << std::endl << _buffer_headers << std::endl;

				try
				{
					//HTTP Up response header
					_http_header.from_buffer(_buffer_headers);
					
					//std::cout << "CONNECTION=" << _http_header.header("Connection") << std::endl;
					//std::cout << "CONTENT-LENGTH=" << _http_header.header("Content-Length") << std::endl;

				}
				catch(const HttpHeader::ExceptionParseError &)
				{
					//std::cout << "HttpHeader parse error" << std::endl;
					finish("Parse Error Up");
				}

				std::string body_len = _http_header.header("Content-Length");
				_body_len = (body_len != "") ? atoi(body_len.c_str()) : 0;
				_body_io = 0;

				std::string version = _http_header.version();
				std::string req_version = _http_header_req.version();
				std::string connection = _http_header.header("Connection");

				_con_persistent = ((req_version == "1.1" && connection != "close") || (req_version == "1.0" && connection == "keep-alive")) &&
							  ((version == "1.1" && connection != "close") ||(version == "1.0" && connection == "keep-alive")) &&
								(_body_len != 0);


				_status = STATUS_CONNECTED;

				boost::asio::async_write(_down_socket, boost::asio::buffer(_buffer_headers),
					boost::bind(&Session::callback_down_write, shared_from_this(),
											boost::asio::placeholders::error,
											boost::asio::placeholders::bytes_transferred));


			}
		}
		else if( _status == STATUS_CONNECTED)
		{
			if(error && error != boost::asio::error::eof)
			{
				finish("Error Up Read connected");
				return;
			}

			if(error == boost::asio::error::eof)
				_con_closed = true;

			_body_io += len;

			boost::asio::async_write(_down_socket, boost::asio::buffer(_up_buffer,len),
				boost::bind(&Session::callback_down_write, shared_from_this(),
										boost::asio::placeholders::error,
										boost::asio::placeholders::bytes_transferred));
	}


	}

	void Session::callback_down_write(const boost::system::error_code & error, size_t len)
	{
		if(error)
		{
			finish("Error down write");
			return;
		}

		if(!_con_closed)
		{

			if( (_body_io < _body_len) || (_body_len == 0) )
			{
				

				async_read(_up_socket, boost::asio::buffer(_up_buffer,len), boost::asio::transfer_at_least(1),
							    boost::bind(&Session::callback_up_read,
								shared_from_this(),
								boost::asio::placeholders::error,
								boost::asio::placeholders::bytes_transferred) );
			}
			else
			{
				if(_con_persistent)
				{
					start();
				}
				else
				{
					finish("Sucess Sent");
				}
			}
		}
		else
		{
			finish("Con Closed");
		}

	}

	void Session::callback_tunnel_down_read(const boost::system::error_code & error, size_t len)
	{
		if(error)
		{
			//std::cout << "[Tunnel EOF] Down" << std::endl;
			finish();
			
		}
		else
		{

			

			if(error != boost::asio::error::eof && len > 0)
			{
				std::vector<char> copy_data(len);
				boost::asio::buffer_copy( boost::asio::buffer(copy_data,len) ,boost::asio::buffer(_down_buffer,len));

				callback_tunnel_up_write(copy_data, len, boost::system::error_code(), len);
			}

			async_read(_down_socket, boost::asio::buffer(_down_buffer), boost::asio::transfer_at_least(1),
							    boost::bind(&Session::callback_tunnel_down_read,
								shared_from_this(), 
								boost::asio::placeholders::error,
								boost::asio::placeholders::bytes_transferred));


		}
	}

	void Session::callback_tunnel_down_write(const std::vector<char> write_buffer ,std::size_t write_size, const boost::system::error_code & error, size_t len)
	{

		//std::cout << "[Tunnel Write] Down" << write_size << " len " << len << std::endl;
		
		if(write_size > 0)
		{
			boost::asio::async_write(_down_socket, boost::asio::buffer(write_buffer,write_size),
			boost::bind(&Session::callback_tunnel_down_write, shared_from_this(),
									std::vector<char>(),
									0,
									boost::asio::placeholders::error,
									boost::asio::placeholders::bytes_transferred));
		}


	

	}

	void Session::callback_tunnel_up_read(const boost::system::error_code & error, size_t len)
	{
		if(error )
		{
			//std::cout << "[Tunnel ERROR] Up" << std::endl;
			finish();
			
		}
		else
		{
		

			if(error != boost::asio::error::eof && len > 0)
			{
				std::vector<char> copy_data(len);
				boost::asio::buffer_copy( boost::asio::buffer(copy_data,len) ,boost::asio::buffer(_up_buffer,len));

				callback_tunnel_down_write(copy_data, len, boost::system::error_code(), len);
			}
			
			
			async_read(_up_socket, boost::asio::buffer(_up_buffer), boost::asio::transfer_at_least(1),
						    boost::bind(&Session::callback_tunnel_up_read,
							shared_from_this(), 
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred));

		}
	}

	void Session::callback_tunnel_up_write(const std::vector<char> write_buffer ,std::size_t write_size, const boost::system::error_code & error, size_t len)
	{
		//std::cout << "[Tunnel Write] Up" << write_size <<  std::endl;
		
		if(write_size > 0)
		{
			boost::asio::async_write(_up_socket, boost::asio::buffer(write_buffer,write_size),
			boost::bind(&Session::callback_tunnel_up_write, shared_from_this(),
									std::vector<char>(),
									0,
									boost::asio::placeholders::error,
									boost::asio::placeholders::bytes_transferred));
		}
		
	}
}

