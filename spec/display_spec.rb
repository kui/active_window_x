# -*- coding:utf-8; mode:ruby; -*-

require 'active_window_x'

include ActiveWindowX

describe Xlib do
  before do
    @display = Display.new nil
  end
  after do
    @display.close
  end

  describe '#root_window' do
    before do
      @root_id = 123456
      Xlib.should_receive(:default_root_window).and_return(@root_id)
    end
    it 'should return the root window' do
      r = @display.root_window
      r.id.should == @root_id
      r.should be_a RootWindow
    end
  end

  describe '#next_event' do
    context 'with PropertyChangeMask'
    before do
      @root = @display.root_window
      @root.select_input Xlib::PropertyChangeMask
      @event = mock Xlib::XPropertyEvent
      Xlib.should_receive(:x_next_event).and_return(@event)
    end
    it 'should return a PropertyEvent' do
      ev = @display.next_event
      ev.type.should == @event.type
    end
  end

end
