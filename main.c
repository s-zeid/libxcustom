/* libxcustomtitle
 * An LD_PRELOAD hack to change X window titles.
 * 
 * Copyright (C) 2015, 2017 Scott Zeid.
 * https://code.s.zeid.me/libxcustomtitle
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * Except as contained in this notice, the name(s) of the above copyright holders
 * shall not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization.
 * 
 */

#define _GNU_SOURCE


#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


Bool have_title() {
 return getenv("X_CUSTOM_TITLE") != NULL;
}


char *get_title(char *default_title) {
 char* env_title = getenv("X_CUSTOM_TITLE");
 if (env_title != NULL)
  return env_title;
 return default_title;
}


static void (*next_XSetWMProperties)(
 Display *display, Window w,
 XTextProperty *window_name, XTextProperty *icon_name,
 char *argv[], int argc,
 XSizeHints *normal_hints, XWMHints *wm_hints, XClassHint *class_hints
) = NULL;

void XSetWMProperties(
 Display *display, Window w,
 XTextProperty *window_name, XTextProperty *icon_name,
 char *argv[], int argc,
 XSizeHints *normal_hints, XWMHints *wm_hints, XClassHint *class_hints
) {
 if (have_title()) {
  XTextProperty new_prop;
  char *title = get_title("");
  Xutf8TextListToTextProperty(display, (char **)&title, 1, XUTF8StringStyle, &new_prop);
  
  next_XSetWMProperties(display, w, &new_prop, &new_prop, argv, argc,
                        normal_hints, wm_hints, class_hints);
 } else {
  next_XSetWMProperties(display, w, window_name, icon_name, argv, argc,
                        normal_hints, wm_hints, class_hints);
 }
}


static void (*next_XmbSetWMProperties)(
 Display *display, Window w,
 _Xconst char *window_name, _Xconst char *icon_name,
 char *argv[], int argc,
 XSizeHints *normal_hints, XWMHints *wm_hints, XClassHint *class_hints
) = NULL;

void XmbSetWMProperties(
 Display *display, Window w,
 _Xconst char *window_name, _Xconst char *icon_name,
 char *argv[], int argc,
 XSizeHints *normal_hints, XWMHints *wm_hints, XClassHint *class_hints
) {
 next_XmbSetWMProperties(display, w,
                         (_Xconst char*)get_title((char*)window_name),
                         (_Xconst char*)get_title((char*)icon_name),
                         argv, argc, normal_hints, wm_hints, class_hints);
}


static void (*next_Xutf8SetWMProperties)(
 Display *display, Window w,
 _Xconst char *window_name, _Xconst char *icon_name,
 char *argv[], int argc,
 XSizeHints *normal_hints, XWMHints *wm_hints, XClassHint *class_hints
) = NULL;

void Xutf8SetWMProperties(
 Display *display, Window w,
 _Xconst char *window_name, _Xconst char *icon_name,
 char *argv[], int argc,
 XSizeHints *normal_hints, XWMHints *wm_hints, XClassHint *class_hints
) {
 next_Xutf8SetWMProperties(display, w,
                           (_Xconst char*)get_title((char*)window_name),
                           (_Xconst char*)get_title((char*)icon_name),
                           argv, argc, normal_hints, wm_hints, class_hints);
}


static int (*next_XChangeProperty)(
 Display *display, Window w,
 Atom property, Atom type,
 int format, int mode,
 _Xconst unsigned char *data, int nelements
) = NULL;

int XChangeProperty(
 Display *display, Window w,
 Atom property, Atom type,
 int format, int mode,
 _Xconst unsigned char *data, int nelements
) {
 if (property == XInternAtom(display, "_NET_WM_NAME", False) ||
     property == XInternAtom(display, "_NET_WM_ICON_NAME", False) ||
     property == XA_WM_NAME || property == XA_WM_ICON_NAME) {
  char *title = get_title((char*)data);
  return next_XChangeProperty(display, w,
                              property, XInternAtom(display, "UTF8_STRING", False),
                              format, mode,
                              (_Xconst unsigned char*)title, strlen((const char*)title));
 } else {
  return next_XChangeProperty(display, w, property, type, format, mode, data, nelements);
 }
}


static void (*next_XSetTextProperty)(
 Display *display, Window w,
 XTextProperty *text_prop, Atom property
) = NULL;

void XSetTextProperty(
 Display *display, Window w,
 XTextProperty *text_prop, Atom property
) {
 if ((property == XInternAtom(display, "_NET_WM_NAME", False) ||
      property == XInternAtom(display, "_NET_WM_ICON_NAME", False) ||
      property == XA_WM_NAME || property == XA_WM_ICON_NAME) && have_title()) {
  XTextProperty new_prop;
  char *title = get_title("");
  Xutf8TextListToTextProperty(display, (char **)&title, 1, XUTF8StringStyle, &new_prop);
  return next_XSetTextProperty(display, w, &new_prop, property);
 } else {
  return next_XSetTextProperty(display, w, text_prop, property);
 }
}


void _init(void) {
 //next_%s = dlsym(RTLD_NEXT, "%s");
 next_XSetWMProperties = dlsym(RTLD_NEXT, "XSetWMProperties");
 next_XmbSetWMProperties = dlsym(RTLD_NEXT, "XmbSetWMProperties");
 next_Xutf8SetWMProperties = dlsym(RTLD_NEXT, "Xutf8SetWMProperties");
 next_XChangeProperty = dlsym(RTLD_NEXT, "XChangeProperty");
 next_XSetTextProperty = dlsym(RTLD_NEXT, "XSetTextProperty");
}
