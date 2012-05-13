# -*- coding:utf-8; mode:ruby; -*-

module ActiveWindowX

  # event type
  attr_reader :type

  # binding for XEvent on X11
  class Event
    def initialize display, raw
      @type = raw.type
    end
  end

end
