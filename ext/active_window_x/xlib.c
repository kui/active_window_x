// -*- coding:utf-8; mode:c; -*-

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

#include <ruby.h>

#include <X11/Xlib.h>

#define GetDisplay(obj, d) {                    \
    Data_Get_Struct(obj, Display, d);           \
  }
#define GetXEvent(obj, ev) {              \
    Data_Get_Struct(obj, XEvent, ev);     \
  }

VALUE xlib_module;
VALUE display_class;
VALUE x_event_class;
VALUE x_property_event_class;
VALUE unknown_display_name_class;
VALUE x_error_event_class;

// XOpenDisplay
VALUE xlib_x_open_display(VALUE self, VALUE name_obj) {
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
  return Data_Wrap_Struct(display_class, 0, 0, d);
}

// XCloseDisplay
VALUE xlib_x_close_display(VALUE self, VALUE display_obj) {
  Display* d;
  GetDisplay(display_obj, d);
  return INT2FIX((int) XCloseDisplay(d));
}

// XGetInputFocus
VALUE xlib_x_get_input_focus(VALUE self, VALUE display_obj) {
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
VALUE xlib_x_query_tree(VALUE self, VALUE d, VALUE w){
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
    rb_raise(rb_eRuntimeError, "XQueryTree fail");

  children_obj = rb_ary_new2(nchildren);
  for(i=0; i < nchildren; i++){
    rb_ary_push(children_obj, ULONG2NUM((unsigned long) children[i]));
  }

  arr_obj = rb_ary_new2(3L);
  rb_ary_push(arr_obj, root == None ? Qnil : ULONG2NUM((unsigned long) root));
  rb_ary_push(arr_obj, parent == None ? Qnil : ULONG2NUM((unsigned long) parent));
  rb_ary_push(arr_obj, children_obj);

  if(children != NULL) XFree(children);

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

// XInternAtom
static VALUE xlib_x_intern_atom(VALUE self, VALUE d, VALUE name_obj, VALUE b) {
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

// XGetAtomName
static VALUE xlib_x_get_atom_name(VALUE self, VALUE d, VALUE atom_obj){
  Display* display;
  Atom atom;
  char *name;
  VALUE r;

  GetDisplay(d, display);
  atom = NUM2ULONG(atom_obj);

  name = XGetAtomName(display, atom);

  if(name == NULL) {
    r = Qnil;
  } else {
    r = rb_str_new2(name);
    XFree(name);
  }
  return r;
}

// XGetWindowProperty
static VALUE xlib_x_get_window_property(VALUE self, VALUE display_obj, VALUE w_obj,
                                      VALUE property_obj, VALUE long_offset_obj,
                                      VALUE long_length_obj, VALUE delete_obj,
                                      VALUE req_type_obj){
  Display* display;
  Window w;
  Atom property;
  long long_offset, long_length;
  Bool delete;
  Atom req_type;
  Atom actual_type_return;
  int actual_format_return;
  unsigned long nitems_return;
  unsigned long bytes_after_return;
  unsigned char *prop_return;
  int result, prop_size, nbytes;
  VALUE ary, prop;

  GetDisplay(display_obj, display);
  w = (Window) NUM2ULONG(w_obj);
  property = (Atom) NUM2ULONG(property_obj);
  long_offset = NUM2LONG(long_offset_obj);
  long_length = NUM2LONG(long_length_obj);
  delete = (delete_obj == Qfalse || delete_obj == Qnil) ? False : True;
  req_type = (Atom) NUM2ULONG(req_type_obj);

  result = XGetWindowProperty(display, w, property, long_offset, long_length, delete,
                              req_type,
                              &actual_type_return, &actual_format_return,
                              &nitems_return, &bytes_after_return, &prop_return);
  if (result != Success)
    rb_raise(rb_eRuntimeError, "XGetWindowProperty faild");

  if (prop_return == NULL) {
    prop = Qnil;
  } else {
    // get nbytes (reffer to Get_Window_Property_Data_And_Type of xprop)
    switch (actual_format_return) {
    case 32 : nbytes = sizeof(long); break;
    case 16 : nbytes = 2; break;
    case 8 : nbytes = 1; break;
    case 0 : nbytes = 0; break;
    default: rb_raise(rb_eRuntimeError,
                      "Unexpected actual_format_return(%d) on XGetWindowProperty",
                      actual_format_return);
    }
    prop_size = nbytes * nitems_return;
    prop = rb_str_new(prop_return, prop_size);
    XFree(prop_return);
  }

  ary = rb_ary_new2(5L);
  rb_ary_push(ary, ULONG2NUM(actual_type_return));
  rb_ary_push(ary, INT2FIX(actual_format_return));
  rb_ary_push(ary, LONG2NUM(nitems_return));
  rb_ary_push(ary, LONG2NUM(bytes_after_return));
  rb_ary_push(ary, prop);

  return ary;
}

VALUE xlib_x_list_properties(VALUE self, VALUE display_obj, VALUE w_obj) {
  Display *display;
  Window w;
  int num_prop_return;
  Atom *result;
  int i;
  VALUE ary;

  GetDisplay(display_obj, display);
  w = (Window) NUM2ULONG(w_obj);

  result = XListProperties(display, w, &num_prop_return);
  if (result == NULL) {
    ary = Qnil;
  }else{
    ary = rb_ary_new2(num_prop_return);
    for(i=0; i<num_prop_return; i++){
      rb_ary_push(ary, result[i] == None ? Qnil : ULONG2NUM(result[i]));
    }
    XFree(result);
  }

  return ary;
}

VALUE xlib_x_select_input(VALUE self, VALUE display_obj, VALUE w_obj, VALUE event_mask_obj){
  Display *display;
  Window w;
  long event_mask;
  int result;

  GetDisplay(display_obj, display);
  w = (Window) NUM2ULONG(w_obj);
  event_mask = NUM2LONG(event_mask_obj);

  result = XSelectInput(display, w, event_mask);

  return INT2NUM(result);
}

VALUE x_event_new(XEvent *xevent) {
  VALUE ret;

  switch (xevent->type){
  case PropertyNotify:
    ret = Data_Wrap_Struct(x_property_event_class, 0, 0, xevent);
    // see `man XPropertyEvent`
    rb_iv_set(ret, "@type", INT2FIX(xevent->xproperty.type));
    rb_iv_set(ret, "@serial", ULONG2NUM(xevent->xproperty.serial));
    rb_iv_set(ret, "@send_event", INT2FIX(xevent->xproperty.send_event));
    rb_iv_set(ret, "@display", Data_Wrap_Struct(display_class, 0, 0, xevent->xproperty.display));
    rb_iv_set(ret, "@window", ULONG2NUM(xevent->xproperty.window));
    rb_iv_set(ret, "@atom", ULONG2NUM(xevent->xproperty.atom));
    rb_iv_set(ret, "@time", ULONG2NUM(xevent->xproperty.time));
    rb_iv_set(ret, "@state", INT2FIX(xevent->xproperty.state));
    break;
  default:
    ret = Data_Wrap_Struct(x_event_class, 0, 0, xevent);
    rb_iv_set(ret, "@type", INT2FIX(xevent->type));
  }

  return ret;
}

VALUE xlib_x_next_event(VALUE self, VALUE display_obj) {
  Display *display;
  XEvent *event_return;
  VALUE event_obj;

  GetDisplay(display_obj, display);
  event_return = ALLOC(XEvent);

  XNextEvent(display, event_return);

  return x_event_new(event_return);
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
  x_event_class = rb_define_class_under(xlib_module, "XEvent", rb_cData);
  rb_define_attr(x_event_class, "type", True, True);
  x_property_event_class =
    rb_define_class_under(xlib_module, "XPropertyEvent", x_event_class);
  rb_define_attr(x_property_event_class, "type", True, True);
  rb_define_attr(x_property_event_class, "serial", True, True);
  rb_define_attr(x_property_event_class, "send_event", True, True);
  rb_define_attr(x_property_event_class, "display", True, True);
  rb_define_attr(x_property_event_class, "window", True, True);
  rb_define_attr(x_property_event_class, "atom", True, True);
  rb_define_attr(x_property_event_class, "time", True, True);
  rb_define_attr(x_property_event_class, "state", True, True);
  unknown_display_name_class =
    rb_define_class_under(xlib_module, "UnknownDisplayName", rb_eRuntimeError);
  x_error_event_class =
    rb_define_class_under(xlib_module, "XErrorEvent", rb_eRuntimeError);

  rb_define_singleton_method(xlib_module, "x_open_display", xlib_x_open_display, 1);
  rb_define_singleton_method(xlib_module, "x_close_display", xlib_x_close_display, 1);
  rb_define_singleton_method(xlib_module, "x_get_input_focus", xlib_x_get_input_focus, 1);
  rb_define_singleton_method(xlib_module, "x_query_tree", xlib_x_query_tree, 2);
  rb_define_singleton_method(xlib_module, "default_root_window",
                             xlib_default_root_window, 1);
  rb_define_singleton_method(xlib_module, "x_intern_atom", xlib_x_intern_atom, 3);
  rb_define_singleton_method(xlib_module, "x_get_atom_name", xlib_x_get_atom_name, 2);
  rb_define_singleton_method(xlib_module, "x_get_window_property",
                             xlib_x_get_window_property, 7);
  rb_define_singleton_method(xlib_module, "x_list_properties", xlib_x_list_properties, 2);
  rb_define_singleton_method(xlib_module, "x_select_input", xlib_x_select_input, 3);
  rb_define_singleton_method(xlib_module, "x_next_event", xlib_x_next_event, 1);

  /*
    Constants on X.h
    the results of the following command
    ```
    grep '#define ' /usr/include/X11/X.h |\
    sed -e 's/^.define \([a-zA-Z]*\).*./rb_define_const(xlib_module, "\1",	INT2NUM(\1));/'|uniq
    ```
  */
#ifdef X
  rb_define_const(xlib_module, "X",	INT2NUM(X));
#endif
  rb_define_const(xlib_module, "None",	INT2NUM(None));
  rb_define_const(xlib_module, "ParentRelative",	INT2NUM(ParentRelative));
  rb_define_const(xlib_module, "CopyFromParent",	INT2NUM(CopyFromParent));
  rb_define_const(xlib_module, "PointerWindow",	INT2NUM(PointerWindow));
  rb_define_const(xlib_module, "InputFocus",	INT2NUM(InputFocus));
  rb_define_const(xlib_module, "PointerRoot",	INT2NUM(PointerRoot));
  rb_define_const(xlib_module, "AnyPropertyType",	INT2NUM(AnyPropertyType));
  rb_define_const(xlib_module, "AnyKey",	INT2NUM(AnyKey));
  rb_define_const(xlib_module, "AnyButton",	INT2NUM(AnyButton));
  rb_define_const(xlib_module, "AllTemporary",	INT2NUM(AllTemporary));
  rb_define_const(xlib_module, "CurrentTime",	INT2NUM(CurrentTime));
  rb_define_const(xlib_module, "NoSymbol",	INT2NUM(NoSymbol));
  rb_define_const(xlib_module, "NoEventMask",	INT2NUM(NoEventMask));
  rb_define_const(xlib_module, "KeyPressMask",	INT2NUM(KeyPressMask));
  rb_define_const(xlib_module, "KeyReleaseMask",	INT2NUM(KeyReleaseMask));
  rb_define_const(xlib_module, "ButtonPressMask",	INT2NUM(ButtonPressMask));
  rb_define_const(xlib_module, "ButtonReleaseMask",	INT2NUM(ButtonReleaseMask));
  rb_define_const(xlib_module, "EnterWindowMask",	INT2NUM(EnterWindowMask));
  rb_define_const(xlib_module, "LeaveWindowMask",	INT2NUM(LeaveWindowMask));
  rb_define_const(xlib_module, "PointerMotionMask",	INT2NUM(PointerMotionMask));
  rb_define_const(xlib_module, "PointerMotionHintMask",	INT2NUM(PointerMotionHintMask));
  rb_define_const(xlib_module, "ButtonMotionMask",	INT2NUM(ButtonMotionMask));
  rb_define_const(xlib_module, "KeymapStateMask",	INT2NUM(KeymapStateMask));
  rb_define_const(xlib_module, "ExposureMask",	INT2NUM(ExposureMask));
  rb_define_const(xlib_module, "VisibilityChangeMask",	INT2NUM(VisibilityChangeMask));
  rb_define_const(xlib_module, "StructureNotifyMask",	INT2NUM(StructureNotifyMask));
  rb_define_const(xlib_module, "ResizeRedirectMask",	INT2NUM(ResizeRedirectMask));
  rb_define_const(xlib_module, "SubstructureNotifyMask",	INT2NUM(SubstructureNotifyMask));
  rb_define_const(xlib_module, "SubstructureRedirectMask",	INT2NUM(SubstructureRedirectMask));
  rb_define_const(xlib_module, "FocusChangeMask",	INT2NUM(FocusChangeMask));
  rb_define_const(xlib_module, "PropertyChangeMask",	INT2NUM(PropertyChangeMask));
  rb_define_const(xlib_module, "ColormapChangeMask",	INT2NUM(ColormapChangeMask));
  rb_define_const(xlib_module, "OwnerGrabButtonMask",	INT2NUM(OwnerGrabButtonMask));
  rb_define_const(xlib_module, "KeyPress",	INT2NUM(KeyPress));
  rb_define_const(xlib_module, "KeyRelease",	INT2NUM(KeyRelease));
  rb_define_const(xlib_module, "ButtonPress",	INT2NUM(ButtonPress));
  rb_define_const(xlib_module, "ButtonRelease",	INT2NUM(ButtonRelease));
  rb_define_const(xlib_module, "MotionNotify",	INT2NUM(MotionNotify));
  rb_define_const(xlib_module, "EnterNotify",	INT2NUM(EnterNotify));
  rb_define_const(xlib_module, "LeaveNotify",	INT2NUM(LeaveNotify));
  rb_define_const(xlib_module, "FocusIn",	INT2NUM(FocusIn));
  rb_define_const(xlib_module, "FocusOut",	INT2NUM(FocusOut));
  rb_define_const(xlib_module, "KeymapNotify",	INT2NUM(KeymapNotify));
  rb_define_const(xlib_module, "Expose",	INT2NUM(Expose));
  rb_define_const(xlib_module, "GraphicsExpose",	INT2NUM(GraphicsExpose));
  rb_define_const(xlib_module, "NoExpose",	INT2NUM(NoExpose));
  rb_define_const(xlib_module, "VisibilityNotify",	INT2NUM(VisibilityNotify));
  rb_define_const(xlib_module, "CreateNotify",	INT2NUM(CreateNotify));
  rb_define_const(xlib_module, "DestroyNotify",	INT2NUM(DestroyNotify));
  rb_define_const(xlib_module, "UnmapNotify",	INT2NUM(UnmapNotify));
  rb_define_const(xlib_module, "MapNotify",	INT2NUM(MapNotify));
  rb_define_const(xlib_module, "MapRequest",	INT2NUM(MapRequest));
  rb_define_const(xlib_module, "ReparentNotify",	INT2NUM(ReparentNotify));
  rb_define_const(xlib_module, "ConfigureNotify",	INT2NUM(ConfigureNotify));
  rb_define_const(xlib_module, "ConfigureRequest",	INT2NUM(ConfigureRequest));
  rb_define_const(xlib_module, "GravityNotify",	INT2NUM(GravityNotify));
  rb_define_const(xlib_module, "ResizeRequest",	INT2NUM(ResizeRequest));
  rb_define_const(xlib_module, "CirculateNotify",	INT2NUM(CirculateNotify));
  rb_define_const(xlib_module, "CirculateRequest",	INT2NUM(CirculateRequest));
  rb_define_const(xlib_module, "PropertyNotify",	INT2NUM(PropertyNotify));
  rb_define_const(xlib_module, "SelectionClear",	INT2NUM(SelectionClear));
  rb_define_const(xlib_module, "SelectionRequest",	INT2NUM(SelectionRequest));
  rb_define_const(xlib_module, "SelectionNotify",	INT2NUM(SelectionNotify));
  rb_define_const(xlib_module, "ColormapNotify",	INT2NUM(ColormapNotify));
  rb_define_const(xlib_module, "ClientMessage",	INT2NUM(ClientMessage));
  rb_define_const(xlib_module, "MappingNotify",	INT2NUM(MappingNotify));
  rb_define_const(xlib_module, "GenericEvent",	INT2NUM(GenericEvent));
  rb_define_const(xlib_module, "LASTEvent",	INT2NUM(LASTEvent));
  rb_define_const(xlib_module, "ShiftMask",	INT2NUM(ShiftMask));
  rb_define_const(xlib_module, "LockMask",	INT2NUM(LockMask));
  rb_define_const(xlib_module, "ControlMask",	INT2NUM(ControlMask));
#ifdef Mod
  rb_define_const(xlib_module, "Mod",	INT2NUM(Mod));
#endif
  rb_define_const(xlib_module, "ShiftMapIndex",	INT2NUM(ShiftMapIndex));
  rb_define_const(xlib_module, "LockMapIndex",	INT2NUM(LockMapIndex));
  rb_define_const(xlib_module, "ControlMapIndex",	INT2NUM(ControlMapIndex));
#ifdef Button
  rb_define_const(xlib_module, "Button",	INT2NUM(Button));
#endif
  rb_define_const(xlib_module, "AnyModifier",	INT2NUM(AnyModifier));
  rb_define_const(xlib_module, "NotifyNormal",	INT2NUM(NotifyNormal));
  rb_define_const(xlib_module, "NotifyGrab",	INT2NUM(NotifyGrab));
  rb_define_const(xlib_module, "NotifyUngrab",	INT2NUM(NotifyUngrab));
  rb_define_const(xlib_module, "NotifyWhileGrabbed",	INT2NUM(NotifyWhileGrabbed));
  rb_define_const(xlib_module, "NotifyHint",	INT2NUM(NotifyHint));
  rb_define_const(xlib_module, "NotifyAncestor",	INT2NUM(NotifyAncestor));
  rb_define_const(xlib_module, "NotifyVirtual",	INT2NUM(NotifyVirtual));
  rb_define_const(xlib_module, "NotifyInferior",	INT2NUM(NotifyInferior));
  rb_define_const(xlib_module, "NotifyNonlinear",	INT2NUM(NotifyNonlinear));
  rb_define_const(xlib_module, "NotifyNonlinearVirtual",	INT2NUM(NotifyNonlinearVirtual));
  rb_define_const(xlib_module, "NotifyPointer",	INT2NUM(NotifyPointer));
  rb_define_const(xlib_module, "NotifyPointerRoot",	INT2NUM(NotifyPointerRoot));
  rb_define_const(xlib_module, "NotifyDetailNone",	INT2NUM(NotifyDetailNone));
  rb_define_const(xlib_module, "VisibilityUnobscured",	INT2NUM(VisibilityUnobscured));
  rb_define_const(xlib_module, "VisibilityPartiallyObscured",	INT2NUM(VisibilityPartiallyObscured));
  rb_define_const(xlib_module, "VisibilityFullyObscured",	INT2NUM(VisibilityFullyObscured));
  rb_define_const(xlib_module, "PlaceOnTop",	INT2NUM(PlaceOnTop));
  rb_define_const(xlib_module, "PlaceOnBottom",	INT2NUM(PlaceOnBottom));
  rb_define_const(xlib_module, "FamilyDECnet",	INT2NUM(FamilyDECnet));
  rb_define_const(xlib_module, "FamilyChaos",	INT2NUM(FamilyChaos));
  rb_define_const(xlib_module, "FamilyInternet",	INT2NUM(FamilyInternet));
  rb_define_const(xlib_module, "FamilyServerInterpreted",	INT2NUM(FamilyServerInterpreted));
  rb_define_const(xlib_module, "PropertyNewValue",	INT2NUM(PropertyNewValue));
  rb_define_const(xlib_module, "PropertyDelete",	INT2NUM(PropertyDelete));
  rb_define_const(xlib_module, "ColormapUninstalled",	INT2NUM(ColormapUninstalled));
  rb_define_const(xlib_module, "ColormapInstalled",	INT2NUM(ColormapInstalled));
  rb_define_const(xlib_module, "GrabModeSync",	INT2NUM(GrabModeSync));
  rb_define_const(xlib_module, "GrabModeAsync",	INT2NUM(GrabModeAsync));
  rb_define_const(xlib_module, "GrabSuccess",	INT2NUM(GrabSuccess));
  rb_define_const(xlib_module, "AlreadyGrabbed",	INT2NUM(AlreadyGrabbed));
  rb_define_const(xlib_module, "GrabInvalidTime",	INT2NUM(GrabInvalidTime));
  rb_define_const(xlib_module, "GrabNotViewable",	INT2NUM(GrabNotViewable));
  rb_define_const(xlib_module, "GrabFrozen",	INT2NUM(GrabFrozen));
  rb_define_const(xlib_module, "AsyncPointer",	INT2NUM(AsyncPointer));
  rb_define_const(xlib_module, "SyncPointer",	INT2NUM(SyncPointer));
  rb_define_const(xlib_module, "ReplayPointer",	INT2NUM(ReplayPointer));
  rb_define_const(xlib_module, "AsyncKeyboard",	INT2NUM(AsyncKeyboard));
  rb_define_const(xlib_module, "SyncKeyboard",	INT2NUM(SyncKeyboard));
  rb_define_const(xlib_module, "ReplayKeyboard",	INT2NUM(ReplayKeyboard));
  rb_define_const(xlib_module, "AsyncBoth",	INT2NUM(AsyncBoth));
  rb_define_const(xlib_module, "SyncBoth",	INT2NUM(SyncBoth));
  rb_define_const(xlib_module, "RevertToNone",	INT2NUM(RevertToNone));
  rb_define_const(xlib_module, "RevertToPointerRoot",	INT2NUM(RevertToPointerRoot));
  rb_define_const(xlib_module, "RevertToParent",	INT2NUM(RevertToParent));
  rb_define_const(xlib_module, "Success",	INT2NUM(Success));
  rb_define_const(xlib_module, "BadRequest",	INT2NUM(BadRequest));
  rb_define_const(xlib_module, "BadValue",	INT2NUM(BadValue));
  rb_define_const(xlib_module, "BadWindow",	INT2NUM(BadWindow));
  rb_define_const(xlib_module, "BadPixmap",	INT2NUM(BadPixmap));
  rb_define_const(xlib_module, "BadAtom",	INT2NUM(BadAtom));
  rb_define_const(xlib_module, "BadCursor",	INT2NUM(BadCursor));
  rb_define_const(xlib_module, "BadFont",	INT2NUM(BadFont));
  rb_define_const(xlib_module, "BadMatch",	INT2NUM(BadMatch));
  rb_define_const(xlib_module, "BadDrawable",	INT2NUM(BadDrawable));
  rb_define_const(xlib_module, "BadAccess",	INT2NUM(BadAccess));
  rb_define_const(xlib_module, "BadAlloc",	INT2NUM(BadAlloc));
  rb_define_const(xlib_module, "BadColor",	INT2NUM(BadColor));
  rb_define_const(xlib_module, "BadGC",	INT2NUM(BadGC));
  rb_define_const(xlib_module, "BadIDChoice",	INT2NUM(BadIDChoice));
  rb_define_const(xlib_module, "BadName",	INT2NUM(BadName));
  rb_define_const(xlib_module, "BadLength",	INT2NUM(BadLength));
  rb_define_const(xlib_module, "BadImplementation",	INT2NUM(BadImplementation));
  rb_define_const(xlib_module, "FirstExtensionError",	INT2NUM(FirstExtensionError));
  rb_define_const(xlib_module, "LastExtensionError",	INT2NUM(LastExtensionError));
  rb_define_const(xlib_module, "InputOutput",	INT2NUM(InputOutput));
  rb_define_const(xlib_module, "InputOnly",	INT2NUM(InputOnly));
  rb_define_const(xlib_module, "CWBackPixmap",	INT2NUM(CWBackPixmap));
  rb_define_const(xlib_module, "CWBackPixel",	INT2NUM(CWBackPixel));
  rb_define_const(xlib_module, "CWBorderPixmap",	INT2NUM(CWBorderPixmap));
  rb_define_const(xlib_module, "CWBorderPixel",	INT2NUM(CWBorderPixel));
  rb_define_const(xlib_module, "CWBitGravity",	INT2NUM(CWBitGravity));
  rb_define_const(xlib_module, "CWWinGravity",	INT2NUM(CWWinGravity));
  rb_define_const(xlib_module, "CWBackingStore",	INT2NUM(CWBackingStore));
  rb_define_const(xlib_module, "CWBackingPlanes",	INT2NUM(CWBackingPlanes));
  rb_define_const(xlib_module, "CWBackingPixel",	INT2NUM(CWBackingPixel));
  rb_define_const(xlib_module, "CWOverrideRedirect",	INT2NUM(CWOverrideRedirect));
  rb_define_const(xlib_module, "CWSaveUnder",	INT2NUM(CWSaveUnder));
  rb_define_const(xlib_module, "CWEventMask",	INT2NUM(CWEventMask));
  rb_define_const(xlib_module, "CWDontPropagate",	INT2NUM(CWDontPropagate));
  rb_define_const(xlib_module, "CWColormap",	INT2NUM(CWColormap));
  rb_define_const(xlib_module, "CWCursor",	INT2NUM(CWCursor));
  rb_define_const(xlib_module, "CWX",	INT2NUM(CWX));
  rb_define_const(xlib_module, "CWY",	INT2NUM(CWY));
  rb_define_const(xlib_module, "CWWidth",	INT2NUM(CWWidth));
  rb_define_const(xlib_module, "CWHeight",	INT2NUM(CWHeight));
  rb_define_const(xlib_module, "CWBorderWidth",	INT2NUM(CWBorderWidth));
  rb_define_const(xlib_module, "CWSibling",	INT2NUM(CWSibling));
  rb_define_const(xlib_module, "CWStackMode",	INT2NUM(CWStackMode));
  rb_define_const(xlib_module, "ForgetGravity",	INT2NUM(ForgetGravity));
  rb_define_const(xlib_module, "NorthWestGravity",	INT2NUM(NorthWestGravity));
  rb_define_const(xlib_module, "NorthGravity",	INT2NUM(NorthGravity));
  rb_define_const(xlib_module, "NorthEastGravity",	INT2NUM(NorthEastGravity));
  rb_define_const(xlib_module, "WestGravity",	INT2NUM(WestGravity));
  rb_define_const(xlib_module, "CenterGravity",	INT2NUM(CenterGravity));
  rb_define_const(xlib_module, "EastGravity",	INT2NUM(EastGravity));
  rb_define_const(xlib_module, "SouthWestGravity",	INT2NUM(SouthWestGravity));
  rb_define_const(xlib_module, "SouthGravity",	INT2NUM(SouthGravity));
  rb_define_const(xlib_module, "SouthEastGravity",	INT2NUM(SouthEastGravity));
  rb_define_const(xlib_module, "StaticGravity",	INT2NUM(StaticGravity));
  rb_define_const(xlib_module, "UnmapGravity",	INT2NUM(UnmapGravity));
  rb_define_const(xlib_module, "NotUseful",	INT2NUM(NotUseful));
  rb_define_const(xlib_module, "WhenMapped",	INT2NUM(WhenMapped));
  rb_define_const(xlib_module, "Always",	INT2NUM(Always));
  rb_define_const(xlib_module, "IsUnmapped",	INT2NUM(IsUnmapped));
  rb_define_const(xlib_module, "IsUnviewable",	INT2NUM(IsUnviewable));
  rb_define_const(xlib_module, "IsViewable",	INT2NUM(IsViewable));
  rb_define_const(xlib_module, "SetModeInsert",	INT2NUM(SetModeInsert));
  rb_define_const(xlib_module, "SetModeDelete",	INT2NUM(SetModeDelete));
  rb_define_const(xlib_module, "DestroyAll",	INT2NUM(DestroyAll));
  rb_define_const(xlib_module, "RetainPermanent",	INT2NUM(RetainPermanent));
  rb_define_const(xlib_module, "RetainTemporary",	INT2NUM(RetainTemporary));
  rb_define_const(xlib_module, "Above",	INT2NUM(Above));
  rb_define_const(xlib_module, "Below",	INT2NUM(Below));
  rb_define_const(xlib_module, "TopIf",	INT2NUM(TopIf));
  rb_define_const(xlib_module, "BottomIf",	INT2NUM(BottomIf));
  rb_define_const(xlib_module, "Opposite",	INT2NUM(Opposite));
  rb_define_const(xlib_module, "RaiseLowest",	INT2NUM(RaiseLowest));
  rb_define_const(xlib_module, "LowerHighest",	INT2NUM(LowerHighest));
  rb_define_const(xlib_module, "PropModeReplace",	INT2NUM(PropModeReplace));
  rb_define_const(xlib_module, "PropModePrepend",	INT2NUM(PropModePrepend));
  rb_define_const(xlib_module, "PropModeAppend",	INT2NUM(PropModeAppend));
  rb_define_const(xlib_module, "GXand",	INT2NUM(GXand));
  rb_define_const(xlib_module, "GXandReverse",	INT2NUM(GXandReverse));
  rb_define_const(xlib_module, "GXcopy",	INT2NUM(GXcopy));
  rb_define_const(xlib_module, "GXandInverted",	INT2NUM(GXandInverted));
  rb_define_const(xlib_module, "GXxor",	INT2NUM(GXxor));
  rb_define_const(xlib_module, "GXor",	INT2NUM(GXor));
  rb_define_const(xlib_module, "GXnor",	INT2NUM(GXnor));
  rb_define_const(xlib_module, "GXequiv",	INT2NUM(GXequiv));
  rb_define_const(xlib_module, "GXinvert",	INT2NUM(GXinvert));
  rb_define_const(xlib_module, "GXorReverse",	INT2NUM(GXorReverse));
  rb_define_const(xlib_module, "GXcopyInverted",	INT2NUM(GXcopyInverted));
  rb_define_const(xlib_module, "GXorInverted",	INT2NUM(GXorInverted));
  rb_define_const(xlib_module, "GXnand",	INT2NUM(GXnand));
  rb_define_const(xlib_module, "GXset",	INT2NUM(GXset));
  rb_define_const(xlib_module, "LineSolid",	INT2NUM(LineSolid));
  rb_define_const(xlib_module, "LineOnOffDash",	INT2NUM(LineOnOffDash));
  rb_define_const(xlib_module, "LineDoubleDash",	INT2NUM(LineDoubleDash));
  rb_define_const(xlib_module, "CapNotLast",	INT2NUM(CapNotLast));
  rb_define_const(xlib_module, "CapButt",	INT2NUM(CapButt));
  rb_define_const(xlib_module, "CapRound",	INT2NUM(CapRound));
  rb_define_const(xlib_module, "CapProjecting",	INT2NUM(CapProjecting));
  rb_define_const(xlib_module, "JoinMiter",	INT2NUM(JoinMiter));
  rb_define_const(xlib_module, "JoinRound",	INT2NUM(JoinRound));
  rb_define_const(xlib_module, "JoinBevel",	INT2NUM(JoinBevel));
  rb_define_const(xlib_module, "FillSolid",	INT2NUM(FillSolid));
  rb_define_const(xlib_module, "FillTiled",	INT2NUM(FillTiled));
  rb_define_const(xlib_module, "FillStippled",	INT2NUM(FillStippled));
  rb_define_const(xlib_module, "FillOpaqueStippled",	INT2NUM(FillOpaqueStippled));
  rb_define_const(xlib_module, "EvenOddRule",	INT2NUM(EvenOddRule));
  rb_define_const(xlib_module, "WindingRule",	INT2NUM(WindingRule));
  rb_define_const(xlib_module, "ClipByChildren",	INT2NUM(ClipByChildren));
  rb_define_const(xlib_module, "IncludeInferiors",	INT2NUM(IncludeInferiors));
  rb_define_const(xlib_module, "Unsorted",	INT2NUM(Unsorted));
  rb_define_const(xlib_module, "YSorted",	INT2NUM(YSorted));
  rb_define_const(xlib_module, "YXSorted",	INT2NUM(YXSorted));
  rb_define_const(xlib_module, "YXBanded",	INT2NUM(YXBanded));
  rb_define_const(xlib_module, "CoordModeOrigin",	INT2NUM(CoordModeOrigin));
  rb_define_const(xlib_module, "CoordModePrevious",	INT2NUM(CoordModePrevious));
  rb_define_const(xlib_module, "Complex",	INT2NUM(Complex));
  rb_define_const(xlib_module, "Nonconvex",	INT2NUM(Nonconvex));
  rb_define_const(xlib_module, "Convex",	INT2NUM(Convex));
  rb_define_const(xlib_module, "ArcChord",	INT2NUM(ArcChord));
  rb_define_const(xlib_module, "ArcPieSlice",	INT2NUM(ArcPieSlice));
  rb_define_const(xlib_module, "GCFunction",	INT2NUM(GCFunction));
  rb_define_const(xlib_module, "GCPlaneMask",	INT2NUM(GCPlaneMask));
  rb_define_const(xlib_module, "GCForeground",	INT2NUM(GCForeground));
  rb_define_const(xlib_module, "GCBackground",	INT2NUM(GCBackground));
  rb_define_const(xlib_module, "GCLineWidth",	INT2NUM(GCLineWidth));
  rb_define_const(xlib_module, "GCLineStyle",	INT2NUM(GCLineStyle));
  rb_define_const(xlib_module, "GCCapStyle",	INT2NUM(GCCapStyle));
  rb_define_const(xlib_module, "GCJoinStyle",	INT2NUM(GCJoinStyle));
  rb_define_const(xlib_module, "GCFillStyle",	INT2NUM(GCFillStyle));
  rb_define_const(xlib_module, "GCFillRule",	INT2NUM(GCFillRule));
  rb_define_const(xlib_module, "GCTile",	INT2NUM(GCTile));
  rb_define_const(xlib_module, "GCStipple",	INT2NUM(GCStipple));
  rb_define_const(xlib_module, "GCTileStipXOrigin",	INT2NUM(GCTileStipXOrigin));
  rb_define_const(xlib_module, "GCTileStipYOrigin",	INT2NUM(GCTileStipYOrigin));
  rb_define_const(xlib_module, "GCFont",	INT2NUM(GCFont));
  rb_define_const(xlib_module, "GCSubwindowMode",	INT2NUM(GCSubwindowMode));
  rb_define_const(xlib_module, "GCGraphicsExposures",	INT2NUM(GCGraphicsExposures));
  rb_define_const(xlib_module, "GCClipXOrigin",	INT2NUM(GCClipXOrigin));
  rb_define_const(xlib_module, "GCClipYOrigin",	INT2NUM(GCClipYOrigin));
  rb_define_const(xlib_module, "GCClipMask",	INT2NUM(GCClipMask));
  rb_define_const(xlib_module, "GCDashOffset",	INT2NUM(GCDashOffset));
  rb_define_const(xlib_module, "GCDashList",	INT2NUM(GCDashList));
  rb_define_const(xlib_module, "GCArcMode",	INT2NUM(GCArcMode));
  rb_define_const(xlib_module, "GCLastBit",	INT2NUM(GCLastBit));
  rb_define_const(xlib_module, "FontLeftToRight",	INT2NUM(FontLeftToRight));
  rb_define_const(xlib_module, "FontRightToLeft",	INT2NUM(FontRightToLeft));
  rb_define_const(xlib_module, "FontChange",	INT2NUM(FontChange));
  rb_define_const(xlib_module, "XYBitmap",	INT2NUM(XYBitmap));
  rb_define_const(xlib_module, "XYPixmap",	INT2NUM(XYPixmap));
  rb_define_const(xlib_module, "ZPixmap",	INT2NUM(ZPixmap));
  rb_define_const(xlib_module, "AllocNone",	INT2NUM(AllocNone));
  rb_define_const(xlib_module, "AllocAll",	INT2NUM(AllocAll));
  rb_define_const(xlib_module, "DoRed",	INT2NUM(DoRed));
  rb_define_const(xlib_module, "DoGreen",	INT2NUM(DoGreen));
  rb_define_const(xlib_module, "DoBlue",	INT2NUM(DoBlue));
  rb_define_const(xlib_module, "CursorShape",	INT2NUM(CursorShape));
  rb_define_const(xlib_module, "TileShape",	INT2NUM(TileShape));
  rb_define_const(xlib_module, "StippleShape",	INT2NUM(StippleShape));
  rb_define_const(xlib_module, "AutoRepeatModeOff",	INT2NUM(AutoRepeatModeOff));
  rb_define_const(xlib_module, "AutoRepeatModeOn",	INT2NUM(AutoRepeatModeOn));
  rb_define_const(xlib_module, "AutoRepeatModeDefault",	INT2NUM(AutoRepeatModeDefault));
  rb_define_const(xlib_module, "LedModeOff",	INT2NUM(LedModeOff));
  rb_define_const(xlib_module, "LedModeOn",	INT2NUM(LedModeOn));
  rb_define_const(xlib_module, "KBKeyClickPercent",	INT2NUM(KBKeyClickPercent));
  rb_define_const(xlib_module, "KBBellPercent",	INT2NUM(KBBellPercent));
  rb_define_const(xlib_module, "KBBellPitch",	INT2NUM(KBBellPitch));
  rb_define_const(xlib_module, "KBBellDuration",	INT2NUM(KBBellDuration));
  rb_define_const(xlib_module, "KBLed",	INT2NUM(KBLed));
  rb_define_const(xlib_module, "KBLedMode",	INT2NUM(KBLedMode));
  rb_define_const(xlib_module, "KBKey",	INT2NUM(KBKey));
  rb_define_const(xlib_module, "KBAutoRepeatMode",	INT2NUM(KBAutoRepeatMode));
  rb_define_const(xlib_module, "MappingSuccess",	INT2NUM(MappingSuccess));
  rb_define_const(xlib_module, "MappingBusy",	INT2NUM(MappingBusy));
  rb_define_const(xlib_module, "MappingFailed",	INT2NUM(MappingFailed));
  rb_define_const(xlib_module, "MappingModifier",	INT2NUM(MappingModifier));
  rb_define_const(xlib_module, "MappingKeyboard",	INT2NUM(MappingKeyboard));
  rb_define_const(xlib_module, "MappingPointer",	INT2NUM(MappingPointer));
  rb_define_const(xlib_module, "DontPreferBlanking",	INT2NUM(DontPreferBlanking));
  rb_define_const(xlib_module, "PreferBlanking",	INT2NUM(PreferBlanking));
  rb_define_const(xlib_module, "DefaultBlanking",	INT2NUM(DefaultBlanking));
  rb_define_const(xlib_module, "DisableScreenSaver",	INT2NUM(DisableScreenSaver));
  rb_define_const(xlib_module, "DisableScreenInterval",	INT2NUM(DisableScreenInterval));
  rb_define_const(xlib_module, "DontAllowExposures",	INT2NUM(DontAllowExposures));
  rb_define_const(xlib_module, "AllowExposures",	INT2NUM(AllowExposures));
  rb_define_const(xlib_module, "DefaultExposures",	INT2NUM(DefaultExposures));
  rb_define_const(xlib_module, "ScreenSaverReset",	INT2NUM(ScreenSaverReset));
  rb_define_const(xlib_module, "ScreenSaverActive",	INT2NUM(ScreenSaverActive));
  rb_define_const(xlib_module, "HostInsert",	INT2NUM(HostInsert));
  rb_define_const(xlib_module, "HostDelete",	INT2NUM(HostDelete));
  rb_define_const(xlib_module, "EnableAccess",	INT2NUM(EnableAccess));
  rb_define_const(xlib_module, "DisableAccess",	INT2NUM(DisableAccess));
  rb_define_const(xlib_module, "StaticGray",	INT2NUM(StaticGray));
  rb_define_const(xlib_module, "GrayScale",	INT2NUM(GrayScale));
  rb_define_const(xlib_module, "StaticColor",	INT2NUM(StaticColor));
  rb_define_const(xlib_module, "PseudoColor",	INT2NUM(PseudoColor));
  rb_define_const(xlib_module, "TrueColor",	INT2NUM(TrueColor));
  rb_define_const(xlib_module, "DirectColor",	INT2NUM(DirectColor));
  rb_define_const(xlib_module, "LSBFirst",	INT2NUM(LSBFirst));
  rb_define_const(xlib_module, "MSBFirst",	INT2NUM(MSBFirst));
}
