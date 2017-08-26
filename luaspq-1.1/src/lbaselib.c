/*
** $Id: lbaselib.c,v 1.312 2015/10/29 15:21:04 roberto Exp $
** Basic library
** See Copyright Notice in lua.h
*/

#define lbaselib_c
#define LUA_LIB

#include "lprefix.h"

#include <windows.h>
#include <assert.h>
#include <wincon.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


static int luaB_print (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  int i;
  lua_getglobal(L, "tostring");
  for (i=1; i<=n; i++) {
    const char *s;
    size_t l;
    lua_pushvalue(L, -1);  /* function to be called */
    lua_pushvalue(L, i);   /* value to print */
    lua_call(L, 1, 1);
    s = lua_tolstring(L, -1, &l);  /* get result */
    if (s == NULL)
      return luaL_error(L, "'tostring' must return a string to 'print'");
    if (i>1) lua_writestring("\t", 1);
    lua_writestring(s, l);
    lua_pop(L, 1);  /* pop result */
  }
  lua_writeline();
  return 0;
}


#define SPACECHARS	" \f\n\r\t\v"

static const char *b_str2int (const char *s, int base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  int neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  if (*s == '-') { s++; neg = 1; }  /* handle signal */
  else if (*s == '+') s++;
  if (!isalnum((unsigned char)*s))  /* no digit? */
    return NULL;
  do {
    int digit = (isdigit((unsigned char)*s)) ? *s - '0'
                   : (toupper((unsigned char)*s) - 'A') + 10;
    if (digit >= base) return NULL;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } while (isalnum((unsigned char)*s));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  return s;
}


static int luaB_tonumber (lua_State *L) {
  if (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    luaL_checkany(L, 1);
    if (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      return 1;
    }
    else {
      size_t l;
      const char *s = lua_tolstring(L, 1, &l);
      if (s != NULL && lua_stringtonumber(L, s) == l + 1)
        return 1;  /* successful conversion to number */
      /* else not a number */
    }
  }
  else {
    size_t l;
    const char *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    if (b_str2int(s, (int)base, &n) == s + l) {
      lua_pushinteger(L, n);
      return 1;
    }  /* else not a number */
  }  /* else not a number */
  lua_pushnil(L);  /* not a number */
  return 1;
}


static int luaB_error (lua_State *L) {
  int level = (int)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  if (lua_isstring(L, 1) && level > 0) {  /* add extra information? */
    luaL_where(L, level);
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  return lua_error(L);
}


static int luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  if (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    return 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  return 1;  /* returns either __metatable field (if present) or metatable */
}


static int luaB_setmetatable (lua_State *L) {
  int t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argcheck(L, t == LUA_TNIL || t == LUA_TTABLE, 2,
                    "nil or table expected");
  if (luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL)
    return luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  return 1;
}


static int luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  return 1;
}


static int luaB_rawlen (lua_State *L) {
  int t = lua_type(L, 1);
  luaL_argcheck(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                   "table or string expected");
  lua_pushinteger(L, lua_rawlen(L, 1));
  return 1;
}


static int luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  return 1;
}

static int luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  return 1;
}


static int luaB_collectgarbage (lua_State *L) {
  static const char *const opts[] = {"stop", "restart", "collect",
    "count", "step", "setpause", "setstepmul",
    "isrunning", NULL};
  static const int optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCSETPAUSE, LUA_GCSETSTEPMUL,
    LUA_GCISRUNNING};
  int o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  int ex = (int)luaL_optinteger(L, 2, 0);
  int res = lua_gc(L, o, ex);
  switch (o) {
    case LUA_GCCOUNT: {
      int b = lua_gc(L, LUA_GCCOUNTB, 0);
      lua_pushnumber(L, (lua_Number)res + ((lua_Number)b/1024));
      return 1;
    }
    case LUA_GCSTEP: case LUA_GCISRUNNING: {
      lua_pushboolean(L, res);
      return 1;
    }
    default: {
      lua_pushinteger(L, res);
      return 1;
    }
  }
}


static int luaB_type (lua_State *L) {
  int t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  return 1;
}


static int pairsmeta (lua_State *L, const char *method, int iszero,
                      lua_CFunction iter) {
  if (luaL_getmetafield(L, 1, method) == LUA_TNIL) {  /* no metamethod? */
    luaL_checktype(L, 1, LUA_TTABLE);  /* argument must be a table */
    lua_pushcfunction(L, iter);  /* will return generator, */
    lua_pushvalue(L, 1);  /* state, */
    if (iszero) lua_pushinteger(L, 0);  /* and initial value */
    else lua_pushnil(L);
  }
  else {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_call(L, 1, 3);  /* get 3 values from metamethod */
  }
  return 3;
}


static int luaB_next (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  if (lua_next(L, 1))
    return 2;
  else {
    lua_pushnil(L);
    return 1;
  }
}


static int luaB_pairs (lua_State *L) {
  return pairsmeta(L, "__pairs", 0, luaB_next);
}


/*
** Traversal function for 'ipairs'
*/
static int ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2) + 1;
  lua_pushinteger(L, i);
  return (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** This function will use either 'ipairsaux' or 'ipairsaux_raw' to
** traverse a table, depending on whether the table has metamethods
** that can affect the traversal.
*/
static int luaB_ipairs (lua_State *L) {
#if defined(LUA_COMPAT_IPAIRS)
  return pairsmeta(L, "__ipairs", 1, ipairsaux);
#else
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  return 3;
#endif
}


static int load_aux (lua_State *L, int status, int envidx) {
  if (status == LUA_OK) {
    if (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      if (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_pop(L, 1);  /* remove 'env' if not used by previous call */
    }
    return 1;
  }
  else {  /* error (message is on top of the stack) */
    lua_pushnil(L);
    lua_insert(L, -2);  /* put before error message */
    return 2;  /* return nil plus error message */
  }
}


static int luaB_loadfile (lua_State *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  const char *mode = luaL_optstring(L, 2, NULL);
  int env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  int status = luaL_loadfilex(L, fname, mode);
  return load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** reserved slot, above all arguments, to hold a copy of the returned
** string to avoid it being collected while parsed. 'load' has four
** optional arguments (chunk, source name, mode, and environment).
*/
#define RESERVEDSLOT	5


/*
** Reader for generic 'load' function: 'lua_load' uses the
** stack for internal stuff, so the reader cannot change the
** stack top. Instead, it keeps its resulting string in a
** reserved slot inside the stack.
*/
static const char *generic_reader (lua_State *L, void *ud, size_t *size) {
  (void)(ud);  /* not used */
  luaL_checkstack(L, 2, "too many nested functions");
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);  /* pop result */
    *size = 0;
    return NULL;
  }
  else if (!lua_isstring(L, -1))
    luaL_error(L, "reader function must return a string");
  lua_replace(L, RESERVEDSLOT);  /* save string in reserved slot */
  return lua_tolstring(L, RESERVEDSLOT, size);
}


static int luaB_load (lua_State *L) {
  int status;
  size_t l;
  const char *s = lua_tolstring(L, 1, &l);
  const char *mode = luaL_optstring(L, 3, "bt");
  int env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  if (s != NULL) {  /* loading a string? */
    const char *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  else {  /* loading from a reader function */
    const char *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_settop(L, RESERVEDSLOT);  /* create reserved slot */
    status = lua_load(L, generic_reader, NULL, chunkname, mode);
  }
  return load_aux(L, status, env);
}

/* }====================================================== */


static int dofilecont (lua_State *L, int d1, lua_KContext d2) {
  (void)d1;  (void)d2;  /* only to match 'lua_Kfunction' prototype */
  return lua_gettop(L) - 1;
}


static int luaB_dofile (lua_State *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  lua_settop(L, 1);
  if (luaL_loadfile(L, fname) != LUA_OK)
    return lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  return dofilecont(L, 0, 0);
}


static int luaB_assert (lua_State *L) {
  if (lua_toboolean(L, 1))  /* condition is true? */
    return lua_gettop(L);  /* return all arguments */
  else {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    return luaB_error(L);  /* call 'error' */
  }
}


static int luaB_select (lua_State *L) {
  int n = lua_gettop(L);
  if (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    return 1;
  }
  else {
    lua_Integer i = luaL_checkinteger(L, 1);
    if (i < 0) i = n + i;
    else if (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    return n - (int)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
static int finishpcall (lua_State *L, int status, lua_KContext extra) {
  if (status != LUA_OK && status != LUA_YIELD) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    return 2;  /* return false, msg */
  }
  else
    return lua_gettop(L) - (int)extra;  /* return all results */
}


static int luaB_pcall (lua_State *L) {
  int status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_insert(L, 1);  /* put it in place */
  status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  return finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
static int luaB_xpcall (lua_State *L) {
  int status;
  int n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  return finishpcall(L, status, 2);
}


static int luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, NULL);
  return 1;
}


//static const luaL_Reg base_funcs[] = {
//  {"assert", luaB_assert},
//  {"collectgarbage", luaB_collectgarbage},
//  {"dofile", luaB_dofile},
//  {"error", luaB_error},
//  {"getmetatable", luaB_getmetatable},
//  {"ipairs", luaB_ipairs},
//  {"loadfile", luaB_loadfile},
//  {"load", luaB_load},
//#if defined(LUA_COMPAT_LOADSTRING)
//  {"loadstring", luaB_load},
//#endif
//  {"next", luaB_next},
//  {"pairs", luaB_pairs},
//  {"pcall", luaB_pcall},
//  {"print", luaB_print},
//  {"rawequal", luaB_rawequal},
//  {"rawlen", luaB_rawlen},
//  {"rawget", luaB_rawget},
//  {"rawset", luaB_rawset},
//  {"select", luaB_select},
//  {"setmetatable", luaB_setmetatable},
//  {"tonumber", luaB_tonumber},
//  {"tostring", luaB_tostring},
//  {"type", luaB_type},
//  {"xpcall", luaB_xpcall},
//  /* placeholders */
//  {"_G", NULL},
//  {"_VERSION", NULL},
//  {NULL, NULL}
//};

static const luaL_Reg base_funcs[] = {
	{ "assert", luaB_assert },
	{ "authors", luaB_authors },
	{ "collectgarbage", luaB_collectgarbage },
	{ "clear", luaB_clear },
	{ "cls", luaB_clear },
	{ "description", luaB_getDescription },
	{ "desc", luaB_getDescription },
	{ "dofile", luaB_dofile },
	{ "error", luaB_error },
	{ "exit", luaB_exit },
	{ "getmetatable", luaB_getmetatable },
	{ "help", luaB_help },
	{ "ipairs", luaB_ipairs },
	{ "loadfile", luaB_loadfile },
	{ "load", luaB_load },
#if defined(LUA_COMPAT_LOADSTRING)
	{ "loadstring", luaB_load },
#endif
	{ "next", luaB_next },
	{ "pairs", luaB_pairs },
	{ "pcall", luaB_pcall },
	{ "print", luaB_print },
	{ "rawequal", luaB_rawequal },
	{ "rawlen", luaB_rawlen },
	{ "rawget", luaB_rawget },
	{ "rawset", luaB_rawset },
	{ "select", luaB_select },
	{ "setmetatable", luaB_setmetatable },
	{ "tonumber", luaB_tonumber },
	{ "tostring", luaB_tostring },
	{ "type", luaB_type },
	{ "usage", luaB_usage },
	{ "version", luaB_version },
	{ "xpcall", luaB_xpcall },
	{ "ztest", luaB_ztest },
	/* placeholders */
	{ "_G", NULL },
	//{ "_VERSION", NULL },
	{ NULL, NULL }
};

LUAMOD_API int luaopen_base (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "_G");
  /* set global _VERSION */
  //lua_pushliteral(L, LUA_VERSION);
  //lua_setfield(L, -2, "_VERSION");
  return 1;
}
typedef struct luaL_FuncNameNotePair {
	const char *name;
	const char* note;
} luaL_FuncNameNotePair;
static const luaL_FuncNameNotePair base_funcs_note_chs[] = {
	{ "assert", "如果其参数 v 的值为假（nil 或 false）， 它就调用 error； 否则，返回所有的参数。 在错误情况时， message 指那个错误对象； 如果不提供这个参数，参数默认为 \"assertion failed!\" 。" },
	{ "authors", "显示lua核心库软件作者" },
	{ "collectgarbage", "垃圾回收 \n \"stop\": 停止垃圾回收。\n \"restart\": 重启垃圾回收。\n \"collect\" : 执行垃圾回收的完整循环。\n \"count\" : 返回 Lua 当前使用的总内存（单位为 Kb）。\n \"step\" : 执行一步（一步可由多步组成）垃圾回收。步数可由参数 arg 控制（值越大，步数越多，0 表示执行一步（指最小的一步））。如果执行后完成了回收循环，返回 true。\n \"setpause\" : 把 arg / 100 作为垃圾回收参数 pause 的新值。\n \"setstepmul\" : 把 arg / 100 作为垃圾回收参数 step mutiplier 的新值。" },
	{ "clear", "清除当前屏幕内容" },
	{ "cls", "清除当前屏幕内容，与clear一样" },
	{ "dofile", "打开该名字的文件，并执行文件中的 Lua 代码块" },
	{ "error", "终止所保护的函数，抛出 message 消息，不再返回" },
	{ "exit", "退出环境" },
	{ "gcinfo", "获取gc信息" },
	{ "getmetatable", "如果对象没有元表，返回空；如果对象有 __metatable 域，返回对应的值；否则，返回对象的元表" },
	{ "help", "显示帮助" },
	{ "ipairs", "返回三个值：迭代器、传入的表 t、值 0 。迭代器能够根据传入的表 t 和索引 i 得到 i+1 和 t[i+1] 的值。" },
	{ "loadfile", "从文件中获取代码块，如果没有指定文件，则从标准输入中获取" },
	{ "load", "通过传入函数 func 的返回值获取代码块片段。func 函数的后一次调用返回的字符串应当能与前一次调用返回的字符串衔接在一起，最终得到完整的代码块。函数返回 nil 或无返回值时表示代码块结束。 \n load 函数会将得到的代码块作为函数返回。返回函数的环境为全局环境。如果出现错误，load 会返回 nil 和 错误信息。 \n chunkname 作为该代码块的名称，用在错误信息与调试信息中。" },
	{ "loadstring", "与 load 类似，只不过是从字符串中获取代码块" },
	{ "next", "返回传入的表中下一个键值对" },
	{ "pcall", "以保护模式调用传入的函数，也就是说不会抛出错误。如果捕获到抛出的错误，第一个参数返回 false，第二个参数返回错误信息；如果没有出现错误，第一个参数返回 ture，后面的参数返回传入函数的返回值。" },
	{ "print", "仅作为快速查看某个值的工具，不用做格式化输出。正式的格式化输出见 string.format 与 io.write。" },
	{ "rawequal", "以 raw 作为前缀的函数均表示该方法在不触发任何元方法的情况下调用。\n rawequal 检查 v1 是否与 v2 相等，返回比较结果。" },
	{ "rawget", "获取 table 中键 index 的关联值，table 参数必须是一个表，找不到返回 nil 。" },
	{ "rawset", "将 table[index] 的值设置为 value 。table 必须是一张表，index 不能是 nil 或 NaN 。value 可以是任何值。返回修改后的 table 。" },
	{ "select", "index 可以是数字或者字符 '#' 。当 index 为数字时，返回第 index + 1 个参数及后面的参数（支持负数形式的 index）；当 index 为 '#' 时，返回参数的个数（不包括第一个参数）。" },
	{ "setmetatable", "给 table 关联元表 metatable 。返回参数 table 。\n 如果元表定义了 __metatable 域，会抛出错误。\n metatable 参数（第二参数）为 nil 表示解除已经关联的元表。" },
	{ "tonumber", " 尝试把 e （第一参数）转换为十进制数值并返回。如果无法转换返回 nil 。 \n base（第二参数） 表示传入参数的进制，默认为 10 进制。base 的可输入范围[2, 36]。高于 10 的数字用字母表示，A - Z 分别表示 11 - 35 。" },
	{ "tostring", "能将任意类型的值转换为合适的字符串形式返回。要控制数字转换为字符串的方式，使用 string.format(formatstring,...) 。 \n 如果值所关联的元表有 __tostring 域，则使用该域的元方法获取字符串。" },
	{ "type", "返回 v 的类型，类型以字符串形式返回。 有以下八种返回值： \"nil\" ， \"number\"， \"string\"， \"boolean\"， \"table\"， \"function\"， \"thread\"， \"userdata\"。" },
	{ "unpack", "返回表中的各个域的值,等价于：\n return list[i], list[i+1], ···, list[j]" },
	{ "usage", "显示lua帮助" },
	{ "version", "显示lua版本号" },
	{ "xpcall", "与 pcall (f, arg1, ...) 类似。不同的是，如果 f 函数抛出了错误，那么 xpcall 不会返回从 f 抛出的错误信息，而是使用 err 函数返回的错误信息。" },
	{ "ztest", "暂无文档" }
};

static const luaL_FuncNameNotePair base_funcs_note_fre[] = {
	{ "assert", " Si la valeur de ses paramètres de V est faux (0 ou faux), qui appelle une erreur; sinon, le retour de tous les paramètres.En cas d'erreur, le message de l'objet d'erreur; si elle ne fournit pas de ce paramètre, un paramètre par défaut pour \"assertion failed!\"." },
	{ "authors", " Bibliothèque centrale de logiciel d'auteur lua." },
	{ "collectgarbage", " Recyclage d'ordures \n \"Stop\": arrête de recyclage d'ordures.\ n \"redémarrage \": le redémarrage de recyclage d'ordures.\ n \"collect \": le cycle complet de la mise en œuvre de déchets de recyclage.\ n \"count\": le retour de mémoire totale de l'utilisation actuelle de lua (unité pour KB).\ n \"Step \": une étape d'exécution (étape peut être composée de plusieurs étapes de récupération de déchets).Le nombre de pas à partir de paramètres de commande peuvent être Arg (plus la valeur de nombre de pas plus de 0, et une étape d'exécution (étape un minimum)).Si la mise en œuvre de l'achèvement de la récupération de circulation, renvoie VRAI.\ n \"setpause \": ARG / 100 comme nouvelle valeur de paramètres de récupération de pause d'ordures.\ n \"setstepmul \": ARG / 100 comme nouvelle valeur de paramètres de récupération step mutiplier d'ordures." },
	{ "clear", " Efface le contenu d'écran actuelle." },
	{ "cls", " Efface le contenu d'écran actuel, et clair comme." },
	{ "dofile", " Ouvrir un fichier pour le nom de fichier, et de mettre en œuvre dans le bloc de code lua." },
	{ "error", " La fonction de protection de fin de message, lance le message, pas de retour." },
	{ "exit", " Le retrait de l'environnement." },
	{ "gcinfo", " Pour acquérir des informations de GC." },
	{ "getmetatable", "Si l'objet n'a pas d'élément de table, de revenir à vide; si l'objet a _ _ chaîne de domaine correspondant à la valeur de retour; sinon, retour des objets de la table d'éléments." },
	{ "help", " L'affichage d'aide." },
	{ "ipairs", " Le retour de trois valeurs: des itérateurs, tableau T, passé de valeur 0.Le générateur selon I T et de la table d'index entrant la valeur i+1 et t[i+1]." },
	{ "loadfile", "À partir d'un fichier d'un bloc de code, s'il n'y a pas de fichier spécifié, à partir de l'entrée standard d'" },
	{ "load", " En fonction de la valeur de retour entrant pour obtenir des segments de bloc de code de fonction.Chaîne de func fonction après une fois de retour d'appel devrait être en mesure de chaîne avant et une fois l'appel de retour reliés l'un à l'autre, et, finalement, l'intégrité du bloc de code.Lorsque la fonction de retour à zéro ou non la valeur de retour dudit code fin de bloc.\ n Load fonction sera de blocs de code en fonction de retour.Retour de l'environnement en fonction de l'environnement global.Si une erreur se produit, load  des informations de retour nil  et les erreurs.\n  Chunkname  comme le nom de blocs de code, utilisé dans des informations de débogage et de fausses informations." },
	{ "loadstring", " Et de charge similaires, mais dans une chaîne de blocs de code d'acquisition." },
	{ "next", " Renvoie une table en entrant une valeur de clé pour." },
	{ "pcall", " En fonction de modes de protection d'un appel entrant, c'est - à - dire ne lance une erreur.Si l'erreur de capture à la lance, le premier paramètre renvoie false, le second paramètre renvoie un message d'erreur; si aucune erreur, le premier paramètre de température de true, derrière les paramètres de retour entrant en fonction de la valeur de retour." },
	{ "print", " Uniquement comme un outil de vérification rapide d'une valeur de pas faire de format de sortie.Voir string.format officiellement de format de sortie et io.write." },
	{ "rawequal", " En fonction de brut comme préfixe ont exprimé le cas dans cette méthode de déclenchement de toute yuan appel de procédé.V1 et V2 de vérifier rawequal \n si égale, et renvoie le résultat de la comparaison." },
	{ "rawget", " Association de valeurs de clé d'index de table table de paramètres, doit être une table, ne trouve pas de retour nil." },
	{ "rawset", " La valeur de table [index] agencé pour value.Une table est une table, index ne peut pas être nil ou NaN.La valeur peut être n'importe quelle valeur.De retour après la modification de la table." },
	{ "select", " Index peuvent être des nombres ou caractères \"#\.Lorsqu'un index numérique, les paramètres de renvoyer un index + 1 de paramètres et à l'arrière (index de soutien négatif); lorsque l'#' retour, le nombre de paramètres (non compris le premier paramètre)." },
	{ "setmetatable", " Une table d'éléments à Association de metatable.Paramètres de retour de la table.\ n Si Yuan feuille définit __metatable, lance une erreur.\n Chaîne de paramètres (deuxième paramètre) est nul de matériel déjà associé à la table d'éléments." },
	{ "tonumber", " Essayer de mettre e (premier paramètre) pour convertir le nombre de décimales et de retour.Si vous ne pouvez pas une transition revient à zéro.\ n de base (un second paramètre) indiquant des paramètres par défaut pour le binaire entrant, base 10.La base peut être de la plage d'entrée [2, 36].Supérieure à 10 chiffres par des lettres, a - Z représentent respectivement 11 - 35." },
	{ "tostring", " Peut être tout type de conversion de la valeur de retour sous forme de chaîne de caractères approprié.Pour la commande de la conversion numérique de la forme d'une chaîne de caractères, à l'aide de l' string.format(formatstring,...).\ n si les valeurs de corrélation de la table d'éléments a __tostring de domaine, qui utilise le procédé à éléments de domaine d'une chaîne." },
	{ "type", " Type de retour de type V, sous la forme d'une chaîne de retour.Les huit types de valeur de retour: \"nil\" ,  \"number\" , \"string\" , \"boolean\" , \"table\" , \"function\" , \"thread\" , \"userdata\"." },
	{ "unpack", " Valeur de retour dans la table, chaque domaine équivalent à: \n return List [i], liste [i + 1], · · ·, liste [j]" },
	{ "usage", " Afficher l'aide lua." },
	{ "version", " Affiche le numéro de version lua." },
	{ "xpcall", " Et pcall F, ARG1,...) similaires.C'est différent, si la fonction f lance une erreur, alors xpcall ne renvoie des informations d'erreur de f jeté, un message d'erreur mais l'utilisation de la fonction renvoie Err." },
	{ "ztest", " Pas de document." }
};
static const luaL_FuncNameNotePair base_funcs_note_eng[] = {
	{ "assert", " If the value of the parameter V is false (nil or false), it calls error; otherwise, all parameters are returned. In the error case, message refers to that error object; if this parameter is not supplied, the parameter defaults to \"assertion failed\"." },
	{ "authors", " Display Lua core library software author." },
	{ "collectgarbage", " Garbage collection \n \"stop\":stop garbage recycling. \n \"restart\": restart garbage collection. \n \"collect\": executes the full cycle of garbage collection. \n \"count\": returns the total memory used by Lua (Kb). \n  \"step\": perform a step (step by step composition) garbage collection. The number of steps can be controlled by the parameter Arg (the larger the number, the more steps, and the 0 indicates the execution step (the smallest step). If the recycle cycle is completed after execution, returns true. \n \"setpause\": take Arg / 100 as the new value of the garbage collection parameter pause. \n \"setstepmul\": take Arg / 100 as the new value of the garbage collection parameter step mutiplier." },
	{ "clear", " Clear the current screen content." },
	{ "cls", " Clear the contents of the current screen, just like clear." },
	{ "dofile", " Open the file with the name and execute the Lua code block in the file." },
	{ "error", " Terminates the protected function, throws the message message, and returns none." },
	{ "exit", " Exit environment." },
	{ "gcinfo", " Get GC information." },
	{ "getmetatable", " If the object does not have a meta table, it returns empty; if the object has a __metatable field, the corresponding value is returned; otherwise, the meta table of the object is returned." },
	{ "help", " Display help." },
	{ "ipairs", " Returns three values: the iterator, the incoming table t, and the value 0. The iterator can obtain the values of i+1 and t[i+1] based on the incoming table t and index i." },
	{ "loadfile", " Gets the code block from the file. If no file is specified, get it from the standard input." },
	{ "load", " Gets a block of code fragments through the return value of the incoming function func. The string returned by the subsequent call of the func function should be able to link to the string returned from the previous call and eventually get the complete block of code. When the function returns nil or returns no value, it indicates the end of the block.  \n The load function returns the resulting block of code as a function. The environment that returns the function is the global environment. If an error occurs, load returns the nil and error messages. \n Chunkname is used as the name of the block of code and is used in error information and debug information." },
	{ "loadstring", " Like load, it's just getting code blocks from strings." },
	{ "next", " Returns the next key key pair in the incoming table." },
	{ "pcall", " Call the incoming function in protected mode, that is, no errors are thrown. If caught out of the error, the first parameter returns false, the second parameter error information is returned; if no error occurs, the first parameter returns ture, behind the returned value to a function parameter." },
	{ "print", " As a tool for quickly looking at a value, you do not have to format output. For formal formatted output, see string.format and io.write." },
	{ "rawequal", " Functions that use raw as a prefix indicate that the method calls without triggering any meta methods. \n Rawequal checks whether V1 is equal to V2 and returns a comparison result." },
	{ "rawget", " Gets the associated value of the key table in the index. The table parameter must be a table and cannot find the return nil." },
	{ "rawset", " Set the value of table[index] to value. Table must be a table, and index cannot be nil or NaN. Value can be any value. Returns the modified table." },
	{ "select", " Index can be a number or character'#'. When index is digital, parameter returns the index + 1 Parameters and behind (support the negative form of index); when index is'#', returns the number of parameters (not including the first parameter)." },
	{ "setmetatable", " Give the \"table\" to associated element table \"metatable\". Return parameter table. \n if the meta table defines the domain, an error is thrown. \n The metatable parameter (second argument) is nil to indicate that the associated meta table has been lifted." },
	{ "tonumber", " Try converting e (first parameter) to decimal value and return it. Returns nil if it cannot be converted. \n The base (second arguments) represents the hexadecimal of the incoming parameter, and defaults to 10. Parameter base has an input range of [2, 36]. Letters higher than 10 are represented by letters, and A - Z represent 11-35 respectively." },
	{ "tostring", " Converts any type of value to the proper string type. To control the conversion of numbers to strings, use string.format (FormatString,...). \n If the value associated with the meta table has the __tostring field, the string is obtained using the meta method of the field." },
	{ "type", " Returns the type of v, and the type returns in string. There are eight kinds of return values: \" nil\",\" number\", \"string\",\"boolean\", \"table\", \" function\", \"thread\", \"userdata\"." },
	{ "unpack", "Returns the value of each domain in the table, which is equivalent to: \n, return, list[i], list[i+1], list[j]" },
	{ "usage", " Display Lua help." },
	{ "version", " Display the Lua version number." },
	{ "xpcall", " Similar to pcall (F, arg1,...). The difference is that if the F function throws an error, the xpcall does not return the error information thrown from the f, but instead uses the error information returned by the err function." },
	{ "ztest", " No document." }
};
#ifdef _WIN32
//------------------------------------------------------------------------------
//
// Clears the screen
//
//------------------------------------------------------------------------------
static void clrscr()
{
	COORD coordScreen = { 0, 0 };
	DWORD cCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD dwConSize;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	GetConsoleScreenBufferInfo(hConsole, &csbi);
	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
	FillConsoleOutputCharacter(hConsole, TEXT(' '), dwConSize, coordScreen, &cCharsWritten);
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);
	SetConsoleCursorPosition(hConsole, coordScreen);
}
#else
#include <stdio.h>
#include <unistd.h>

// 清除屏幕
#define CLEAR() printf("\033[2J")

// 上移光标
#define MOVEUP(x) printf("\033[%dA", (x))

// 下移光标
#define MOVEDOWN(x) printf("\033[%dB", (x))

// 左移光标
#define MOVELEFT(y) printf("\033[%dD", (y))

// 右移光标
#define MOVERIGHT(y) printf("\033[%dC",(y))

// 定位光标
#define MOVETO(x,y) printf("\033[%d;%dH", (x), (y))

// 光标复位
#define RESET_CURSOR() printf("\033[H")

// 隐藏光标
#define HIDE_CURSOR() printf("\033[?25l")

// 显示光标
#define SHOW_CURSOR() printf("\033[?25h")

//反显
#define HIGHT_LIGHT() printf("\033[7m")
#define UN_HIGHT_LIGHT() printf("\033[27m")
#endif
static int luaB_clear(lua_State *L){
#ifdef _WIN32
	clrscr();
#else
	CLEAR();
#endif
	return 0;
}

#define LUA_PROGNAME		"lua"
static const char *progname = LUA_PROGNAME;
static void print_usage(FILE* f) {
	fprintf(f,
		"usage: %s [options] [script [args]].\n"
		"Available options are:\n"
		"  -e stat  execute string " LUA_QL("stat") "\n"
		"  -l name  require library " LUA_QL("name") "\n"
		"  -i       enter interactive mode after executing " LUA_QL("script") "\n"
		"  -v       show version information\n"
		"  --       stop handling options\n"
		"  -        execute stdin and stop handling options\n"
		,
		progname);
	fflush(stderr);
}

void Test(lua_State *L);
static void Test(lua_State *L)
{
	const char* text1 = "This is a easter egg.";
	luaB_print_spec(L, text1);
	const char* text3 = "Easter egg author:Q. John";
	luaB_print_spec(L, text3);
}
static int luaB_ztest(lua_State *L) {
	Test(L);
	return 0;
}
static int luaB_authors(lua_State *L){/*Almost as a heretic in c++, left braces do not line up.*/
	return luaB_print_spec(L, LUA_AUTHORS);
}
static int luaB_exit(lua_State *L){
	exit(0);
	return 0;
}
static int luaB_usage(lua_State *L){
	print_usage(stdout);
	return 0;
}

static int luaB_print_spec(lua_State *L, const char* cc) {
	fputs(cc, stdout);
	lua_pop(L, 1);  /* pop result */
	fputs("\n", stdout);
	return 0;
}
void luaB_print_spec3(const char* cc1, const char* cc2);
static int luaB_help(lua_State *L){
	LANGID lid = GetSystemDefaultLangID();
	size_t size_chs = sizeof(base_funcs_note_chs) / sizeof(luaL_FuncNameNotePair);
	size_t size_eng = sizeof(base_funcs_note_eng) / sizeof(luaL_FuncNameNotePair);
	size_t size_fre = sizeof(base_funcs_note_fre) / sizeof(luaL_FuncNameNotePair);
	assert(size_chs == size_eng);
	assert(size_chs == size_fre);
	assert(size_fre == size_eng);
	char result[100];
	sprintf_s(result, sizeof(result), "Functions size is:%d", size_chs);
	luaB_print_spec(L, result);
	switch (lid)
	{
	case 0x0804:
	case 0x1004:
		//case 0x0c04://Hong Kong
		//case 0x0404://Taiwan
	{
				   for (size_t i = 0; i < size_chs; i++)
				   {
					   luaB_print_spec3((base_funcs_note_chs[i]).name, (base_funcs_note_chs[i]).note);
				   }
				   break;
	}
	case 0x0409:
	case 0x0809:
	case 0x0c09:
	case 0x1009:
	case 0x1409:
	case 0x1809:
	case 0x1c09:
	case 0x2009:
	case 0x2409:
	case 0x2809:
	case 0x2c09:
	{
				   for (size_t i = 0; i < size_eng; i++)
				   {
					   luaB_print_spec3((base_funcs_note_eng[i]).name, (base_funcs_note_eng[i]).note);
				   }
				   break;
	}
	case 0x040c:
	case 0x080c:
	case 0x0c0c:
	case 0x100c:
	case 0x140c:
	{
				   for (size_t i = 0; i < size_fre; i++)
				   {
					   luaB_print_spec3((base_funcs_note_fre[i]).name, (base_funcs_note_fre[i]).note);
				   }
				   break;
	}
	}
	return 0;
}
static int luaB_version(lua_State *L) {
	return luaB_print_spec(L, LUA_RELEASE);
}
const char* ____Description_chs = "Lua是一个小巧的脚本语言。是巴西里约热内卢天主教大学（Pontifical Catholic University of Rio de Janeiro）里的一个研究小组，由Roberto Ierusalimschy、Waldemar Celes 和 Luiz Henrique de Figueiredo所组成并于1993年开发。 其设计目的是为了嵌入应用程序中，从而为应用程序提供灵活的扩展和定制功能。Lua由标准C编写而成，几乎在所有操作系统和平台上都可以编译，运行。Lua并没有提供强大的库，这是由它的定位决定的。所以Lua不适合作为开发独立应用程序的语言。Lua 有一个同时进行的JIT项目，提供在特定平台上的即时编译功能。\n Lua脚本可以很容易的被C / C++ 代码调用，也可以反过来调用C / C++的函数，这使得Lua在应用程序中可以被广泛应用。不仅仅作为扩展脚本，也可以作为普通的配置文件，代替XML, ini等文件格式，并且更容易理解和维护。Lua由标准C编写而成，代码简洁优美，几乎在所有操作系统和平台上都可以编译，运行。 一个完整的Lua解释器不过200k，在目前所有脚本引擎中，Lua的速度是最快的。这一切都决定了Lua是作为嵌入式脚本的最佳选择。 ";
const char* ____Description_eng = "Lua is a small scripting language. Brazil Catholic University of Rio De Janeiro (Pontifical Catholic University of Rio de Janeiro) a research team, Roberto Ierusalimschy, Waldemar Celes by Luiz Henrique and de Figueiredo formed and developed in 1993. The design is designed to embed applications, providing flexible extension and customization capabilities for applications. Lua is written by standard C and can be compiled and run on almost all operating systems and platforms. Lua does not provide a powerful library, which is determined by its location. Therefore, Lua is not suitable as a language for developing independent applications. Lua has a simultaneous JIT project that provides immediate compile capabilities on a particular platform. \n Lua scripts can easily be called by C / C++ code, and can also call C / C++ functions in turn, which makes Lua available for applications in a wide variety of applications. Not only as an extension script, but also as an ordinary configuration file instead of XML, ini and other file formats, and easier to understand and maintain. Lua is written by standard C, which is simple and elegant, and can be compiled and run on almost all operating systems and platforms. A complete Lua interpreter, but 200K, is the fastest Lua in all scripting engines. All this determines that Lua is the best choice as an embedded script.";
static int luaB_getDescription(lua_State *L){
	assert(sizeof(____Description_chs) / sizeof(const char)-sizeof(____Description_eng) / sizeof(const char) >= 0);
	LANGID lid = GetSystemDefaultLangID();
	switch (lid)
	{
	case 0x0804:
	case 0x1004:
		//case 0x0c04://Hong Kong
		//case 0x0404://Taiwan
	{
				   luaB_print_spec(L, ____Description_chs);
				   break;
	}
	case 0x0409:
	case 0x0809:
	case 0x0c09:
	case 0x1009:
	case 0x1409:
	case 0x1809:
	case 0x1c09:
	case 0x2009:
	case 0x2409:
	case 0x2809:
	case 0x2c09:
	{
				   luaB_print_spec(L, ____Description_eng);
				   break;
	}
	default:
	{
			   break;
	}
	}
	return 0;
}
static void luaB_print_spec3(const char* cc1, const char* cc2)
{
	printf(cc1);
	printf(cc2);
	printf("\n");
}