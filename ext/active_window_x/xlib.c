// -*- coding:utf-8; mode:c; -*-

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

#include <ruby.h>

#include <X11/Xlib.h>

#define GetDisplay(obj, d) {\
    Data_Get_Struct(obj, Display, d);           \
  }
#define GetXTextProperty(obj, t) {\
    Data_Get_Struct(obj, XTextProperty, t);     \
  }

VALUE xlib_module;
VALUE display_class;
VALUE x_text_property_class;
VALUE unknown_display_name_class;
VALUE x_error_event_class;

// XOpenDisplay
VALUE xlib_open_display(VALUE self, VALUE name_obj) {
  const char* name;
  Display* d;
  if (TYPE(name_obj) == T_NIL){
    name = NULL;
  }else{
    name = (const char*) StringValuePtr(name_obj);
  }
  d = XOpenDisplay(name);

  if (d == NULL) {
    rb_raise(unknown_display_name_class, "invalid name: %s", name);
    return Qnil;
  }
  return Data_Wrap_Struct(display_class, 1, -1, d);
}

// XCloseDisplay
VALUE xlib_close_display(VALUE self, VALUE display_obj) {
  Display* d;
  GetDisplay(display_obj, d);
  return INT2FIX((int) XCloseDisplay(d));
}

// XGetInputFocus
VALUE xlib_get_input_focus(VALUE self, VALUE display_obj) {
  Display* d;
  Window window;
  int revert_to;
  VALUE arr_obj;

  GetDisplay(display_obj, d);
  XGetInputFocus(d, &window, &revert_to);

  arr_obj = rb_ary_new2(2L);
  rb_ary_push(arr_obj, window == None ? Qnil : ULONG2NUM((unsigned long) window));
  rb_ary_push(arr_obj, INT2FIX(revert_to));

  return arr_obj;
}

// XQueryTree
VALUE xlib_query_tree(VALUE self, VALUE d, VALUE w){
  Display* display;
  Window window;
  Window root;
  Window parent;
  Window* children;
  unsigned int nchildren, i;
  VALUE arr_obj, children_obj;

  GetDisplay(d, display);
  window = NUM2ULONG(w);
  if(!XQueryTree(display, window, &root, &parent, &children, &nchildren))
    rb_raise(rb_eRuntimeError, "fail to execute XQueryTree");

  arr_obj = rb_ary_new2(3L);
  rb_ary_push(arr_obj, root == None ? Qnil : ULONG2NUM((unsigned long) root));
  rb_ary_push(arr_obj, parent == None ? Qnil : ULONG2NUM((unsigned long) parent));
  children_obj = rb_ary_new2((long) nchildren);
  for(i=0; i < nchildren; i++)
    rb_ary_push(children_obj, ULONG2NUM((unsigned long) children[i]));
  rb_ary_push(arr_obj, children_obj);

  XFree(children);

  return arr_obj;
}

// DefaultRootWindow
VALUE xlib_default_root_window(VALUE self, VALUE d) {
  Display* display;
  Window root;
  GetDisplay(d, display);
  root = DefaultRootWindow(display);
  return root == None ? Qnil : ULONG2NUM(root);
}

static VALUE xlib_intern_atom(VALUE self, VALUE d, VALUE name_obj, VALUE b) {
  Display* display;
  Atom atom;
  char *name;
  Bool bool;

  GetDisplay(d, display);
  name = StringValuePtr(name_obj);
  bool = (b == Qfalse || b == Qnil) ? False : True;
  atom = XInternAtom(display, name, bool);

  return atom == None ? Qnil : ULONG2NUM((unsigned long) atom);
}

static VALUE xlib_get_atom_name(VALUE self, VALUE d, VALUE atom_obj){
  Display* display;
  Atom atom;
  char *name;

  GetDisplay(d, display);
  atom = NUM2ULONG(atom_obj);

  name = XGetAtomName(display, atom);

  return name == NULL ? Qnil : rb_str_new2(name);
}

#define ERROR_MESSAGE_BUFF 256
int error_handler(Display* d, XErrorEvent* error_event){
  char desc[ERROR_MESSAGE_BUFF];
  int length = ERROR_MESSAGE_BUFF;
  XGetErrorText(d, error_event->error_code, desc, length);
  rb_raise(x_error_event_class, "%s", desc);
  return 1;
}

void Init_xlib(void){

  setlocale(LC_ALL, "");
  XSetErrorHandler(error_handler);

  xlib_module = rb_define_module("Xlib");
  display_class = rb_define_class_under(xlib_module, "Display", rb_cData);
  x_text_property_class = rb_define_class_under(xlib_module, "XTextProperty", rb_cData);
  unknown_display_name_class =
    rb_define_class_under(xlib_module, "UnknownDisplayName", rb_eRuntimeError);
  x_error_event_class =
    rb_define_class_under(xlib_module, "XErrorEvent", rb_eRuntimeError);

  rb_define_singleton_method(xlib_module, "open_display", xlib_open_display, 1);
  rb_define_singleton_method(xlib_module, "close_display", xlib_close_display, 1);
  rb_define_singleton_method(xlib_module, "get_input_focus", xlib_get_input_focus, 1);
  rb_define_singleton_method(xlib_module, "query_tree", xlib_query_tree, 2);
  rb_define_singleton_method(xlib_module, "default_root_window", xlib_default_root_window, 1);
  rb_define_singleton_method(xlib_module, "intern_atom", xlib_intern_atom, 3);
  rb_define_singleton_method(xlib_module, "get_atom_name", xlib_get_atom_name, 2);
}
