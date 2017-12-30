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
	class Session : public boost::enable_shared_from_this<Session>
	{
	public:
		enum
		{
			STATUS_READ_HEADER,
			STATUS_CONNECTING,
			STATUS_CONNECTED,
			STATUS_READ_HEADER_UP
		};

		Session(boost::asio::ip::tcp::socket & socket);

		void start();

		void finish(const std::string & reason = "");

		bool is_closed() const { return _con_closed; }

	private:
		void callback_down_read(const boost::system::error_code & error, size_t len);
		void callback_down_write(const boost::system::error_code & error, size_t len);

		void callback_up_read(const boost::system::error_code & error, size_t len);
		void callback_up_write(const boost::system::error_code & error, size_t len);

		void callback_resolve(const boost::system::error_code & error, boost::asio::ip::tcp::resolver::iterator iterator);

		void callback_connect(const boost::system::error_code & error);


		void callback_tunnel_down_read(const boost::system::error_code & error, size_t len);
		void callback_tunnel_down_write(const std::vector<char> write_buffer ,std::size_t write_size, const boost::system::error_code & error, size_t len);

		void callback_tunnel_up_read(const boost::system::error_code & error, size_t len);
		void callback_tunnel_up_write(const std::vector<char> write_buffer ,std::size_t write_size, const boost::system::error_code & error, size_t len);

		int _status;

		//Sockets src/dst
		boost::asio::ip::tcp::socket _down_socket;
		boost::asio::ip::tcp::socket _up_socket;

		//Socket I/O buffers
		boost::array<char, THIPROXY_SESSION_BUFFER_LEN> _down_buffer;
		boost::array<char, THIPROXY_SESSION_BUFFER_LEN> _up_buffer;

		//Send/Recv buffers
		std::string _buffer_headers;
		std::string _buffer_request;


		//HTTP body
		std::size_t _body_len;
		std::size_t _body_io;

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