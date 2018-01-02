/*
* http.hpp implementations.
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
#include "thiproxy/http.hpp"

namespace thiproxy
{
	HttpHeader::HttpHeader() : _uri_port(80)
	{}

	void HttpHeader::from_buffer(const std::string & buffer)
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

			_uri = _uri.substr( p0 , std::min(p1,p2) - p0);
		}

		//Print
		//std::cout << "HTTP:" << std::endl;
		//std::cout << "\taction=" << _action << std::endl;
		//std::cout << "\turi=" << _uri << std::endl;
		//std::cout << "\tport=" << _uri_port << std::endl;
		//std::cout << "\tversion=" << _version << std::endl;

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

	std::string::size_type HttpHeader::str_find_or_throw(const std::string & str, const std::string & pattern)
	{
		std::string::size_type p = str.find(pattern);
		if(p == std::string::npos)
			throw ExceptionParseError();
		return p;
	}
}

