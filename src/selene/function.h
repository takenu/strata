/* NOTE: This file originates, except for small modifications, from Selene
 * (found at https://github.com/jeremyong/Selene), as of 10 May 2015.
 * It is not written by Chathran developers and copyright belongs
 * to the original author, according to the zlib license under the terms
 * of which the below code is included in the Chathran project. */

#pragma once

#include <functional>
#include "LuaRef.h"
#include <memory>
#include "primitives.h"
#include "util.h"

namespace sel {
/*
 * Similar to an std::function but refers to a lua function
 */
template <class>
class function {};

template <typename R, typename... Args>
class function<R(Args...)> {
private:
    LuaRef _ref;
    lua_State *_state;
public:
    function(int ref, lua_State *state) : _ref(state, ref), _state(state) {}

    R operator()(Args... args) {
        int handler_index = SetErrorHandler(_state);
        _ref.Push(_state);
        detail::_push_n(_state, args...);
        constexpr int num_args = sizeof...(Args);
        lua_pcall(_state, num_args, 1, handler_index);
        lua_remove(_state, handler_index);
        R ret = detail::_pop(detail::_id<R>{}, _state);
        lua_settop(_state, 0);
        return ret;
    }

    void Push(lua_State *state) {
        _ref.Push(state);
    }
};

template <typename... Args>
class function<void(Args...)> {
private:
    LuaRef _ref;
    lua_State *_state;
public:
    function(int ref, lua_State *state) : _ref(state, ref), _state(state) {}

    void operator()(Args... args) {
        int handler_index = SetErrorHandler(_state);
        _ref.Push(_state);
        detail::_push_n(_state, args...);
        constexpr int num_args = sizeof...(Args);
        lua_pcall(_state, num_args, 1, handler_index);
        lua_remove(_state, handler_index);
        lua_settop(_state, 0);
    }

    void Push(lua_State *state) {
        _ref.Push(state);
    }
};

// Specialization for multireturn types
template <typename... R, typename... Args>
class function<std::tuple<R...>(Args...)> {
private:
    LuaRef _ref;
    lua_State *_state;
public:
    function(int ref, lua_State *state) : _ref(state, ref), _state(state) {}

    std::tuple<R...> operator()(Args... args) {
        int handler_index = SetErrorHandler(_state);
        _ref.Push(_state);
        detail::_push_n(_state, args...);
        constexpr int num_args = sizeof...(Args);
        constexpr int num_ret = sizeof...(R);
        lua_pcall(_state, num_args, num_ret, handler_index);
        lua_remove(_state, handler_index);
        return detail::_pop_n_reset<R...>(_state);
    }

    void Push(lua_State *state) {
        _ref.Push(state);
    }
};
}
