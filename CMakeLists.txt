cmake_minimum_required(VERSION 3.5)
project(rpc)

set(SOURCE_FILES
        src/sdk/smsdk_ext.cpp
        src/CommandProcessor.cpp
        src/CoreConfig.cpp
        src/EventLoop.cpp
        src/Exstension.cpp
        src/RPCContext.cpp
        src/RPCMethod.cpp
        src/smn_rpccontext.cpp
        src/SocketHandler.cpp
        src/Server.cpp
        src/smn_json.cpp
        src/Crypto.cpp
        src/RPCCall.cpp 
        src/smn_rpccall.cpp
        src/smn_extension.cpp)

if(NOT TARGET)
        set(TARGET linux)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -DSOURCEMOD_BUILD -fno-strict-aliasing -m32")

if(${TARGET} STREQUAL linux)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LINUX")
else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWIN32 -D_STAT_DEFINED")
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -funroll-loops -s")

set(CMAKE_SKIP_BUILD_RPATH true)
set(CMAKE_VERBOSE_MAKEFILE false)

include_directories(include sourcemod/public sourcepawn/include amtl)

add_library(rpc.ext SHARED ${SOURCE_FILES})

if(${TARGET} STREQUAL linux)
        target_link_libraries(rpc.ext -lpthread -Wl,--no-undefined -Wl,-Bstatic -lcrypto -lz -static-libgcc -lboost_thread -lboost_system -lstdc++ -Wl,-Bdynamic -lm -ldl)
else()
        target_link_libraries(rpc.ext -lpthread -Wl,--no-undefined -Wl,-Bstatic -lcrypto -lz -static-libgcc -lboost_thread-mt -lboost_system -lstdc++ -Wl,-Bdynamic -lm -lwsock32 -lgdi32 -lws2_32)
endif()

set_target_properties(rpc.ext PROPERTIES PREFIX "")

if(NOT ${TARGET} STREQUAL linux)
        set_target_properties(rpc.ext PROPERTIES SUFFIX ".dll")
endif()