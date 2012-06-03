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

    # _name_ : a display name. if you give nil, EventListener use a Default Display
    # _timeout_ : an intervel to check an interaction for the termination of the listening loop
    def initialize name=nil, timeout=DEFAULT_TIMEOUT, &block
      @display = Display.new name
      @default_timeout = timeout

      @root = @display.root_window
      @aw_atom = Atom.new @display, '_NET_ACTIVE_WINDOW'
      @name_atom = Atom.new @display, 'WM_NAME'
      @delete_atom = Atom.new @display, 'WM_DELETE_WINDOW'
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
          next if not event

          yield event if event.type
        end
      ensure
        destroy
      end
    end

    def destroy
      @display.close
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

      event = listen_with_no_select

      event
    end

    def listen_with_no_select
      type = nil
      active_window = nil

      event = @display.next_event
      if event.class != PropertyEvent
        return nil
      end
      if event.atom == @aw_atom
        type = :active_window
        active_window = @root.active_window
      elsif event.atom == @name_atom
        type = :title
        active_window = event.window
      end

      if type == :active_window and @active_window != active_window
        @active_window.select_input(Xlib::NoEventMask) if @active_window
        @active_window = active_window
        @active_window.select_input(Xlib::PropertyChangeMask) if @active_window
      end

      event = Event.new type, active_window
      if window_closed?(event.window)
        event.window = @root.active_window
      end

      event
    end

    def pending_events_num
      @display.pending
    end

    def connection
      @display.connection
    end

    def window_closed? w
      return false if w.nil?
      begin
        w.prop_raw 'WM_STATE'
        false
      rescue Xlib::XErrorEvent
        true
      end
    end

    class Event
      attr_accessor :type, :window
      def initialize type, window
        @type = type; @window = window
      end
    end
  end
end
