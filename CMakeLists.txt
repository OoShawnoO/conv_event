project(conv_event)

set(CMAKE_CXX_STANDARD 20)

add_executable(conv_event main.cpp conn.h conv.h threadpool.h ErrorLog/ErrorLog.h json/json.h)