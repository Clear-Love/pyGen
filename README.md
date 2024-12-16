# python 代码生成工具

## 如何使用

- 作为子目录添加到你的项目
```cmake
add_subdirectory(pygen)
target_link_libraries(your_target pygen)
```
- 在代码中包含头文件

```
#include <pygen.hpp>
```

## 示例

```cpp
#include <pygen.hpp>

int main(int argc, char const *argv[]) {
    pygen::PyFile f("./test.py");
    f.import("typing", {"List", "Literal", "Any"});
    f.addFuncs("main", {}, {}, "hello world!", [](pygen::body_ctx& ctx){
        ctx.statment("print(\"hello world\")");
    });
}

```