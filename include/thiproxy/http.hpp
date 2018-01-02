/*
* HTTP protocol helpers.
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
#ifndef THIPROXY_HTTP
#define THIPROXY_HTTP

//std
#include <string>
#include <iostream>
#include <map>
#include <exception>
#include <algorithm>

namespace thiproxy
{
	//HTTP Header
	class HttpHeader
	{
	public:

		//Thrown when an error occurs while parsing header from string
		class ExceptionParseError : public std::exception
		{
		public:
			ExceptionParseError() : std::exception("HttpHeader : Parse Error"){}
		};

		HttpHeader();
		
		//Parse HTTP Header from string buffer (throws)
		void from_buffer(const std::string & buffer);

		//URI
		inline std::string uri() const { return _uri; }

		//URI port
		inline int uri_port() const { return _uri_port; }

		//Protocol version
		inline std::string version() const { return _version; }

		//Request action (GET,POST,CONNECT...)
		inline std::string action() const { return _action; }

		//Get value from key-value headers (ie. Content-Length: <bytes>)
		std::string header(const std::string & key) const
		{
			return (_headers.count(key) ? _headers.at(key) : "");
		}


	private:

		//Helper
		std::string::size_type str_find_or_throw(const std::string & str, const std::string & pattern);

		std::string _request;

		std::string _action;
		std::string _uri;
		int _uri_port;
		std::string _version;

		std::map<std::string, std::string> _headers;
	};

	//HTTP Message
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