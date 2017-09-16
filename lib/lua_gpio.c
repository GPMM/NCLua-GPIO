#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "../lib/GPIO.h"

#define INPUT  0
#define OUTPUT 1

#define LOW  0
#define HIGH 1

/*
   static int
   lgpio_init (lua_State *L)
   {
   init_gpio();
   return 0;
   }
 */

// TODO Refactor for better interface with lua  TODO

static int
lgpio_setmode (lua_State *L)
{
  int pin, mode;

  pin = luaL_checkint(L, 1);
  mode = luaL_checkint(L, 2);

  switch (mode)
  {
    case INPUT:
      INP_GPIO(pin);
      break;
    case OUTPUT:
      INP_GPIO(pin);
      OUT_GPIO(pin);
      break;
  }

  return 0;
}

static int
lgpio_setup (lua_State *L)
{
  int pin, status;

  pin = luaL_checkint(L, 1);
  status = luaL_checkint(L, 2);

  switch (status)
  {
    case HIGH:
      GPIO_SET = 1 << pin;
      break;
    case LOW:
      GPIO_CLR = 1 << pin;
      break;
  }

  return 0;
}

static int
lgpio_cleanup (lua_State *L)
{
  int pin;

  pin = luaL_checkint(L, 1);

  if (GET_GPIO(pin))
  {
    GPIO_CLR = 1 << pin;
  }

  return 0;
}

static int
lgpio_close (lua_State *L)
{
  close_gpio();
  return 0;
}


static const struct
luaL_Reg funcs[] = {
  //  { "init",     lgpio_init },
  { "setmode",  lgpio_setmode },
  { "setup",    lgpio_setup },
  { "cleanup",  lgpio_cleanup },
  { "__gc",     lgpio_close },
  { NULL,       NULL }
};

int luaopen_lgpio (lua_State *L);

int
luaopen_lgpio (lua_State *L)
{
  init_gpio();
  luaL_newmetatable(L, "lgpio");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_setfuncs(L, funcs, 0);
  //luaL_newlib(L, funcs);

  return 1;
}
