cmake_minimum_required(VERSION 2.6)
PROJECT(thiproxy)

#Boost directory
SET(BOOST_DIR "C:/Users/Dani/Development/dependencies/boost_1_58_0")

INCLUDE_DIRECTORIES(include)
INCLUDE_DIRECTORIES(${BOOST_DIR})
LINK_DIRECTORIES("${BOOST_DIR}/lib64-msvc-12.0")

SET(BOOST_LIBS libboost_thread-vc120-mt-1_58
			   libboost_system-vc120-mt-1_58)

SET(SRC_FILES  src/thiproxy.cpp
               src/server.cpp
               src/session.cpp
               src/http.cpp)

ADD_EXECUTABLE(thiproxy ${SRC_FILES})
TARGET_LINK_LIBRARIES(thiproxy ${BOOST_LIBS})

