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
      @display.root_window.id.should == @root_id
    end
  end

end
