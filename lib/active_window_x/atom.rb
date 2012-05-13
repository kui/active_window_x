# -*- coding:utf-8; mode:ruby; -*-

module ActiveWindowX

  # binding for Atom on X11
  class Atom < Xid
    @@cache = {}

    def initialize display, second
      if second.kind_of? Numeric
        super
      elsif second.kind_of? String
        id = display.intern_atom second
        if id == Xlib::None
          raise ArgumentError, 'invalid an atom name: #{second}'
        end
        super
      else
        raise ArgumentError, 'expect Numeric or String with the second argument'
      end

      @@cache[@display.raw] ||= {}
      @cache = @@cache[@display.raw]
    end

    alias :intern :id

    def name
      if @cache.has_key? @id
        @cache[@id]
      else
        @cache[@id] = Xlib::x_get_atom_name @display.raw, @id
      end
    end
  end

end
