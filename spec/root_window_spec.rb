# -*- coding:utf-8; mode:ruby; -*-

require 'active_window_x'

include ActiveWindowX

describe RootWindow do
  before do
    @display_raw = double Xlib::Display
    @display_raw.stub(:kind_of?).with(Data).and_return(true)
    Xlib.stub(:x_open_display).and_return(@display_raw)
    @display = Display.new
    @root_id = 9999
    Xlib.stub(:default_root_window).with(@display_raw).and_return(@root_id)
    @root = @display.root_window
  end

  describe '#active_window' do
    before do
      @prop_atom = 111
      @active_window_id = 123456
      Xlib.should_receive(:x_intern_atom).with(@display.raw, '_NET_ACTIVE_WINDOW', false).
        and_return(@prop_atom)
    end
    context ', which Xlib::x_get_window_property find an active window,' do
      before do
        Xlib.should_receive(:x_get_window_property).
          with(@display.raw, @root.id, @prop_atom, 0, Window::READ_BUFF_LENGTH, false, Xlib::AnyPropertyType).
          and_return([0,32,2,0,[@active_window_id,0].pack('l!*')])
      end
      it 'should return the active windows' do
        @display.active_window.id.should == @active_window_id
      end
    end
    context ', which Xlib::x_get_window_property cannot find an active window,' do
      before do
        Xlib.should_receive(:x_get_window_property).
          with(@display.raw, @root.id, @prop_atom, 0, Window::READ_BUFF_LENGTH, false, Xlib::AnyPropertyType).
          and_return([0,32,2,0,[Xlib::None,0].pack('l!*')])
      end
      it 'should return nil' do
        @display.active_window.should == nil
      end
    end
  end

end
