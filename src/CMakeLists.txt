file(GLOB_RECURSE srcs *.cpp)
# 添加静态库或动态库
add_library(pygen STATIC ${srcs})
# add_library(my_library SHARED ${srcs})

# 链接 fmt 库
target_link_libraries(pygen PRIVATE fmt::fmt)

# 设置头文件目录
target_include_directories(pygen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})