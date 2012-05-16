require "mkmf"

if have_library('X11')
  create_makefile "active_window_x/xlib"
else
  raise "Cannot found X11"
end
