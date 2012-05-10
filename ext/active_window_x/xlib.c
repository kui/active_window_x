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
  return Data_Wrap_Struct(display_class, 0, -1, d);
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

  // if(children != NULL) XFree(children);

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

// XGetAtomName
static VALUE xlib_get_atom_name(VALUE self, VALUE d, VALUE atom_obj){
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
    // XFree(name);
  }
  return r;
}

// XGetWindowProperty
static VALUE xlib_get_window_property(VALUE self, VALUE display_obj, VALUE w_obj,
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
  int result;
  VALUE ary;

  GetDisplay(display_obj, display);
  w = (Window) NUM2ULONG(w_obj);
  property = (Atom) NUM2ULONG(property_obj);
  long_offset = NUM2LONG(long_offset_obj);
  long_length = NUM2LONG(long_length_obj);
  delete = (delete_obj == Qfalse || delete_obj == Qnil) ? False : True;
  req_type = (Atom) NUM2ULONG(property_obj);

  result = XGetWindowProperty(display, w, property, long_offset, long_length, delete,
                              req_type,
                              &actual_type_return, &actual_format_return,
                              &nitems_return, &bytes_after_return, &prop_return) ;
  if(result != Success)
    rb_raise(rb_eRuntimeError, "fail on XGetWindowProperty");

  ary = rb_ary_new2(5L);
  rb_ary_push(ary,
              actual_type_return == None ?
              Qnil :
              ULONG2NUM((unsigned long) actual_type_return));
  rb_ary_push(ary, INT2FIX(actual_format_return));
  rb_ary_push(ary, LONG2NUM(nitems_return));
  rb_ary_push(ary, LONG2NUM(bytes_after_return));
  rb_ary_push(ary, prop_return == NULL ? Qnil : rb_str_new2(prop_return));

  return ary;
}

VALUE xlib_list_properties(VALUE self, VALUE display_obj, VALUE w_obj) {
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
    for(i=0; i<num_prop_return; i++)
      rb_ary_push(ary, result[i] == None ? Qnil : ULONG2NUM(result[i]));
    // XFree(result);
  }

  return ary;
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
  rb_define_singleton_method(xlib_module, "default_root_window",
                             xlib_default_root_window, 1);
  rb_define_singleton_method(xlib_module, "intern_atom", xlib_intern_atom, 3);
  rb_define_singleton_method(xlib_module, "get_atom_name", xlib_get_atom_name, 2);
  rb_define_singleton_method(xlib_module, "get_window_property",
                             xlib_get_window_property, 7);
  rb_define_singleton_method(xlib_module, "list_properties", xlib_list_properties, 2);

  /*
    Constants on X.h
    the results of the following command
    grep '#define ' /usr/include/X11/X.h  |\
    sed -e 's/^.define \([a-zA-Z]*\).*./rb_define_const(xlib_module, "\1",       \1);/'|uniq
  */
#ifdef X
  rb_define_const(xlib_module, "X",        X);
#endif
  rb_define_const(xlib_module, "None",     None);
  rb_define_const(xlib_module, "ParentRelative",   ParentRelative);
  rb_define_const(xlib_module, "CopyFromParent",   CopyFromParent);
  rb_define_const(xlib_module, "PointerWindow",    PointerWindow);
  rb_define_const(xlib_module, "InputFocus",       InputFocus);
  rb_define_const(xlib_module, "PointerRoot",      PointerRoot);
  rb_define_const(xlib_module, "AnyPropertyType",  AnyPropertyType);
  rb_define_const(xlib_module, "AnyKey",   AnyKey);
  rb_define_const(xlib_module, "AnyButton",        AnyButton);
  rb_define_const(xlib_module, "AllTemporary",     AllTemporary);
  rb_define_const(xlib_module, "CurrentTime",      CurrentTime);
  rb_define_const(xlib_module, "NoSymbol",         NoSymbol);
  rb_define_const(xlib_module, "NoEventMask",      NoEventMask);
  rb_define_const(xlib_module, "KeyPressMask",     KeyPressMask);
  rb_define_const(xlib_module, "KeyReleaseMask",   KeyReleaseMask);
  rb_define_const(xlib_module, "ButtonPressMask",  ButtonPressMask);
  rb_define_const(xlib_module, "ButtonReleaseMask",        ButtonReleaseMask);
  rb_define_const(xlib_module, "EnterWindowMask",  EnterWindowMask);
  rb_define_const(xlib_module, "LeaveWindowMask",  LeaveWindowMask);
  rb_define_const(xlib_module, "PointerMotionMask",        PointerMotionMask);
  rb_define_const(xlib_module, "PointerMotionHintMask",    PointerMotionHintMask);
#ifdef Button
  rb_define_const(xlib_module, "Button",   Button);
#endif
  rb_define_const(xlib_module, "ButtonMotionMask",         ButtonMotionMask);
  rb_define_const(xlib_module, "KeymapStateMask",  KeymapStateMask);
  rb_define_const(xlib_module, "ExposureMask",     ExposureMask);
  rb_define_const(xlib_module, "VisibilityChangeMask",     VisibilityChangeMask);
  rb_define_const(xlib_module, "StructureNotifyMask",      StructureNotifyMask);
  rb_define_const(xlib_module, "ResizeRedirectMask",       ResizeRedirectMask);
  rb_define_const(xlib_module, "SubstructureNotifyMask",   SubstructureNotifyMask);
  rb_define_const(xlib_module, "SubstructureRedirectMask",         SubstructureRedirectMask);
  rb_define_const(xlib_module, "FocusChangeMask",  FocusChangeMask);
  rb_define_const(xlib_module, "PropertyChangeMask",       PropertyChangeMask);
  rb_define_const(xlib_module, "ColormapChangeMask",       ColormapChangeMask);
  rb_define_const(xlib_module, "OwnerGrabButtonMask",      OwnerGrabButtonMask);
  rb_define_const(xlib_module, "KeyPress",         KeyPress);
  rb_define_const(xlib_module, "KeyRelease",       KeyRelease);
  rb_define_const(xlib_module, "ButtonPress",      ButtonPress);
  rb_define_const(xlib_module, "ButtonRelease",    ButtonRelease);
  rb_define_const(xlib_module, "MotionNotify",     MotionNotify);
  rb_define_const(xlib_module, "EnterNotify",      EnterNotify);
  rb_define_const(xlib_module, "LeaveNotify",      LeaveNotify);
  rb_define_const(xlib_module, "FocusIn",  FocusIn);
  rb_define_const(xlib_module, "FocusOut",         FocusOut);
  rb_define_const(xlib_module, "KeymapNotify",     KeymapNotify);
  rb_define_const(xlib_module, "Expose",   Expose);
  rb_define_const(xlib_module, "GraphicsExpose",   GraphicsExpose);
  rb_define_const(xlib_module, "NoExpose",         NoExpose);
  rb_define_const(xlib_module, "VisibilityNotify",         VisibilityNotify);
  rb_define_const(xlib_module, "CreateNotify",     CreateNotify);
  rb_define_const(xlib_module, "DestroyNotify",    DestroyNotify);
  rb_define_const(xlib_module, "UnmapNotify",      UnmapNotify);
  rb_define_const(xlib_module, "MapNotify",        MapNotify);
  rb_define_const(xlib_module, "MapRequest",       MapRequest);
  rb_define_const(xlib_module, "ReparentNotify",   ReparentNotify);
  rb_define_const(xlib_module, "ConfigureNotify",  ConfigureNotify);
  rb_define_const(xlib_module, "ConfigureRequest",         ConfigureRequest);
  rb_define_const(xlib_module, "GravityNotify",    GravityNotify);
  rb_define_const(xlib_module, "ResizeRequest",    ResizeRequest);
  rb_define_const(xlib_module, "CirculateNotify",  CirculateNotify);
  rb_define_const(xlib_module, "CirculateRequest",         CirculateRequest);
  rb_define_const(xlib_module, "PropertyNotify",   PropertyNotify);
  rb_define_const(xlib_module, "SelectionClear",   SelectionClear);
  rb_define_const(xlib_module, "SelectionRequest",         SelectionRequest);
  rb_define_const(xlib_module, "SelectionNotify",  SelectionNotify);
  rb_define_const(xlib_module, "ColormapNotify",   ColormapNotify);
  rb_define_const(xlib_module, "ClientMessage",    ClientMessage);
  rb_define_const(xlib_module, "MappingNotify",    MappingNotify);
  rb_define_const(xlib_module, "GenericEvent",     GenericEvent);
  rb_define_const(xlib_module, "LASTEvent",        LASTEvent);
  rb_define_const(xlib_module, "ShiftMask",        ShiftMask);
  rb_define_const(xlib_module, "LockMask",         LockMask);
  rb_define_const(xlib_module, "ControlMask",      ControlMask);
#ifdef Mod
  rb_define_const(xlib_module, "Mod",      Mod);
#endif
  rb_define_const(xlib_module, "ShiftMapIndex",    ShiftMapIndex);
  rb_define_const(xlib_module, "LockMapIndex",     LockMapIndex);
  rb_define_const(xlib_module, "ControlMapIndex",  ControlMapIndex);
  rb_define_const(xlib_module, "AnyModifier",      AnyModifier);
  rb_define_const(xlib_module, "NotifyNormal",     NotifyNormal);
  rb_define_const(xlib_module, "NotifyGrab",       NotifyGrab);
  rb_define_const(xlib_module, "NotifyUngrab",     NotifyUngrab);
  rb_define_const(xlib_module, "NotifyWhileGrabbed",       NotifyWhileGrabbed);
  rb_define_const(xlib_module, "NotifyHint",       NotifyHint);
  rb_define_const(xlib_module, "NotifyAncestor",   NotifyAncestor);
  rb_define_const(xlib_module, "NotifyVirtual",    NotifyVirtual);
  rb_define_const(xlib_module, "NotifyInferior",   NotifyInferior);
  rb_define_const(xlib_module, "NotifyNonlinear",  NotifyNonlinear);
  rb_define_const(xlib_module, "NotifyNonlinearVirtual",   NotifyNonlinearVirtual);
  rb_define_const(xlib_module, "NotifyPointer",    NotifyPointer);
  rb_define_const(xlib_module, "NotifyPointerRoot",        NotifyPointerRoot);
  rb_define_const(xlib_module, "NotifyDetailNone",         NotifyDetailNone);
  rb_define_const(xlib_module, "VisibilityUnobscured",     VisibilityUnobscured);
  rb_define_const(xlib_module, "VisibilityPartiallyObscured",      VisibilityPartiallyObscured);
  rb_define_const(xlib_module, "VisibilityFullyObscured",  VisibilityFullyObscured);
  rb_define_const(xlib_module, "PlaceOnTop",       PlaceOnTop);
  rb_define_const(xlib_module, "PlaceOnBottom",    PlaceOnBottom);
  rb_define_const(xlib_module, "FamilyInternet",   FamilyInternet);
  rb_define_const(xlib_module, "FamilyDECnet",     FamilyDECnet);
  rb_define_const(xlib_module, "FamilyChaos",      FamilyChaos);
  rb_define_const(xlib_module, "FamilyServerInterpreted",  FamilyServerInterpreted);
  rb_define_const(xlib_module, "PropertyNewValue",         PropertyNewValue);
  rb_define_const(xlib_module, "PropertyDelete",   PropertyDelete);
  rb_define_const(xlib_module, "ColormapUninstalled",      ColormapUninstalled);
  rb_define_const(xlib_module, "ColormapInstalled",        ColormapInstalled);
  rb_define_const(xlib_module, "GrabModeSync",     GrabModeSync);
  rb_define_const(xlib_module, "GrabModeAsync",    GrabModeAsync);
  rb_define_const(xlib_module, "GrabSuccess",      GrabSuccess);
  rb_define_const(xlib_module, "AlreadyGrabbed",   AlreadyGrabbed);
  rb_define_const(xlib_module, "GrabInvalidTime",  GrabInvalidTime);
  rb_define_const(xlib_module, "GrabNotViewable",  GrabNotViewable);
  rb_define_const(xlib_module, "GrabFrozen",       GrabFrozen);
  rb_define_const(xlib_module, "AsyncPointer",     AsyncPointer);
  rb_define_const(xlib_module, "SyncPointer",      SyncPointer);
  rb_define_const(xlib_module, "ReplayPointer",    ReplayPointer);
  rb_define_const(xlib_module, "AsyncKeyboard",    AsyncKeyboard);
  rb_define_const(xlib_module, "SyncKeyboard",     SyncKeyboard);
  rb_define_const(xlib_module, "ReplayKeyboard",   ReplayKeyboard);
  rb_define_const(xlib_module, "AsyncBoth",        AsyncBoth);
  rb_define_const(xlib_module, "SyncBoth",         SyncBoth);
  rb_define_const(xlib_module, "RevertToNone",     RevertToNone);
  rb_define_const(xlib_module, "RevertToPointerRoot",      RevertToPointerRoot);
  rb_define_const(xlib_module, "RevertToParent",   RevertToParent);
  rb_define_const(xlib_module, "Success",  Success);
  rb_define_const(xlib_module, "BadRequest",       BadRequest);
  rb_define_const(xlib_module, "BadValue",         BadValue);
  rb_define_const(xlib_module, "BadWindow",        BadWindow);
  rb_define_const(xlib_module, "BadPixmap",        BadPixmap);
  rb_define_const(xlib_module, "BadAtom",  BadAtom);
  rb_define_const(xlib_module, "BadCursor",        BadCursor);
  rb_define_const(xlib_module, "BadFont",  BadFont);
  rb_define_const(xlib_module, "BadMatch",         BadMatch);
  rb_define_const(xlib_module, "BadDrawable",      BadDrawable);
  rb_define_const(xlib_module, "BadAccess",        BadAccess);
  rb_define_const(xlib_module, "BadAlloc",         BadAlloc);
  rb_define_const(xlib_module, "BadColor",         BadColor);
  rb_define_const(xlib_module, "BadGC",    BadGC);
  rb_define_const(xlib_module, "BadIDChoice",      BadIDChoice);
  rb_define_const(xlib_module, "BadName",  BadName);
  rb_define_const(xlib_module, "BadLength",        BadLength);
  rb_define_const(xlib_module, "BadImplementation",        BadImplementation);
  rb_define_const(xlib_module, "FirstExtensionError",      FirstExtensionError);
  rb_define_const(xlib_module, "LastExtensionError",       LastExtensionError);
  rb_define_const(xlib_module, "InputOutput",      InputOutput);
  rb_define_const(xlib_module, "InputOnly",        InputOnly);
  rb_define_const(xlib_module, "CWBackPixmap",     CWBackPixmap);
  rb_define_const(xlib_module, "CWBackPixel",      CWBackPixel);
  rb_define_const(xlib_module, "CWBorderPixmap",   CWBorderPixmap);
  rb_define_const(xlib_module, "CWBorderPixel",    CWBorderPixel);
  rb_define_const(xlib_module, "CWBitGravity",     CWBitGravity);
  rb_define_const(xlib_module, "CWWinGravity",     CWWinGravity);
  rb_define_const(xlib_module, "CWBackingStore",   CWBackingStore);
  rb_define_const(xlib_module, "CWBackingPlanes",  CWBackingPlanes);
  rb_define_const(xlib_module, "CWBackingPixel",   CWBackingPixel);
  rb_define_const(xlib_module, "CWOverrideRedirect",       CWOverrideRedirect);
  rb_define_const(xlib_module, "CWSaveUnder",      CWSaveUnder);
  rb_define_const(xlib_module, "CWEventMask",      CWEventMask);
  rb_define_const(xlib_module, "CWDontPropagate",  CWDontPropagate);
  rb_define_const(xlib_module, "CWColormap",       CWColormap);
  rb_define_const(xlib_module, "CWCursor",         CWCursor);
  rb_define_const(xlib_module, "CWX",      CWX);
  rb_define_const(xlib_module, "CWY",      CWY);
  rb_define_const(xlib_module, "CWWidth",  CWWidth);
  rb_define_const(xlib_module, "CWHeight",         CWHeight);
  rb_define_const(xlib_module, "CWBorderWidth",    CWBorderWidth);
  rb_define_const(xlib_module, "CWSibling",        CWSibling);
  rb_define_const(xlib_module, "CWStackMode",      CWStackMode);
  rb_define_const(xlib_module, "ForgetGravity",    ForgetGravity);
  rb_define_const(xlib_module, "NorthWestGravity",         NorthWestGravity);
  rb_define_const(xlib_module, "NorthGravity",     NorthGravity);
  rb_define_const(xlib_module, "NorthEastGravity",         NorthEastGravity);
  rb_define_const(xlib_module, "WestGravity",      WestGravity);
  rb_define_const(xlib_module, "CenterGravity",    CenterGravity);
  rb_define_const(xlib_module, "EastGravity",      EastGravity);
  rb_define_const(xlib_module, "SouthWestGravity",         SouthWestGravity);
  rb_define_const(xlib_module, "SouthGravity",     SouthGravity);
  rb_define_const(xlib_module, "SouthEastGravity",         SouthEastGravity);
  rb_define_const(xlib_module, "StaticGravity",    StaticGravity);
  rb_define_const(xlib_module, "UnmapGravity",     UnmapGravity);
  rb_define_const(xlib_module, "NotUseful",        NotUseful);
  rb_define_const(xlib_module, "WhenMapped",       WhenMapped);
  rb_define_const(xlib_module, "Always",   Always);
  rb_define_const(xlib_module, "IsUnmapped",       IsUnmapped);
  rb_define_const(xlib_module, "IsUnviewable",     IsUnviewable);
  rb_define_const(xlib_module, "IsViewable",       IsViewable);
  rb_define_const(xlib_module, "SetModeInsert",    SetModeInsert);
  rb_define_const(xlib_module, "SetModeDelete",    SetModeDelete);
  rb_define_const(xlib_module, "DestroyAll",       DestroyAll);
  rb_define_const(xlib_module, "RetainPermanent",  RetainPermanent);
  rb_define_const(xlib_module, "RetainTemporary",  RetainTemporary);
  rb_define_const(xlib_module, "Above",    Above);
  rb_define_const(xlib_module, "Below",    Below);
  rb_define_const(xlib_module, "TopIf",    TopIf);
  rb_define_const(xlib_module, "BottomIf",         BottomIf);
  rb_define_const(xlib_module, "Opposite",         Opposite);
  rb_define_const(xlib_module, "RaiseLowest",      RaiseLowest);
  rb_define_const(xlib_module, "LowerHighest",     LowerHighest);
  rb_define_const(xlib_module, "PropModeReplace",  PropModeReplace);
  rb_define_const(xlib_module, "PropModePrepend",  PropModePrepend);
  rb_define_const(xlib_module, "PropModeAppend",   PropModeAppend);
  rb_define_const(xlib_module, "GXand",    GXand);
  rb_define_const(xlib_module, "GXandReverse",     GXandReverse);
  rb_define_const(xlib_module, "GXcopy",   GXcopy);
  rb_define_const(xlib_module, "GXandInverted",    GXandInverted);
  rb_define_const(xlib_module, "GXxor",    GXxor);
  rb_define_const(xlib_module, "GXor",     GXor);
  rb_define_const(xlib_module, "GXnor",    GXnor);
  rb_define_const(xlib_module, "GXequiv",  GXequiv);
  rb_define_const(xlib_module, "GXinvert",         GXinvert);
  rb_define_const(xlib_module, "GXorReverse",      GXorReverse);
  rb_define_const(xlib_module, "GXcopyInverted",   GXcopyInverted);
  rb_define_const(xlib_module, "GXorInverted",     GXorInverted);
  rb_define_const(xlib_module, "GXnand",   GXnand);
  rb_define_const(xlib_module, "GXset",    GXset);
  rb_define_const(xlib_module, "LineSolid",        LineSolid);
  rb_define_const(xlib_module, "LineOnOffDash",    LineOnOffDash);
  rb_define_const(xlib_module, "LineDoubleDash",   LineDoubleDash);
  rb_define_const(xlib_module, "CapNotLast",       CapNotLast);
  rb_define_const(xlib_module, "CapButt",  CapButt);
  rb_define_const(xlib_module, "CapRound",         CapRound);
  rb_define_const(xlib_module, "CapProjecting",    CapProjecting);
  rb_define_const(xlib_module, "JoinMiter",        JoinMiter);
  rb_define_const(xlib_module, "JoinRound",        JoinRound);
  rb_define_const(xlib_module, "JoinBevel",        JoinBevel);
  rb_define_const(xlib_module, "FillSolid",        FillSolid);
  rb_define_const(xlib_module, "FillTiled",        FillTiled);
  rb_define_const(xlib_module, "FillStippled",     FillStippled);
  rb_define_const(xlib_module, "FillOpaqueStippled",       FillOpaqueStippled);
  rb_define_const(xlib_module, "EvenOddRule",      EvenOddRule);
  rb_define_const(xlib_module, "WindingRule",      WindingRule);
  rb_define_const(xlib_module, "ClipByChildren",   ClipByChildren);
  rb_define_const(xlib_module, "IncludeInferiors",         IncludeInferiors);
  rb_define_const(xlib_module, "Unsorted",         Unsorted);
  rb_define_const(xlib_module, "YSorted",  YSorted);
  rb_define_const(xlib_module, "YXSorted",         YXSorted);
  rb_define_const(xlib_module, "YXBanded",         YXBanded);
  rb_define_const(xlib_module, "CoordModeOrigin",  CoordModeOrigin);
  rb_define_const(xlib_module, "CoordModePrevious",        CoordModePrevious);
  rb_define_const(xlib_module, "Complex",  Complex);
  rb_define_const(xlib_module, "Nonconvex",        Nonconvex);
  rb_define_const(xlib_module, "Convex",   Convex);
  rb_define_const(xlib_module, "ArcChord",         ArcChord);
  rb_define_const(xlib_module, "ArcPieSlice",      ArcPieSlice);
  rb_define_const(xlib_module, "GCFunction",       GCFunction);
  rb_define_const(xlib_module, "GCPlaneMask",      GCPlaneMask);
  rb_define_const(xlib_module, "GCForeground",     GCForeground);
  rb_define_const(xlib_module, "GCBackground",     GCBackground);
  rb_define_const(xlib_module, "GCLineWidth",      GCLineWidth);
  rb_define_const(xlib_module, "GCLineStyle",      GCLineStyle);
  rb_define_const(xlib_module, "GCCapStyle",       GCCapStyle);
  rb_define_const(xlib_module, "GCJoinStyle",      GCJoinStyle);
  rb_define_const(xlib_module, "GCFillStyle",      GCFillStyle);
  rb_define_const(xlib_module, "GCFillRule",       GCFillRule);
  rb_define_const(xlib_module, "GCTile",   GCTile);
  rb_define_const(xlib_module, "GCStipple",        GCStipple);
  rb_define_const(xlib_module, "GCTileStipXOrigin",        GCTileStipXOrigin);
  rb_define_const(xlib_module, "GCTileStipYOrigin",        GCTileStipYOrigin);
  rb_define_const(xlib_module, "GCFont",   GCFont);
  rb_define_const(xlib_module, "GCSubwindowMode",  GCSubwindowMode);
  rb_define_const(xlib_module, "GCGraphicsExposures",      GCGraphicsExposures);
  rb_define_const(xlib_module, "GCClipXOrigin",    GCClipXOrigin);
  rb_define_const(xlib_module, "GCClipYOrigin",    GCClipYOrigin);
  rb_define_const(xlib_module, "GCClipMask",       GCClipMask);
  rb_define_const(xlib_module, "GCDashOffset",     GCDashOffset);
  rb_define_const(xlib_module, "GCDashList",       GCDashList);
  rb_define_const(xlib_module, "GCArcMode",        GCArcMode);
  rb_define_const(xlib_module, "GCLastBit",        GCLastBit);
  rb_define_const(xlib_module, "FontLeftToRight",  FontLeftToRight);
  rb_define_const(xlib_module, "FontRightToLeft",  FontRightToLeft);
  rb_define_const(xlib_module, "FontChange",       FontChange);
  rb_define_const(xlib_module, "XYBitmap",         XYBitmap);
  rb_define_const(xlib_module, "XYPixmap",         XYPixmap);
  rb_define_const(xlib_module, "ZPixmap",  ZPixmap);
  rb_define_const(xlib_module, "AllocNone",        AllocNone);
  rb_define_const(xlib_module, "AllocAll",         AllocAll);
  rb_define_const(xlib_module, "DoRed",    DoRed);
  rb_define_const(xlib_module, "DoGreen",  DoGreen);
  rb_define_const(xlib_module, "DoBlue",   DoBlue);
  rb_define_const(xlib_module, "CursorShape",      CursorShape);
  rb_define_const(xlib_module, "TileShape",        TileShape);
  rb_define_const(xlib_module, "StippleShape",     StippleShape);
  rb_define_const(xlib_module, "AutoRepeatModeOff",        AutoRepeatModeOff);
  rb_define_const(xlib_module, "AutoRepeatModeOn",         AutoRepeatModeOn);
  rb_define_const(xlib_module, "AutoRepeatModeDefault",    AutoRepeatModeDefault);
  rb_define_const(xlib_module, "LedModeOff",       LedModeOff);
  rb_define_const(xlib_module, "LedModeOn",        LedModeOn);
  rb_define_const(xlib_module, "KBKeyClickPercent",        KBKeyClickPercent);
  rb_define_const(xlib_module, "KBBellPercent",    KBBellPercent);
  rb_define_const(xlib_module, "KBBellPitch",      KBBellPitch);
  rb_define_const(xlib_module, "KBBellDuration",   KBBellDuration);
  rb_define_const(xlib_module, "KBLed",    KBLed);
  rb_define_const(xlib_module, "KBLedMode",        KBLedMode);
  rb_define_const(xlib_module, "KBKey",    KBKey);
  rb_define_const(xlib_module, "KBAutoRepeatMode",         KBAutoRepeatMode);
  rb_define_const(xlib_module, "MappingSuccess",   MappingSuccess);
  rb_define_const(xlib_module, "MappingBusy",      MappingBusy);
  rb_define_const(xlib_module, "MappingFailed",    MappingFailed);
  rb_define_const(xlib_module, "MappingModifier",  MappingModifier);
  rb_define_const(xlib_module, "MappingKeyboard",  MappingKeyboard);
  rb_define_const(xlib_module, "MappingPointer",   MappingPointer);
  rb_define_const(xlib_module, "DontPreferBlanking",       DontPreferBlanking);
  rb_define_const(xlib_module, "PreferBlanking",   PreferBlanking);
  rb_define_const(xlib_module, "DefaultBlanking",  DefaultBlanking);
  rb_define_const(xlib_module, "DisableScreenSaver",       DisableScreenSaver);
  rb_define_const(xlib_module, "DisableScreenInterval",    DisableScreenInterval);
  rb_define_const(xlib_module, "DontAllowExposures",       DontAllowExposures);
  rb_define_const(xlib_module, "AllowExposures",   AllowExposures);
  rb_define_const(xlib_module, "DefaultExposures",         DefaultExposures);
  rb_define_const(xlib_module, "ScreenSaverReset",         ScreenSaverReset);
  rb_define_const(xlib_module, "ScreenSaverActive",        ScreenSaverActive);
  rb_define_const(xlib_module, "HostInsert",       HostInsert);
  rb_define_const(xlib_module, "HostDelete",       HostDelete);
  rb_define_const(xlib_module, "EnableAccess",     EnableAccess);
  rb_define_const(xlib_module, "DisableAccess",    DisableAccess);
  rb_define_const(xlib_module, "StaticGray",       StaticGray);
  rb_define_const(xlib_module, "GrayScale",        GrayScale);
  rb_define_const(xlib_module, "StaticColor",      StaticColor);
  rb_define_const(xlib_module, "PseudoColor",      PseudoColor);
  rb_define_const(xlib_module, "TrueColor",        TrueColor);
  rb_define_const(xlib_module, "DirectColor",      DirectColor);
  rb_define_const(xlib_module, "LSBFirst",         LSBFirst);
  rb_define_const(xlib_module, "MSBFirst",         MSBFirst);
}
