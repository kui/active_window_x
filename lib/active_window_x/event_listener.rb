# -*- coding:utf-8; mode:ruby; -*-

module ActiveWindowX

  # listen event changing active window
  class EventListener

    DEFAULT_TIMEOUT = 0.5

    # current active window
    attr_reader :active_window

    # true  if #start loop continued
    # false if #start loop did not continue
    # nil   if #start loop was not started
    # set false if you want to terminate #start loop when next timeout or event receiving
    attr_accessor :continue

    def initialize name=nil, timeout=DEFAULT_TIMEOUT, &block
      @display = Display.new name
      @default_timeout = timeout

      @root = @display.root_window
      @aw_atom = @display.intern_atom '_NET_ACTIVE_WINDOW'
      @name_atom = @display.intern_atom 'WM_NAME'
      @conn = @display.connection
      @active_window = @root.active_window

      @active_window.select_input Xlib::PropertyChangeMask if @active_window
      @root.select_input Xlib::PropertyChangeMask

      if block_given?
        start @default_timeout, &block
      end
    end

    # event listener loop
    #
    # ActiveWindowX::EventListener.new.start do |e|
    #   puts e.type, e.window.id
    # end
    def start timeout=@default_timeout

      if not block_given?
        raise ArgumentError, 'expect to give a block'
      end

      @continue = true
      begin
        while @continue
          event = listen timeout
          yield event if event and event.type
        end
      ensure
        @display.close if @display.closed?
      end
    end

    # receive a event
    #
    # return value:
    #  a ActiveWindowX::EventListener::Event if an event was send within _timeout_ sec
    #  nil if timeout
    def listen timeout=nil
      if @display.pending == 0 and
          select([@conn], [], [], timeout) == nil
        # ope on timeout
        return nil
      end

      event = @display.next_event
      if event.atom.id == @aw_atom
        type = :active_window
        active_window = @root.active_window
      elsif event.atom.id == @name_atom
        type = :title
        active_window = event.window
      else
        type = nil
        active_window = nil
      end

      if type == :active_window and @active_window != active_window
        @active_window.select_input(Xlib::NoEventMask) if @active_window
        @active_window = active_window
        @active_window.select_input(Xlib::PropertyChangeMask) if @active_window
      end

      Event.new type, active_window
    end

    class Event
      attr_reader :type, :window
      def initialize type, window
        @type = type; @window = window
      end
    end
  end
end
