# -*- coding:utf-8; mode:ruby; -*-

module ActiveWindowX

  # binding for XClientMessageEvent on X11
  class ClientMessageEvent < Event

    # the number of last request processed by server
    attr_reader :serial

    # true if this came from a SendEvent request
    attr_reader :send_event

    # Display the event was read from
    attr_reader :display

    # the window whose associated property was changed
    attr_reader :window

    # an atom that indicates how the data should be interpreted
    # by the receiving client
    attr_reader :message_type

    # 8, 16, or 32 and specifies whether the data should be viewed
    # as a list of bytes, shorts, or longs
    attr_reader :format

    # a union that contains the members b, s, and l.  The b, s, and l members
    # represent data of twenty 8-bit values, ten 16-bit values,
    # and five 32-bit values
    attr_reader :data

    def initialize display, raw
      super
      @serial = raw.serial
      @send_event = (raw.send_event != 0)
      @display = display
      @window = Window.new display, raw.window
      @message_type = Atom.new display, raw.message_type
      @format = raw.message_type
      @data = raw.data
    end
  end
end
