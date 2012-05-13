# -*- coding:utf-8; mode:ruby; -*-

require 'active_window_x'

include ActiveWindowX

describe Xlib do
  before do
    @display = mock Xlib::Display
    @id = 123456
    @window = Window.new @display, @id
  end

  describe '#root' do
    before do
      @root = Window.new @display, 111
      Xlib.should_receive(:query_tree).
        with(@display, @window).
        and_return([@root, nil, nil])
    end
    it 'should return the root Window' do
      @window.root.should == @root
    end
  end

  describe '#parent' do
    before do
      @parent = Window.new @display, 111
      Xlib.should_receive(:query_tree).
        with(@display, @window).
        and_return([nil, @parent, nil])
    end
    it 'should return the parent Window' do
      @window.parent.should == @parent
    end
  end

  describe '#children' do
    before do
      @children = []
      @children.push Window.new @display, 111
      @children.push Window.new @display, 222
      @children.push Window.new @display, 333
      Xlib.should_receive(:query_tree).
        with(@display, @window).
        and_return([nil, nil, @children])
    end
    it 'should return the child Windows' do
      @window.children.should == @children
    end
  end
end
