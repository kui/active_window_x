require "mkmf"

if have_library('X11')
  create_makefile "xlib"
else
  raise "Cannot found X11"
end
