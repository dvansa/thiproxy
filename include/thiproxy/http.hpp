#ifndef THIPROXY_HTTP
#define THIPROXY_HTTP

//std
#include <string>
#include <iostream>
#include <map>
#include <exception>

namespace thiproxy
{
	class HttpHeader
	{
	public:

		class ExceptionParseError : public std::exception
		{
		public:
			ExceptionParseError() : std::exception("HttpHeader : Parse Error"){}
		};

		HttpHeader() : _uri_port(80)
		{

		}
		
		void from_buffer(const std::string & buffer)
		{
			//First line
			auto p = str_find_or_throw(buffer, "\r\n");
			std::string first_line = buffer.substr(0,p);
			std::string headers = buffer.substr(p+1);

			p = str_find_or_throw(first_line, "HTTP");
			if(p == 0) // Status HTTP
			{

			} //Request HTTP 
			else
			{
				p = str_find_or_throw(first_line, " ");
				// GET, POST, UPDATE, CONNECT ...
				_action = first_line.substr(0,p);

				first_line = first_line.substr(p+1);
				p = str_find_or_throw(first_line, " ");

				//host URI
				_uri = first_line.substr(0,p);

				//Http version
				_version = first_line.substr(p + 6); // "HTTP/"

				std::string tmpuri = _uri;

				auto p0 = tmpuri.find("://");
				if(p0 != std::string::npos)
				{
					tmpuri[p0 + 0] = tmpuri[p0 + 1] = tmpuri[p0 + 2] = 'X';
					p0 += 3;
				} 
				else
					p0 = 0;

				auto p1 = tmpuri.find("/");

				auto p2 = tmpuri.find(":");

				if(p2 != std::string::npos)
				{
					_uri_port = atoi(tmpuri.substr(p2 + 1).c_str());

					if(_uri_port == 0)
						_uri_port = 80;
				}

				_uri = _uri.substr( p0 , p1 - p0);
			}

			

			//Print
			std::cout << "HTTP:" << std::endl;
			std::cout << "\taction=" << _action << std::endl;
			std::cout << "\turi=" << _uri << std::endl;
			std::cout << "\tport=" << _uri_port << std::endl;
			std::cout << "\tversion=" << _version << std::endl;

			//Read headers
			while( (p = headers.find("\r\n")) != std::string::npos)
			{
				std::string line = headers.substr(0, p);
				headers = headers.substr(p+2);

				if(line.empty())
					break;

				p = line.find(": ");
				if(p != std::string::npos)
				{
					_headers[line.substr(0,p)] = line.substr(p+2);
				}
			}
			
		}

		inline std::string uri() const { return _uri; }
		inline int uri_port() const { return _uri_port; }
		inline std::string version() const { return _version; }
		inline std::string action() const { return _action; }

		std::string header(const std::string & key) const
		{
			if(_headers.count(key))
				return _headers.at(key);
			else
				return "";
		}


	private:

		//Helper
		std::string::size_type str_find_or_throw(const std::string & str, const std::string & pattern)
		{
			std::string::size_type p = str.find(pattern);
			if(p == std::string::npos)
				throw ExceptionParseError();
			return p;
		}

		std::string _request;

		std::string _action;
		std::string _uri;
		int _uri_port;
		std::string _version;

		std::map<std::string, std::string> _headers;
	};

	class HttpMessage
	{
	public:

		HttpMessage(int type) : _type(type) {}

		enum
		{
			HTTP_REQUEST,
			HTTP_RESPONSE,
			HTTP_ENTITY
		};

	private:
		int _type;

	};
}

#endif