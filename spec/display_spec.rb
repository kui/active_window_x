# -*- coding:utf-8; mode:ruby; -*-

require 'active_window_x'

include ActiveWindowX

describe Display do
  before do
    @display_raw = double Xlib::Display
    Xlib.stub(:x_open_display).and_return(@display_raw)
    @display = Display.new nil
    @root_id = 9999
    Xlib.stub(:default_root_window).with(@display_raw).and_return(@root_id)
    Xlib.stub(:x_select_input).and_return(1)
  end

  describe '#root_window' do
    it 'should return the root window' do
      r = @display.root_window
      r.id.should == @root_id
      r.should be_a RootWindow
    end
  end

  describe '#next_event' do
    before do
      @root = @display.root_window
      @root.select_input Xlib::PropertyChangeMask
      @event = double Xlib::XPropertyEvent
      @event.stub(:type){@type}
      @event.stub(:serial){1000}
      @event.stub(:send_event){0}
      @event.stub(:time){2000}
      @event.stub(:state){nil}
      @window_id = 222
      @event.stub(:window){@window_id}
      @atom_id = 333
      @event.stub(:atom){@atom_id}
      Xlib.should_receive(:x_next_event).and_return(@event)
    end
    context 'with PropertyChangeMask' do
      before do
        @type = Xlib::PropertyNotify
      end
      it 'should return a PropertyEvent' do
        ev = @display.next_event
        ev.type.should == @event.type
        ev.window.id.should == @event.window
        ev.atom.id.should == @event.atom
      end
    end
    context 'with other event type' do
      before do
        @type = 0
      end
      it 'should return a PropertyEvent' do
        ev = @display.next_event
        ev.type.should == @event.type
      end
    end
  end

end
