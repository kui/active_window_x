# -*- coding:utf-8; mode:ruby; -*-

require "active_window_x/window"

module ActiveWindowX

  # binding for a root Window on X11
  class RootWindow < Window

    def active_window
      prop_val = prop '_NET_ACTIVE_WINDOW'
      if prop_val.nil? or prop_val.first == Xlib::None
        nil
      else
        Window.new(@display, prop_val.first)
      end
    end

  end
end
