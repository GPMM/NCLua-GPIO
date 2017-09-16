#!/usr/bin/env lua

-- TODO Move file to test dir and fix require path issues TODO

require('os')
local lgpio = require('lgpio')

--lgpio.init()
lgpio.setmode(18, 1)
lgpio.setup(18, 1)
os.execute('sleep 1')
lgpio.setup(18, 0)
