# -*- coding:utf-8; mode:ruby; -*-

require 'active_window_x'
require 'dl'

include ActiveWindowX

describe Xlib do
  describe '.#x_open_display' do
    context 'with nil' do
      it 'should return a Display' do
        Xlib::x_open_display(nil).class.should == Xlib::Display
      end
    end
    context 'with the default display name' do
      it 'should return a Display' do
        Xlib::x_open_display(ENV['DISPLAY']).class.should == Xlib::Display
      end
    end if ENV['DISPLAY']
    context 'with an invalid display name' do
      it 'should raise a exception' do
        lambda{ Xlib::x_open_display('foo') }.should raise_error(Xlib::UnknownDisplayName)
      end
    end
  end

  describe '.#x_close_display' do
    context 'with a Display' do
      it 'should return 0' do
        d = Xlib::x_open_display(nil)
        Xlib::x_close_display(d).should == 0
      end
    end
  end

  describe '.#get_input_focus' do
    before do
      @display = Xlib::x_open_display(nil)
    end
    after do
      Xlib::x_close_display @display
    end
    it 'should return a Array of Number (for window id)' do
      result = Xlib::x_get_input_focus(@display)
      result[0].should be_a Numeric
      result[1].should be_within(0).of(2)
    end
  end

  describe '.#x_query_tree' do
    before do
      @display = Xlib::x_open_display(nil)
    end
    after do
      Xlib::x_close_display @display
    end
    context 'with the root window' do
      before do
        @root = Xlib::default_root_window(@display)
      end
      it 'should return the root, the parent and then children' do
        result = Xlib::x_query_tree(@display, @root)
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
        result = Xlib::x_query_tree(@display, @root)
        children = result[2]
        @window = children.first
      end
      it 'should return the root, the parent and then children' do
        result = Xlib::x_query_tree(@display, @window)
        result.shift.should == @root
        result.shift.should be_a Numeric
        children = result.shift
        children.each do |w|
          w.should be_a Numeric
        end
      end
    end
  end

  describe '.#intern_atom' do
    before do
      @display = Xlib::x_open_display(nil)
    end
    after do
      Xlib::x_close_display @display
    end
    context "with a atom name" do
      it 'should return a Numeric' do
        Xlib::x_intern_atom(@display, "Name", false).should be_a Numeric
      end
    end
  end

  describe '.#get_atom_name' do
    before do
      @display = Xlib::x_open_display(nil)
      @name = "Name"
      @atom = Xlib::x_intern_atom(@display, @name, false)
    end
    after do
      Xlib::x_close_display @display
    end
    context "with a atom" do
      it 'should return the atom name' do
        Xlib::x_get_atom_name(@display, @atom).should == @name
      end
    end
  end

  describe '.#get_window_property' do
    before do
      @display = Xlib::x_open_display nil
      @window = Xlib::default_root_window @display
      @length = 1024
    end
    after do
      Xlib::x_close_display @display
    end
    context 'with unknown property' do
      before do
        @name = "FOO"
        @atom = Xlib::x_intern_atom @display, @name, false
      end
      it 'should return a Array such as [Xlib::None, ]' do
        r = Xlib::x_get_window_property(@display, @window, @atom, 0, @length, false, Xlib::AnyPropertyType);
        r.shift.should == Xlib::None
        r.shift.should == 0
        r.shift.should == 0
        r.shift.should == 0
        r.length.should == 1
      end
    end
    context 'with an invalid type and a propery named as "_NET_ACTIVE_WINDOW"' do
      before do
        @name = "_NET_ACTIVE_WINDOW"
        @atom = Xlib::x_intern_atom @display, @name, false
        @window_atom = Xlib::x_intern_atom(@display, "WINDOW", false)
      end
      it 'should return a Array such as [Xlib::None, ]' do
        r = Xlib::x_get_window_property(@display, @window, @atom, 0, @length, false, 10)
        p r
        r.shift.should == @window_atom
        r.shift.should == 32
        r.shift.should == 0
        r.shift.should # == DL::sizeof('l')
        r.length.should == 1
      end
    end
    context 'with an validi type and a propery named as "_NET_ACTIVE_WINDOW"' do
      before do
        @name = "_NET_ACTIVE_WINDOW"
        @atom = Xlib::x_intern_atom @display, @name, false
        @window_atom = Xlib::x_intern_atom(@display, "WINDOW", false)
      end
      it 'should return a Array such as [Xlib::None, ]' do
        r = Xlib::x_get_window_property(@display, @window, @atom, 0, @length, false, @window_atom)
        p r
        r.shift.should == @window_atom
        r.shift.should == 32
        l = r.shift
        r.shift.should
        r.shift.length == DL::sizeof('l') * l
      end
    end
  end

  describe '.#list_properties' do
    before do
      @display = Xlib::x_open_display nil
      @root = Xlib::default_root_window @display
    end
    after do
      Xlib::x_close_display @display
    end
    context 'with the root window' do
      it 'should return a Array of Numeric' do
        arr = Xlib::x_list_properties(@display, @root)
        arr.should be_a Array
        arr.each{|a| a.should be_a Numeric; }
      end
    end
    context 'with child windows of the root window' do
      before do
        arr = Xlib::x_query_tree @display, @root
        @children = arr[2]
      end
      it 'should return a Array of Numeric' do
        @children.each do |child|
          arr = Xlib::x_list_properties(@display, child)
          if arr
            arr.should be_a Array
            arr.each{|a| a.should be_a Numeric }
          else
            arr.should be_nil
          end
        end
      end
    end
  end

end if ENV.has_key? 'DISPLAY' # if X Window System running, this spec should be executed
