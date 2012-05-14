# -*- coding:utf-8; mode:ruby; -*-

module ActiveWindowX

  # binding for XID on X11
  class Xid
    # a display which has this XID
    attr_reader :display

    # raw XID (#define Window unsinged long)
    attr_reader :id

    def initialize display, id
      if display.kind_of? Display
        @display = display
      elsif display.kind_of? Xlib::Display
        @display = Display.new display
      else
        raise ArgumentError, "expect #{Display.name} or #{Xlib::Display.name}"
      end
      @id ||= id
    end

    def == xid
      xid.kind_of?(Xid) and (xid.id == @id)
    end
  end

end
