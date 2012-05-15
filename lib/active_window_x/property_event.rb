# -*- coding:utf-8; mode:ruby; -*-

module ActiveWindowX

  # binding for XPropertyEvent on X11
  class PropertyEvent < Event

    # the number of last request processed by server
    attr_reader :serial

    # true if this came from a SendEvent request
    attr_reader :send_event

    # Display the event was read from
    attr_reader :display

    # the window whose associated property was changed
    attr_reader :window

    # the property's atom and indicates which property was changed or desired
    attr_reader :atom

    # the server time when the property was changed
    attr_reader :time

    # * PropertyNewValue when a property of the window is changed using
    #   XChangeProperty or XRotateWindowProperties (even when adding zero-length
    #   data using XChangeProperty) and when replacing all or part of a property
    #   with identical data using XChangeProperty or XRotateWindowProperties.
    # * PropertyDelete when a property of the window is deleted using
    #   XDeleteProperty or, if the delete argument is True, XGetWindowProperty
    attr_reader :state

    def initialize display, raw
      super
      @serial = raw.serial
      @send_event = (raw.send_event != 0)
      @display = display
      @window = Window.new display, raw.window
      @atom = Atom.new display, raw.atom
      @time = raw.time
      @state = raw.state
    end
  end

end
