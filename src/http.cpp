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
	HttpHeader::HttpHeader() : _url_port(80)
	{}

	void HttpHeader::from_buffer(const std::string & buffer)
	{
		_buffer.clear();
		_buffer.insert(_buffer.end(), buffer.begin(), buffer.end());

		//First line
		auto p = str_find_or_throw(buffer, "\r\n");
		std::string first_line = buffer.substr(0,p);

		_request = first_line;

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

			//host URL
			_url = first_line.substr(0,p);

			//Http version
			_version = first_line.substr(p + 6); // "HTTP/"

			std::string tmpurl = _url;

			auto p0 = tmpurl.find("://");
			if(p0 != std::string::npos)
			{
				tmpurl[p0 + 0] = tmpurl[p0 + 1] = tmpurl[p0 + 2] = 'X';
				p0 += 3;
			} 
			else
				p0 = 0;

			auto p1 = tmpurl.find("/");

			auto p2 = tmpurl.find(":");

			if(p2 != std::string::npos)
			{
				_url_port = atoi(tmpurl.substr(p2 + 1).c_str());

				if(_url_port == 0)
					_url_port = 80;
			}

			_url = _url.substr( p0 , std::min(p1,p2) - p0);
		}

		//Print
		//std::cout << "HTTP:" << std::endl;
		//std::cout << "\taction=" << _action << std::endl;
		//std::cout << "\turl=" << _url << std::endl;
		//std::cout << "\tport=" << _url_port << std::endl;
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

	void HttpHeader::from_buffer(const HttpBuffer & buffer)
	{
		_buffer = buffer;
		HttpHeader::from_buffer(std::string(buffer.begin(), buffer.end()));
	}

	std::string::size_type HttpHeader::str_find_or_throw(const std::string & str, const std::string & pattern)
	{
		std::string::size_type p = str.find(pattern);
		if(p == std::string::npos)
			throw ExceptionParseError();
		return p;
	}

	HttpBuffer HttpHeader::to_buffer() const
	{
		std::vector<char> buffer;

		buffer.insert(buffer.begin(), _request.begin(), _request.end());
		buffer.push_back('\r');
		buffer.push_back('\n');

		for(auto& h : _headers)
		{
			buffer.insert(buffer.end(), h.first.begin(), h.first.end() );
			buffer.push_back(':');
			buffer.push_back(' ');
			buffer.insert(buffer.end(), h.second.begin(), h.second.end() );
			buffer.push_back('\r');
			buffer.push_back('\n');
		}

		buffer.push_back('\r');
		buffer.push_back('\n');

		return buffer;
	}

	std::size_t HttpHeader::size()
	{
		//If buffer is not generated create it
		if(_buffer.size() == 0)
			_buffer = to_buffer();

		return _buffer.size();
	}

	HttpBuffer HttpMessage::to_buffer() const
	{
		std::vector<char> buffer, buffer_header;

		buffer_header = header.to_buffer();

		buffer.insert(buffer.end(), buffer_header.begin(), buffer_header.end());
		buffer.insert(buffer.end(), content.begin(), content.end());

		return buffer;
	}
}

