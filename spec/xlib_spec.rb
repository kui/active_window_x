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

  describe '.get_input_focus' do
    before do
      @display = Xlib::open_display(nil)
    end
    after do
      Xlib::close_display @display
    end
    it 'should return a Array of Number (for window id)' do
      result = Xlib::get_input_focus(@display)
      result[0].should be_a Numeric
      result[1].should be_within(0).of(2)
    end
  end

  describe '.get_default_root' do
  end

  describe '.query_tree' do
    before do
      @display = Xlib::open_display(nil)
    end
    after do
      Xlib::close_display @display
    end
    context 'with the root window' do
      before do
        @root = Xlib::default_root_window(@display)
      end
      it 'should return the root, the parent and then children' do
        result = Xlib::query_tree(@display, @root)
        result.shift.should == @root
        result.shift.should be_nil
        children = result.shift
        children.each do |w|
          w.should be_a Numeric
        end
      end
    end
    context 'with a child window of the root window' do
      before do
        @root = Xlib::default_root_window(@display)
        result = Xlib::query_tree(@display, @root)
        children = result[2]
        @window = children.first
      end
      it 'should return the root, the parent and then children' do
        result = Xlib::query_tree(@display, @window)
        result.shift.should == @root
        result.shift.should be_a Numeric
        children = result.shift
        children.each do |w|
          w.should be_a Numeric
        end
      end
    end
  end

  describe '.intern_atom' do
    before do
      @display = Xlib::open_display(nil)
    end
    after do
      Xlib::close_display @display
    end
    context "with a atom name" do
      it 'should return a Numeric' do
        Xlib::intern_atom(@display, "Name", false).should be_a Numeric
      end
    end
  end

  describe '.get_atom_name' do
    before do
      @display = Xlib::open_display(nil)
      @name = "Name"
      @atom = Xlib::intern_atom(@display, @name, false)
    end
    after do
      Xlib::close_display @display
    end
    context "with a atom" do
      it 'should return the atom name' do
        Xlib::get_atom_name(@display, @atom).should == @name
      end
    end
  end

  describe '.get_window_property' do
    before do
      @display = Xlib::open_display nil
      @window = Xlib::default_root_window @display
      @name = "Name"
      @atom = Xlib::intern_atom @display, @name, false
      @length = 1024
    end
    after do
      Xlib::close_display @display
    end
    context 'with ' do
      it 'should return ' do
        p Xlib::get_window_property(@display, @window, @atom, 0, @length, false, Xlib::AnyPropertyType);
      end
    end
  end

  describe '.list_properties' do
    before do
      @display = Xlib::open_display nil
      @window = Xlib::default_root_window @display
    end
    after do
      Xlib::close_display @display
    end
    context 'with ' do
      it 'should return ' do
        arr = Xlib::list_properties(@display, @window)
        arr.should be_a Array
        arr.each{|a| p Xlib::get_atom_name(@display, a)}
      end
    end
  end
end
