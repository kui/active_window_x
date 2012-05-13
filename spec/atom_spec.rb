# -*- coding:utf-8; mode:ruby; -*-

require 'active_window_x'

include ActiveWindowX

describe Window do
  before do
    @raw_display = mock Xlib::Display
    @display = mock Display
    @display.stub(:raw){@raw_display}
    @display.stub(:kind_of?).with(Display).and_return(true)
    @id = 123
    @atom = Atom.new @display, @id
  end

  describe '#name' do
    before do
      @name = 'FOOO'
      Xlib.should_receive(:x_get_atom_name).with(@display.raw, @id).
        and_return(@name)
    end
    it 'should return a String as an atom name' do
      @atom.name.should == @name
      @atom.name.should == @name
    end
  end
end
