#pragma once
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

namespace pygen {

namespace details {
    std::vector<std::string> Split(const std::string& src,  const std::string& sep);
}


class PyVal {
public:
    PyVal(const std::string &name="", const std::string &type="None")
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
protected:
    friend class PyFile;
    virtual ~Context() = default;
    Context(uint32_t indent = 0): indent(indent), code(new std::string("")) {}; // 构造函数私有，只能有pyFile构造出来
    Context(uint32_t indent, std::shared_ptr<std::string> code): indent(indent), code(code) {}; 
    virtual std::string generate() { return *code;}; // 输出到字符串
    uint32_t indent;                    // 缩进数
    std::shared_ptr<std::string> code;
    template <typename... T>
    void writeLine(fmt::format_string<T...> fmt, T&&... args) {
        auto s = fmt::format(fmt, args...);
        auto lines = details::Split(s, "\n");
        for (auto& line: lines) {
            for (int i = 0; i < indent; i++) {
                code->append("    "); // 一个缩进四个空格
            }
            code->append(line);
            code->append("\n");
        }
    };
};

/**
 * @brief 函数体上下文
 *
 */
class body_ctx : public Context {
public:
    /**
     * @brief 任意表达式或函数调用
     *
     * @param expr 表达式
     */
    template <typename... T>
    void statment(fmt::format_string<T...> fmt, T&&... args) {
        writeLine(fmt, args...);
    };
    // 条件语句
    else_ctx &ifCtx(const std::string& condition,
                    std::function<void(body_ctx &)> body);
    // 循环语句
    void whileCtx(const std::string& condition,
                  std::function<void(body_ctx &)> body);

    /**
     * @brief for循环语句
     *
     * @param idx 循环变量
     * @param start 起始值，可以取到
     * @param end 结束值，取不到
     * @param body 循环体回调函数
     */
    void forCtx(const std::string& idx, int start, int end,
                std::function<void(body_ctx &)> body);

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
                    std::function<void(body_ctx &)> body);
protected:
    body_ctx(int indent) : Context(indent) {};
    body_ctx(int indent, std::shared_ptr<std::string> code) : Context(indent) {};
    friend class PyFile;
    friend class func_ctx;
    friend class else_ctx;
    std::vector<else_ctx> ctxs;
};

class else_ctx : public body_ctx {
public:
    else_ctx(int indent, std::shared_ptr<std::string> code): body_ctx(indent, code) {};
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
                      std::function<void(body_ctx &)> body);
};

class func_ctx : public Context {
private:
    friend class cls_ctx;
    friend class PyFile;
    func_ctx(const std::string& name, const std::vector<PyVal> &params,
             const std::vector<PyVal> &retvals, const std::string& comment, int indent,
             std::function<void(body_ctx&)> body);
};

class cls_ctx : public Context {
public:
    void
    addMemFunc(const std::string& name, std::vector<PyVal> params,
               std::vector<PyVal> retvals, const std::string& comment,
               std::function<void(body_ctx&)> body);
    void addMemVal(PyVal val);
    void addMemVal(std::initializer_list<PyVal> vals);

protected:
    friend class PyFile;
    cls_ctx(const std::string& name, const std::string& comment,
     int indent): name(name), comment(comment), Context(indent) {};
    cls_ctx(int indent): Context(indent){};
    std::string generate() override;
private:
    std::vector<PyVal> MemVals{{"self", ""}};
    std::string name;
    std::string comment;
};

class PyFile {
private:
    std::ofstream file;
    body_ctx header; // 文件头：import语句
    body_ctx vals; // 全局变量
    std::vector<func_ctx> funcs; // 函数
    std::vector<cls_ctx> clss; // 自定义类

    void write(); // 写入到文件中

public:
    explicit PyFile(const std::string& filename)
        : file(filename, std::ios::out), header(0), vals(0) {
        if (!file.is_open()) {
            std::cerr << fmt::format("Failed to open file: {}", filename) << '\n';
            throw std::runtime_error("Failed to open file: " + filename);
        }
    }

    ~PyFile() {
        if (file.is_open()) {
            write();
            file.close();
        } else {
        }
    }

    void addClass(const std::string& name, const std::string& comment, std::function<void(cls_ctx&)> body);
    void addFuncs(const std::string& name, std::vector<PyVal> params,
             std::vector<PyVal> retvals, const std::string& comment,
             std::function<void(body_ctx&)> body);
    
    void import(const std::string& lib, std::initializer_list<std::string> mods);
    void import(const std::string& lib);
    void expr(const std::string& expr);
};

}; // namespace pygen