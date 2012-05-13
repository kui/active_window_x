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

  describe 'active_window' do
    before do
      @id = 123456
    end
  end
end
