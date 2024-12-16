#include <pygen.hpp>

int main(int argc, char const *argv[]) {
    pygen::PyFile f("./test.py");
    f.import("typing", {"List", "Literal", "Any"});
    f.addFuncs("main", {}, {}, "hello world!", [](pygen::body_ctx& ctx){
        ctx.statment("print(\"hello world\")");
    });
}
