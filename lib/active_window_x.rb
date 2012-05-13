# -*- coding:undecided-unix; mode:ruby; -*-

require "active_window_x/version"
require "active_window_x/display"
require "active_window_x/window"
require "active_window_x/root_window"

module ActiveWindowX
  require "active_window_x/xlib"
  class Display; end
  class Window; end
  class RootWindow < Window; end
end
