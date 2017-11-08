lgpio = require('lgpio')

local gpio = {
  class = 'gpio',
  INPUT = 0,
  OUTPUT = 1,
  LOW = 0,
  HIGH = 1,
  NONE = 0,
  RISING = 1,
  FALLING = 2,
  BOTH = 3
}

function gpio:add_event(gpio, edge)
  lgpio.add_event(gpio, edge)
end

function gpio:get_events()
  local evts = {}
  local evt = lgpio.get_event()
  while evt ~= nil
  do
    table.insert(evts, {
          gpio = evt['gpio'],
          value = evt['value']
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

function output(gpio, value)
  lgpio.output(gpio, value)
end

-- XXX EOF XXX --
