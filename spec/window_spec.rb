# -*- coding:utf-8; mode:ruby; -*-

require 'active_window_x'

include ActiveWindowX

describe Window do
  before do
    @raw_display = mock Xlib::Display
    @display = mock Display
    @display.stub(:raw){@raw_display}
    @display.stub(:kind_of?).with(Display).and_return(true)
    @id = 123456
    @window = Window.new @display, @id
  end

  describe '#root' do
    context ', which recive a window from x_query_tree,' do
      before do
        @root = 111
        Xlib.should_receive(:x_query_tree).
          with(@display.raw, @window.id).
          and_return([@root, nil, []])
      end
      it 'should return the root Window' do
        @window.root.id.should == @root
      end
    end
    context ', which recive nil from x_query_tree,' do
      before do
        Xlib.should_receive(:x_query_tree).
          with(@display.raw, @window.id).
          and_return([nil, nil, []])
      end
      it 'should return nil' do
        @window.root.should be_nil
      end
    end
  end

  describe '#parent' do
    context ', which recive a window from x_query_tree,' do
      before do
        @parent = 111
        Xlib.should_receive(:x_query_tree).
          with(@display.raw, @window.id).
          and_return([nil, @parent, []])
      end
      it 'should return the parent Window' do
        @window.parent.id.should == @parent
      end
    end
    context ', which recive nil from x_query_tree,' do
      before do
        Xlib.should_receive(:x_query_tree).
          with(@display.raw, @window.id).
          and_return([nil, nil, []])
      end
      it 'should return nil' do
        @window.root.should be_nil
      end
    end
  end

  describe '#children' do
    before do
      @children = []
      @children.push 111
      @children.push 222
      @children.push 333
      Xlib.should_receive(:x_query_tree).
        with(@display.raw, @window.id).
        and_return([nil, nil, @children])
    end
    it 'should return the child Windows' do
      c = @window.children
      c[0].id.should == @children[0]
      c[1].id.should == @children[1]
      c[2].id.should == @children[2]
    end
  end

  describe '#title' do
    context ', when the _NET_WM_NAME and WM_NAME prop was not found,' do
      before do
        @window.stub(:prop).with('_NET_WM_NAME').and_return(nil)
        @window.stub(:prop).with('WM_NAME').and_return(nil)
      end
      it 'should return nil' do
        @window.title.should be_nil
      end
    end
    context ', when the WM_NAME prop was found,' do
      before do
        @title = 'foo bar baz'
        @window.stub(:prop).with('_NET_WM_NAME').and_return(nil)
        @window.stub(:prop).with('WM_NAME').and_return(@title)
      end
      it 'should return the value' do
        @window.title.should == @title
      end
    end
    context ', when the _NET_WM_NAME prop was found,' do
      before do
        @title = 'foo bar baz'
        @window.stub(:prop).with('_NET_WM_NAME').and_return(@title)
      end
      it 'should return the value' do
        @window.title.should == @title
      end
    end
  end

  describe '#app_name' do
    context ', when the class prop was not found,' do
      before do
        @window.stub(:prop).with('WM_CLASS').and_return(nil)
      end
      it 'should return nil' do
        @window.app_name.should be_nil
      end
    end
    context ', when the class prop was found,' do
      before do
        @name = "bar"
        @class = "foo"
        @value = "#{@name}\0#{@class}\0"
        @window.stub(:prop).with('WM_CLASS').and_return(@value)
      end
      it 'should return the value' do
        @window.app_name.should == @name
      end
    end
  end

  describe '#app_class' do
    context ', when the class prop was not found,' do
      before do
        @window.stub(:prop).with('WM_CLASS').and_return(nil)
      end
      it 'should return nil' do
        @window.app_class.should be_nil
      end
    end
    context ', when the class prop was found,' do
      before do
        @name = "bar"
        @class = "foo"
        @value = "#{@name}\0#{@class}\0"
        @window.stub(:prop).with('WM_CLASS').and_return(@value)
      end
      it 'should return the value' do
        @window.app_class.should == @class
      end
    end
  end

  describe '#pid' do
    context ', when the pid prop was not found,' do
      before do
        @window.stub(:prop).with('_NET_WM_PID').and_return(nil)
      end
      it 'should return nil' do
        @window.pid.should be_nil
      end
    end
    context ', when the pid prop was found,' do
      before do
        @pid = 1111111
        @window.stub(:prop).with('_NET_WM_PID').and_return([@pid])
      end
      it 'should return nil' do
        @window.pid.should == @pid
      end
    end
  end

  describe '#command' do
    context ', when the pid prop was not found,'
    context ', when the pid prop was found,'
  end

  describe '#prop' do
    before do
      @prop_id = 1234
      @prop_name = 'FOO'
      @display.should_receive(:intern_atom).with(@prop_name).and_return(@prop_id)
    end
    context 'with a property name, which does not exist for the window,' do
      before do
        Xlib.should_receive(:x_get_window_property).
          with(@display.raw, @window.id, @prop_id, 0, Window::READ_BUFF_LENGTH, false, Xlib::AnyPropertyType).
          and_return([nil, 0, 0, 0, nil])
      end
      it 'should return nil' do
        @window.prop(@prop_name).should be_nil
      end
    end
    context 'with a property name, which exist for the window and is the long type,' do
      before do
        @prop = [123, 456]
        Xlib.should_receive(:x_get_window_property).
          with(@display.raw, @window.id, @prop_id, 0, Window::READ_BUFF_LENGTH, false, Xlib::AnyPropertyType).
          and_return([@prop_id, 32, @prop.length, 0, @prop.pack('l!*')])
      end
      it 'should return a Array of Numeric' do
        @window.prop(@prop_name).should == @prop
      end
    end
    context 'with a property name, which exist for the window and is the short type,' do
      before do
        @prop = [12, 34, 46]
        Xlib.should_receive(:x_get_window_property).
          with(@display.raw, @window.id, @prop_id, 0, Window::READ_BUFF_LENGTH, false, Xlib::AnyPropertyType).
          and_return([@prop_id, 16, @prop.length, 0, @prop.pack('s*')])
      end
      it 'should return a Array of Numeric' do
        @window.prop(@prop_name).should == @prop
      end
    end
    context 'with a property name, which exist for the window and is the char type,' do
      before do
        @prop = "abcdefg\0hijklmn"
        Xlib.should_receive(:x_get_window_property).
          with(@display.raw, @window.id, @prop_id, 0, Window::READ_BUFF_LENGTH, false, Xlib::AnyPropertyType).
          and_return([@prop_id, 8, @prop.length, 0, @prop])
      end
      it 'should return String' do
        @window.prop(@prop_name).should == @prop
      end
    end
  end

  describe '#prop_atom_ids' do
    context ', which recieve property atoms(Numeric),' do
      before do
        @prop_list = [000, 111, 222]
        Xlib.should_receive(:x_list_properties).and_return(@prop_list)
      end
      it 'shuold return the atoms' do
        @window.prop_atom_ids.should == @prop_list
      end
    end
    context ', which recieve no property atoms,' do
      before do
        @prop_list = nil
        Xlib.should_receive(:x_list_properties){ @prop_list }
      end
      it 'shuold return an empty array' do
        @window.prop_atom_ids.should == []
      end
    end
  end

  describe '#select_input' do
    context 'with a valid mask' do
      before do
        @mask = Xlib::PropertyChangeMask
        Xlib.should_receive(:x_select_input).with(@display.raw, @id, @mask).and_return(1)
      end
      it 'should call Xlib::x_select_input' do
        @window.select_input @mask
      end
    end
  end
end
