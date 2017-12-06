lgpio = require('lgpio')

local assert = assert
local engine = require('nclua.event.engine')

local gpio

do
  gpio = engine.new()
  gpio.class = 'gpio'
  gpio.INPUT = 0
  gpio.OUTPUT = 1
  gpio.LOW = 0
  gpio.HIGH = 1
  gpio.NONE = 0
  gpio.RISING = 1
  gpio.FALLING = 2
  gpio.BOTH = 3
end

function gpio:add_event(gpio, edge)
  lgpio.add_event(gpio, edge)
end

function gpio:get_events()
  local evts = {}
  local evt = lgpio.get_event()
  while evt ~= nil
  do
    table.insert(evts, {
          class = 'gpio',
          gpio = evt.gpio,
          value = evt.value
        })
    evt = lgpio.get_event()
  end
  return evts
end

function gpio:remove_event(gpio)
  lgpio.remove_event(gpio)
end

function gpio:setup(gpio, mode)
  lgpio.set(gpio, mode)
end

function gpio:input(gpio)
  local value = lgpio.input(gpio)
  return value
end

function gpio:output(gpio, value)
  lgpio.output(gpio, value)
end

-- XXX NCLua methods XXX --

function gpio:check (evt)
  assert(evt.class == gpio.class)
  return evt
end

function gpio:filter (class)
  assert(evt.class == gpio.class)
  return { class = class }
end

function gpio:cycle ()
  evts = gpio:get_events()
  if evts ~= nil then
    for _,evt in ipairs(evts) do
      gpio.OUTQ:enqueue(evt)
    end
  end
  if not gpio.INQ:is_empty () then
    gpio.OUTQ:enqueue (gpio.INQ:dequeue (#gpio.INQ))
  end
end

return gpio

-- XXX EOF XXX --
