# -*- coding:utf-8; mode:ruby; -*-

require 'active_window_x'

include ActiveWindowX

describe RootWindow do
  before do
    @display = Display.new
    @root = @display.root_window
  end
  after do
    @display.close
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
