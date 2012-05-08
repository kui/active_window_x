# -*- coding:utf-8; mode:ruby; -*-

require 'active_window_x'

include ActiveWindowX

describe Xlib do
  describe '.open_display' do
    context 'with nil' do
      it 'should return a Display' do
        Xlib::open_display(nil).class.should == Xlib::Display
      end
    end
    context 'with the default display name' do
      it 'should return a Display' do
        Xlib::open_display(ENV['DISPLAY']).class.should == Xlib::Display
      end
    end if ENV['DISPLAY']
    context 'with an invalid display name' do
      it 'should raise a exception' do
        lambda{ Xlib::open_display('foo') }.should raise_error(Xlib::UnknownDisplayName)
      end
    end
  end
  describe '.close_display' do
    context 'with a Display' do
      it 'should return 0' do
        d = Xlib::open_display(nil)
        Xlib::close_display(d).should == 0
      end
    end
  end
end
