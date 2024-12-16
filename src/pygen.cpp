#include <fmt/format.h>
#include <pygen.hpp>

namespace pygen {

using namespace std;

string parseParams(vector<PyVal> params) {
    vector<string> res;
    for (auto &param : params) {
        if (param.name == "self") {
            res.push_back("self");
            continue;
        }
        res.push_back(fmt::format("{}:{}", param.name, param.type));
    }
    return fmt::format("{}", fmt::join(res, ", "));
}

string getRetsType(vector<PyVal> params) {
    auto n = params.size();
    if (n == 0) {
        return "None";
    } else if (n == 1) {
        return params[0].type;
    } else {
        vector<string> arr;
        for (auto &p : params) { arr.push_back(p.type); }
        return fmt::format("Tuple[{}]", fmt::join(arr, ","));
    }
}

string getRets(vector<PyVal> params) {
    auto n = params.size();
    if (n == 0) {
        return "";
    } else if (n == 1) {
        return params[0].name;
    } else {
        vector<string> arr;
        for (auto &p : params) { arr.push_back(p.name); }
        return fmt::format("({})", fmt::join(arr, ","));
    }
}

void PyFile::write() {
    file << header.generate() << '\n';
    file << vals.generate() << '\n';
    for (auto &ctx : clss) { file << ctx.generate() << '\n'; }
    for (auto &ctx : funcs) { file << ctx.generate() << '\n'; }
}

void PyFile::addClass(
    const std::string& name, const std::string& comment, function<void(cls_ctx &)> body) {
    clss.push_back(cls_ctx(name, comment, 0));
    body(clss.back());
}

void PyFile::addFuncs(
    const std::string& name,
    vector<PyVal> params,
    vector<PyVal> retvals,
    const std::string& comment,
    function<void(body_ctx &)> body) {
    funcs.push_back(func_ctx(name, params, retvals, comment, 0, body));
}

void PyFile::import(
    const std::string& lib, std::initializer_list<std::string> mods) {
    // auto n = mods.size();
    vector<string> vars;
    for (auto &mod : mods) { vars.push_back(mod); }
    header.writeLine(
        "from {} import {}", lib, fmt::format("{}", fmt::join(vars, ", ")));
}

void PyFile::import(const std::string& lib) {
    header.writeLine("import {}", lib);
}

void PyFile::expr(const std::string& expr) {
    header.writeLine(expr);
}

void cls_ctx::addMemFunc(
    const std::string& name,
    vector<PyVal> params,
    vector<PyVal> retvals,
    const std::string& comment,
    function<void(body_ctx &)> body) {
    params.emplace(params.begin(), "self", "");
    func_ctx ctx(name, params, retvals, comment, indent + 1, body);
    code->append(ctx.generate());
    code->append("\n");
}

void cls_ctx::addMemVal(PyVal val) {
    MemVals.push_back(val);
}

void cls_ctx::addMemVal(std::initializer_list<PyVal> vals) {
    for (auto &val : vals) { MemVals.push_back(val); }
}

std::string cls_ctx::generate() {
    std::string ret;
    ret.append(fmt::format("class {}:\n", name));
    if (comment != "") {
        ret.append(fmt::format("    \"\"\" {} \"\"\"\n", comment));
    }
    func_ctx ctx(
        "__init__", MemVals, {{"", "None"}}, "", indent + 1,
        [&](body_ctx &body) {
            for (size_t i = 1; i < MemVals.size(); i++) {
                auto val = MemVals[i];
                body.statment(fmt::format("self.{} = {}", val.name, val.name));
            }
        });
    ret.append(ctx.generate());
    ret.append("\n");
    ret.append(*code);
    return ret;
}

func_ctx::func_ctx(
    const std::string& name,
    const vector<PyVal> &params,
    const vector<PyVal> &retvals,
    const std::string& comment,
    int indent,
    function<void(body_ctx &)> body)
    : Context(indent) {
    writeLine(
        "def {}({}) -> {}:", name, parseParams(params), getRetsType(retvals));
    body_ctx ctx(indent + 1);
    if (comment != "") { ctx.writeLine("\"\"\"{}\"\"\"", comment); }
    body(ctx);
    ctx.writeLine("return {}", getRets(retvals));

    code->append(ctx.generate());
}

else_ctx &body_ctx::ifCtx(
    const std::string& condition, std::function<void(body_ctx &)> body) {
    writeLine("if {}:", condition);
    body_ctx ctx(indent + 1);
    body(ctx);
    code->append(ctx.generate());
    ctxs.emplace_back(indent, code);
    return ctxs.back();
}

void body_ctx::whileCtx(
    const std::string& condition, std::function<void(body_ctx &)> body) {
    writeLine("while {}:", condition);
    body_ctx ctx(indent + 1);
    body(ctx);
    code->append(ctx.generate());
}

void body_ctx::forCtx(
    const std::string& idx,
    int start,
    int end,
    std::function<void(body_ctx &)> body) {
    writeLine("for {} in range({}, {}):", idx, start, end);
    body_ctx ctx(indent + 1);
    body(ctx);
    code->append(ctx.generate());
}

void body_ctx::forenumCtx(
    const std::string &idx,
    const std::string &it,
    const std::string &iter,
    std::function<void(body_ctx &)> body) {
    writeLine("for {}, {} in enumerate({}):", idx, it, iter);
    body_ctx ctx(indent + 1);
    body(ctx);
    code->append(ctx.generate());
}

else_ctx &else_ctx::elifCtx(
    const std::string &condition, std::function<void(body_ctx &)> body) {
    writeLine("elif {}:", condition);
    body_ctx ctx(indent + 1);
    body(ctx);
    code->append(ctx.generate());
    ctxs.emplace_back(indent, code);
    return ctxs.back();
}

std::vector<std::string> details::Split(const std::string &src, const std::string &sep) {
    std::vector<std::string> res;
    std::string::size_type pos1, pos2;
    pos2 = src.find(sep);
    pos1 = 0;
    while (std::string::npos != pos2) {
        res.push_back(src.substr(pos1, pos2 - pos1));
        pos1 = pos2 + sep.size();
        pos2 = src.find(sep, pos1);
    }
    if (pos1 != src.length()) { res.push_back(src.substr(pos1)); }
    return res;
}
} // namespace pygen
