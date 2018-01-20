# ThiProxy
Thin HTTP proxy server written in C++


Features:
  - Bridges HTTP requests
  - Establishes TCP connections for HTTP "CONNECT" (ie. https)
  - User defined behaviour with SessionController/SessionAction

## Dependencies
  - [Boost.Asio](https://http://www.boost.org/doc/libs/master/doc/html/boost_asio.html)

## Compile & Run
(note: cmake configured for VS2013 and boost 1.58)
```
mkdir build && cd build
cmake -G "Visual Studio 12 2013 Win64" ..

```