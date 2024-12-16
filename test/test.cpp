#include <pygen.hpp>

int main(int argc, char const *argv[]) {
    pygen::PyFile f("./test.py");
    f.import("typing", {"List", "Literal", "Any"});
    f.addFuncs("main", {}, {}, "hello world!", [](pygen::body_ctx& ctx){
        ctx.statment("print(\"hello world\")");
        ctx.forCtx("i", 1, 10, [](pygen::body_ctx& ctx) {
            ctx.statment("i += 1");
        });
    });
    f.addFuncs("add", {{"a", "int"}, {"b", "int"}}, {{"s", "int"}}, "两数之和", 
        [](pygen::body_ctx& ctx){
        ctx.statment("s = a+b");
    });
    f.addClass("Dev", "", [](pygen::cls_ctx& ctx) {
        ctx.addMemVal({"name", "str"});
        ctx.addMemFunc("getIdn", {}, {{"newName", "str"}}, "", [](pygen::body_ctx& func) {
            func.statment("self.name = newName");
        });
    });
}
