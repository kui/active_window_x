# -*- coding:utf-8; mode:ruby; -*-

module ActiveWindowX

  # listen event changing active window
  class EventListener

    def initialize name=nil
      @display = Display.new name
    end

    def start
      root = @display.root_window
      aw_atom = @display.intern_atom '_NET_ACTIVE_WINDOW'
      root.select_input Xlib::PropertyChangeMask

      while true
      end
    end

  end
end
