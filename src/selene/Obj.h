/* NOTE: This file originates, except for small modifications, from Selene
 * (found at https://github.com/jeremyong/Selene), as of 10 May 2015.
 * It is not written by Chathran developers and copyright belongs
 * to the original author, according to the zlib license under the terms
 * of which the below code is included in the Chathran project. */

#pragma once

#include "ObjFun.h"
#include <functional>
#include <memory>
#include "State.h"
#include <string>
#include <utility>
#include <vector>

namespace sel {
struct BaseObj {
    virtual ~BaseObj() {}
};
template <typename T, typename... Members>
class Obj : public BaseObj {
private:
    std::vector<std::unique_ptr<BaseFun>> _funs;

    template <typename M>
    void _register_member(lua_State *state,
                          T *t,
                          const char *member_name,
                          M T::*member) {
        _register_member(state, t, member_name, member,
                         typename std::is_const<M>::type{});
    }

    template <typename M>
    void _register_member(lua_State *state,
                          T *t,
                          const char *member_name,
                          M T::*member,
                          std::false_type) {
        std::function<M()> lambda_get = [t, member]() {
            return t->*member;
        };
        _funs.emplace_back(
            new ObjFun<1, M>{state, std::string{member_name}, lambda_get});

        std::function<void(M)> lambda_set = [t, member](M value) {
            t->*member = value;
        };
        _funs.emplace_back(
            new ObjFun<0, void, M>
            {state, std::string{"set_"} + member_name, lambda_set});
    }

    template <typename M>
    void _register_member(lua_State *state,
                          T *t,
                          const char *member_name,
                          M T::*member,
                          std::true_type) {
        std::function<M()> lambda_get = [t, member]() {
            return t->*member;
        };
        _funs.emplace_back(
            new ObjFun<1, M>{state, std::string{member_name}, lambda_get});
    }

    template <typename Ret, typename... Args>
    void _register_member(lua_State *state,
                          T *t,
                          const char *fun_name,
                          Ret(T::*fun)(Args&&...)) {
        std::function<Ret(Args&&...)> lambda = [t, fun](Args&&... args) {
            return (t->*fun)(std::forward<Args>(args)...);
        };
        constexpr int arity = detail::_arity<Ret>::value;
        _funs.emplace_back(
            new ObjFun<arity, Ret, Args...>
            {state, std::string(fun_name), lambda});
    }

    template <typename Ret, typename... Args>
    void _register_member(lua_State *state,
                          T *t,
                          const char *fun_name,
                          Ret(T::*fun)(Args...)) {
        std::function<Ret(Args...)> lambda = [t, fun](Args... args) {
            return (t->*fun)(args...);
        };
        constexpr int arity = detail::_arity<Ret>::value;
        _funs.emplace_back(
            new ObjFun<arity, Ret, Args...>
            {state, std::string(fun_name), lambda});
    }

    void _register_members(lua_State *, T *) {}

    template <typename M, typename... Ms>
    void _register_members(lua_State *state, T *t,
                           const char *name,
                           M member,
                           Ms... members) {
        _register_member(state, t, name, member);
        _register_members(state, t, members...);
    }
public:
    Obj(lua_State *state, T *t, Members... members) {
        lua_createtable(state, 0, sizeof...(Members));
        _register_members(state, t, members...);
    }
};
}
