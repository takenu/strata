/* NOTE: This file originates, except for small modifications, from Selene
 * (found at https://github.com/jeremyong/Selene), as of 10 May 2015.
 * It is not written by Chathran developers and copyright belongs
 * to the original author, according to the zlib license under the terms
 * of which the below code is included in the Chathran project. */

#pragma once

#include "BaseFun.h"

namespace sel {

template <typename T>
class Dtor : public BaseFun {
private:
    std::string _metatable_name;
public:
    Dtor(lua_State *l,
         const std::string &metatable_name)
        : _metatable_name(metatable_name) {
        lua_pushlightuserdata(l, (void *)(this));
        lua_pushcclosure(l, &detail::_lua_dispatcher, 1);
        lua_setfield(l, -2, "__gc");
    }

    int Apply(lua_State *l) {
        T *t = (T *)luaL_checkudata(l, 1, _metatable_name.c_str());
        t->~T();
        return 0;
    }
};
}
