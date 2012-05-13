# -*- coding:utf-8; mode:ruby; -*-

module ActiveWindowX

  # binding for Atom on X11
  class Atom < Xid
    @@cache = {}

    def initialize display, second
      if second.kind_of? Numeric
        super
      elsif second.kind_of? String
        super
        @id = display.intern_atom second
        if @id == Xlib::None
          raise ArgumentError, 'invalid an atom name: #{second}'
        end
      else
        raise ArgumentError, 'expect Numeric or String with the second argument'
      end
    end

    alias :intern :id

    def name
      @display.atom_name @id
    end
  end

end
