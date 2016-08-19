cmake_minimum_required(VERSION 3.5)
project(rpc)

set(CMAKE_BUILD_TYPE Release)

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
        src/RPCCall.cpp src/smn_rpccall.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -D_LINUX -DSOURCEMOD_BUILD -fno-strict-aliasing -m32")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -funroll-loops -s")

set(CMAKE_CXX_COMPILER gcc)
set(CMAKE_SKIP_BUILD_RPATH true)
set(CMAKE_VERBOSE_MAKEFILE false)

include_directories(include sourcemod/public sourcepawn/include amtl $ENV{BOOST_DIR}/boost)
link_directories($ENV{BOOST_DIR}/stage/lib)

add_library(rpc.ext SHARED ${SOURCE_FILES})

target_link_libraries(rpc.ext -lpthread -Wl,--no-undefined -Wl,-Bstatic -static-libgcc -lboost_thread -lboost_system -lstdc++ -Wl,-Bdynamic -lm)
set_target_properties(rpc.ext PROPERTIES PREFIX "")