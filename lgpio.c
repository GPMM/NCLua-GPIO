/* github.com/renatoor */

#include "gpio_lib.h"
#include "gpio_event.h" 

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define LGPIO "lgpio"

/*

  callback function to enqueue event

*/
void
gpio_callback (unsigned int gpio, int value)
{
  gpio_evt_t *evt;

  evt = new_gpio_evt(gpio, value);

  if (evt == NULL)
  {
    fprintf(stderr, "Failed to create event.\n");
    return;
  }

  enqueue(evt);
  return;
}

/*

  lgpio.add_event(gpio, edge)

  gpio - int:gpio value
  edge - int:rising, falling, both, none

  add event detection for gpio

*/
static int
l_add_event (lua_State *L)
{
  int gpio, edge;  

  gpio = luaL_checkint(L, 1);
  edge = luaL_checkint(L, 2);

  add_gpio_event(gpio, edge, gpio_callback);  

  return 0;
}

/*

  lgpio.remove_event(gpio)

  gpio - int:gpio value

  remove event detection for gpio

*/
static int
l_remove_event (lua_State *L)
{
  int gpio;

  gpio = luaL_checkint(L, 1);
  
  remove_gpio_event(gpio);

  return 0;
}

/*

  lgpio.get_event()

  retrieve one event available from queue
  if queue is empty return nil

*/
static int
l_get_event (lua_State *L)
{
  gpio_evt_t *evt;

  evt = dequeue();

  if (evt == NULL)
  {
    lua_pushnil(L);
    return 1;
  }

  lua_newtable(L);
  lua_pushstring(L, "gpio");
  lua_pushnumber(L, evt->gpio);
  lua_settable(L, -3);
  lua_pushstring(L, "value");
  lua_pushnumber(L, evt->value);
  lua_settable(L, -3);

  free(evt);

  return 1;
}

/*

  lgpio.setup(gpio, mode)

  gpio - int:gpio value
  mode - int:input, output

*/
static int
l_setup (lua_State *L)
{
  int gpio, mode;

  gpio = luaL_checkint(L, 1);
  mode = luaL_checkint(L, 2);

  switch(mode)
  {
    case INPUT:
      INP_GPIO(gpio);
      break;
    case OUTPUT:
      INP_GPIO(gpio);
      OUT_GPIO(gpio);
      break;
  }

  return 0;
}

/*

  lgpio.input(gpio)

  gpio - int:gpio number

  return current value from gpio

*/
static int
l_input (lua_State *L)
{
  int gpio, value;

  gpio = luaL_checkint(L, 1);
  value = GET_GPIO(gpio);
  lua_pushnumber(L, value);

  return 1;
}

/*

  lgpio.output(gpio, state)

  gpio - int:gpio number
  state - int:HIGH=1 LOW=0

  set gpio to state

*/
static int
l_output (lua_State *L)
{
  int gpio, state;

  gpio = luaL_checkint(L, 1);
  state = luaL_checkint(L, 2);

  if (state != GET_GPIO(gpio))
  {
    switch (state)
    {
      case HIGH:
        GPIO_SET = 1 << gpio;
        break;
      case LOW:
        GPIO_CLR = 1 << gpio;
        break;
    }
  }


  return 0;
}

/* GC */
static int
l_cleanup (lua_State *L)
{
  close_gpio();
  free_all();
  return 0;
}


static const struct luaL_Reg funcs[] = {
  /* gpio_event */
  { "add_event",    l_add_event },
  { "remove_event", l_remove_event },
  { "get_event",    l_get_event },
  /* gpio_lib */
  { "setup",        l_setup },
  { "input",        l_input },
  { "output",       l_output },
  /* clean up */
  { "__gc",         l_cleanup },
  {  NULL,          NULL }
};

int luaopen_lgpio (lua_State *L);

int
luaopen_lgpio (lua_State *L)
{
  init_gpio();
  luaL_newmetatable(L, LGPIO);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_setfuncs(L, funcs, 0);
  return 1;
}

/* XXX EOF XXX */
