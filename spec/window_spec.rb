# -*- coding:utf-8; mode:ruby; -*-

require 'active_window_x'

include ActiveWindowX

describe Xlib do
  before do
    @raw_display = mock Xlib::Display
    @display = mock Display
    @display.stub(:raw){@raw_display}
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
end
