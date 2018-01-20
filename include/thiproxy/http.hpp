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
#include <vector>

namespace thiproxy
{
	//HTTP Buffer type
	typedef std::vector<char> HttpBuffer;

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
		
		//Parse HTTP Header from string buffer (throws on fail)
		void from_buffer(const std::string & buffer);

		//Parse HTTP Header from HTTP buffer (throws on fail)
		void from_buffer(const HttpBuffer & buffer);

		//Returns HTTP header as plain buffer
		HttpBuffer to_buffer() const;

		//Header buffer length
		std::size_t size();

		//URL
		inline std::string url() const { return _url; }

		//URL port
		inline int url_port() const { return _url_port; }

		//Protocol version
		inline std::string version() const { return _version; }

		//Request action (GET,POST,CONNECT...)
		inline std::string action() const { return _action; }

		inline std::string request() const { return _request; }

		//Get value from key-value headers (ie. Content-Length: <bytes>)
		std::string header(const std::string & key) const
		{
			return (_headers.count(key) ? _headers.at(key) : "");
		}


	private:

		//Parsing Helper
		std::string::size_type str_find_or_throw(const std::string & str, const std::string & pattern);

		//Request line
		std::string _request;

		// GET, POST, CONNECT...
		std::string _action;

		//URL
		std::string _url;

		//Connetion port
		int _url_port;

		//Protocol version
		std::string _version;

		//HTTP Headers (key : value)
		std::map<std::string, std::string> _headers;

		//Buffer
		HttpBuffer _buffer;
	};

	//HTTP Message
	class HttpMessage
	{
	public:

		HttpMessage() {}

		//Create HTTP Message from type, header and optional content buffer
		HttpMessage(int type, const HttpHeader & h, const HttpBuffer & c = HttpBuffer() ) : _type(type), header(h), content(c) {}

		enum
		{
			HTTP_REQUEST,
			HTTP_RESPONSE,
			HTTP_ENTITY
		};

		//Returns HTTP message as plain buffer
		HttpBuffer to_buffer() const;

		//Header
		HttpHeader header;

		//Contents
		HttpBuffer content;

		inline std::size_t size() { return header.size()+content.size(); }
		inline std::size_t size_header() { return header.size(); } 
		inline std::size_t size_content() { return content.size(); } 

	private:
		int _type;

	};
}

#endif