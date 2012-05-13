# -*- coding:utf-8; mode:ruby; -*-

require 'active_window_x'

include ActiveWindowX

describe Xlib do
  before do
    @raw_display = mock Xlib::Display
    @display = Display.new @raw_display
  end

  describe '#root' do
    before do
      @root_id = 123456
      Xlib.should_receive(:default_root_window).and_return(@root_id)
    end
    it 'should return the root window' do
      @display.root.id.should == @root_id
    end
  end

  describe '#active_window' do
    before do
      @root_id = 123
      @prop_atom = 111
      @id = 123456
      Xlib.should_receive(:x_intern_atom).with(@display, '_NET_ACTIVE_WINDOW').
        and_return(@prop_atom)
    end
    context ', which Xlib::x_get_window_property find an active window,' do
      before do
        Xlib.should_receive(:x_get_window_property).
          with(@display, @root_id, @prop_atom, 0, Display::READ_BUFF_LENGTH, false, Xlib::AnyPropertyType).
          and_return([0,32,2,0,[@id,0].pack('l!*')])
      end
      it 'should return the active windows' do
        @display.active_window.id.should == @id
      end
    end
    context ', which Xlib::x_get_window_property cannot find an active window,' do
      before do
        Xlib.should_receive(:x_get_window_property).
          with(@display, @root_id, @prop_atom, 0, Display::READ_BUFF_LENGTH, false, Xlib::AnyPropertyType).
          and_return([0,32,2,0,[Xlib::None,0].pack('l!*')])
      end
      it 'should return nil' do
        @display.active_window.should == nil
      end
    end
  end
end
