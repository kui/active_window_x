# -*- coding:utf-8-unix; mode:ruby; -*-
# a script to repro a freeze bug which is occurred when x_next_event is called after BadWindow
# the freeze do not accept INT signal

require 'active_window_x'

include ActiveWindowX

@display = Xlib::x_open_display nil

# raise BadWindow
begin
  Xlib::x_query_tree @display, 1
rescue Xlib::XErrorEvent
end

puts :freeze
Xlib::x_next_event @display
