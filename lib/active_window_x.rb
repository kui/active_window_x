# -*- coding:undecided-unix; mode:ruby; -*-

require "active_window_x/version"
require "active_window_x/display"
require "active_window_x/xid"
require "active_window_x/window"
require "active_window_x/root_window"
require "active_window_x/atom"
require "active_window_x/event"
require "active_window_x/property_event"

module ActiveWindowX
  require "active_window_x/xlib"

  class Display; end

  class XID; end
  class Window < Xid; end
  class RootWindow < Window; end
  class Atom < Xid; end

  class Event; end
  class PropertyEvent < Event; end
end
