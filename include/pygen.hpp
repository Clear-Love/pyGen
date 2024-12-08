#pragma once
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace pygen {

class PyVal {
public:
    PyVal(const std::string &name, const std::string &type)
        : name(name), type(type) {
    }
    std::string name;
    std::string type;
};

class else_ctx;
/**
 * @brief python上下文基类
 *
 */
class Context {
public:
    virtual ~Context() = 0;

protected:
    Context(uint32_t indent = 0)
        : indent(indent) {}; // 构造函数私有，只能有pyFile构造出来
    virtual std::string generate() = 0; // 输出到字符串
    uint32_t indent;                    // 缩进数
    std::string code;
    void writeLine(std::string line) {
        std::ostringstream oss(code);
        for (int i = 0; i < indent; i++) {
            oss << "    "; // 一个缩进四个空格
        }
        oss << line << '\n';
    };
    // 删除移动构造
    Context(Context &&) = delete;
};

/**
 * @brief 函数体上下文
 *
 */
class body_ctx : public Context {
public:
    body_ctx(int indent) : Context(indent) {};
    std::string generate() override {
        return code;
    };

    /**
     * @brief 函数体语句，变量初始化或者赋值
     *
     * @param val
     * @param expr 表达式
     */
    void statment(const PyVal &val, const std::string &expr) {};

    /**
     * @brief 任意表达式或函数调用
     *
     * @param expr 表达式
     */
    void statment(const std::string &expr) {};
    // 条件语句
    else_ctx &ifCtx(const std::string &condition,
                    std::function<void(body_ctx &)> body) {};
    // 循环语句
    void whileCtx(const std::string &condition,
                  std::function<void(body_ctx &)> body) {};

    /**
     * @brief for循环语句
     *
     * @param idx 循环变量
     * @param start 起始值，可以取到
     * @param end 结束值，取不到
     * @param body 循环体回调函数
     */
    void forCtx(const std::string &idx, int start, int end,
                std::function<void(body_ctx &)> body) {};

    /**
     * @brief 迭代对象 eg：for i, v in enumerate(iterable):
     *
     * @param idx 循环下标变量
     * @param it 循环变量
     * @param iter 可循环对象
     * @param body 循环体
     */
    void forenumCtx(const std::string &idx, const std::string &it,
                    const std::string &iter,
                    std::function<void(body_ctx &)> body) {};
};

class else_ctx : public body_ctx {
public:
    else_ctx(int indent) : body_ctx(indent) {
    }
    std::string generate() override {
        return code;
    }

    /**
     * @brief else 语句 链式调用的结尾，没有返回值
     *
     * @param body else 代码块回调
     */
    void elseCtx(std::function<void(body_ctx &)> body) {};

    /**
     * @brief elif 语句 链式调用，只能在ifCtx之后调用
     *
     * @param condition 判断语句
     * @param body elif代码块回调
     * @return else_ctx&
     */
    else_ctx &elifCtx(const std::string &condition,
                      std::function<void(body_ctx &)> body) {};
};

class func_ctx : public Context {
public:
    func_ctx(const std::string &name, const std::vector<PyVal> &params,
             const PyVal &retval, const std::string &comment,
             std::function<void(body_ctx, std::vector<PyVal>, PyVal)> body)
        : name(name), params(params), retval(retval), comment(comment) {};

private:
    std::string generate() override {
        //writeLine(fmt::format("def {}({}):", name, fmt::join(params, ", ")));
        indent++;
        writeLine(fmt::format("\"\"\" {} \"\"\"", comment));
        return code;
    };

    std::string name;
    std::vector<PyVal> params;
    PyVal retval;
    std::string comment;
    std::function<void(func_ctx, std::vector<PyVal>, PyVal)> body;
};

class cls_ctx : public Context {
public:
    cls_ctx(const std::string &name, const std::string &comment)
        : name(name), comment(comment) {};
    std::string generate() override {
        writeLine(fmt::format("class {}:", name));
        indent++;
        writeLine(fmt::format("\"\"\" {} \"\"\"", comment));
        return code;
    };

    void
    addMemFunc(const std::string &name, const std::vector<PyVal> &params,
               const PyVal &retval, const std::string &comment,
               std::function<void(body_ctx, std::vector<PyVal>, PyVal)> body) {
    };
    void
    initFunc(const std::vector<PyVal> &params, const std::string &comment,
             std::function<void(body_ctx, std::vector<PyVal>, PyVal)> body) {};

private:
    std::string name;
    std::string comment;
    std::vector<Context> members;
};
}; // namespace pygen