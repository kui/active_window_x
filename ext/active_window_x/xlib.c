// -*- coding:utf-8; mode:c; -*-

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

#include <ruby.h>

#include <X11/Xlib.h>

#define GetDisplay(obj, d) {\
    Data_Get_Struct(obj, Display, d);  \
  }

VALUE xlib_module;
VALUE display_class;
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

#define ERROR_MESSAGE_BUFF 256
int error_handler(Display* d, XErrorEvent* error_event){
  char* name;
  char desc[ERROR_MESSAGE_BUFF];
  int length = ERROR_MESSAGE_BUFF;
  switch (error_event->error_code) {
  case BadRequest:  name = "BadRequest"; break;
  case BadValue:    name = "BadValue"; break;
  case BadWindow:   name = "BadWindow"; break;
  case BadPixmap:   name = "BadPixmap"; break;
  case BadAtom:     name = "BadAtom"; break;
  case BadCursor:   name = "BadCursor"; break;
  case BadFont:     name = "BadFont"; break;
  case BadMatch:    name = "BadMatch"; break;
  case BadDrawable: name = "BadDrawable"; break;
  case BadAccess:   name = "BadAccess"; break;
  case BadAlloc:    name = "BadAlloc"; break;
  case BadColor:    name = "BadColor"; break;
  case BadGC:       name = "BadGC"; break;
  case BadIDChoice: name = "BadIDChoice"; break;
  case BadName:     name = "BadName"; break;
  case BadLength:   name = "BadLength"; break;
  case BadImplementation:  name = "BadImplementation"; break;
  default: name = "UnknownErrorCode";
  }
  XGetErrorText(d, error_event->error_code, desc, length);
  rb_raise(x_error_event_class, "%s:%s", name, desc);
  return 1;
}

void Init_xlib(void){

  setlocale(LC_ALL, "");
  XSetErrorHandler(error_handler);

  xlib_module = rb_define_module("Xlib");
  display_class = rb_define_class_under(xlib_module, "Display", rb_cData);
  unknown_display_name_class =
    rb_define_class_under(xlib_module, "UnknownDisplayName", rb_eRuntimeError);
  x_error_event_class =
    rb_define_class_under(xlib_module, "XErrorEvent", rb_eRuntimeError);

  rb_define_singleton_method(xlib_module, "open_display", xlib_open_display, 1);
  rb_define_singleton_method(xlib_module, "close_display", xlib_close_display, 1);
}
