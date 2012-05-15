# -*- coding:utf-8; mode:ruby; -*-

module ActiveWindowX

  # binding for Window on X11
  class Window < Xid

    # a buffer for #x_get_window_property
    READ_BUFF_LENGTH = 1024

    def x_query_tree
      Xlib::x_query_tree @display.raw, @id
    end

    # a return value of XQueryTree
    # which is the root window for a display contains this window
    def root
      (r = x_query_tree[0]) and Window.new(@display, r)
    end

    # a return value of XQueryTree
    # which is nil, if this window is RootWindow, or a Window.
    def parent
      (r = x_query_tree[1]) and Window.new(@display, r)
    end

    # a return value of XQueryTree
    # which is an Array of Window
    def children
      x_query_tree[2].map{|w| Window.new(@display, w)}
    end

    # window title (web page title in browser, current command or dir in terminal app, etc)
    def title
      title = prop('_NET_WM_NAME')
      title or prop('WM_NAME')
    end

    # window property getter with easy way for XGetWindowProperty
    # which return nil, if the specified property name does not exist,
    # a String or a Array of Number
    def prop atom
      val, format, nitems = prop_raw atom
      case format
      when 32; val.unpack("l!#{nitems}")
      when 16; val.unpack("s#{nitems}")
      when 8; val[0, nitems]
      when 0; nil
      end
    end

    # window property getter with easy way for XGetWindowProperty
    # which return [propety_value, format, number_of_items]
    def prop_raw atom
      if atom.kind_of?(Numeric) or atom.kind_of?(String)
        atom = Atom.new @display, atom
      elsif not atom.kind_of? Atom
        raise ArgumentError, "expect Numeric, String or #{Atom.name}"
      end
      actual_type, actual_format, nitems, bytes_after, val =
        Xlib::x_get_window_property @display.raw, @id, atom.id, 0, READ_BUFF_LENGTH, false, Xlib::AnyPropertyType
      return [val, actual_format, nitems]
    end

    # Array of the property atom ID(Numeric) list for this window
    def prop_atom_ids
      r = Xlib::x_list_properties @display.raw, @id
      r.nil? ? [] : r
    end

    # Array of the property atom list for this window
    def prop_atoms
      prop_atom_ids.map{|i| Atom.new @display, i}
    end

    def select_input mask
      Xlib::x_select_input @display.raw, @id, mask
    end
  end

end
