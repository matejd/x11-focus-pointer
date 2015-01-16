// Tiny X11 (Xlib) client that moves the pointer to the middle of newly-focused (via Alt+Tab) windows.
//
#include <X11/Xlib.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>

int errorHandler(Display* display, XErrorEvent* event);

int errorHandler(Display*, XErrorEvent*)
{
    // TODO: are any of these errors important?
    // http://tronche.com/gui/x/xlib/event-handling/protocol-errors/default-handlers.html
    return 0;
}

int main(int, char**)
{
    XSetErrorHandler(errorHandler);
    Display* const display = XOpenDisplay(0);
    if (!display) {
        printf("Failed to open X display!\n");
        exit(1);
    }

    const int screen = DefaultScreen(display);
    const Window rootWindow = RootWindow(display, screen);

    Window root, parent, *children;
    unsigned int numChildren;
    if (!XQueryTree(display, rootWindow, &root, &parent, &children, &numChildren)) {
        printf("Failed to query the root window!");
        exit(2);
    }

    const long eventMask = FocusChangeMask | SubstructureNotifyMask;
    XSelectInput(display, rootWindow, eventMask);
    for (unsigned int i = 0; i < numChildren; ++i) {
        XSelectInput(display, children[i], eventMask);
    }
    if (children)
        XFree(children);

    int previousEventMode = -1;
    int previousEventDetail = -1;
    while (true) {
        XEvent event;
        XNextEvent(display, &event);
        if (event.type == FocusIn) {
            const XFocusChangeEvent focusEvent = *reinterpret_cast<XFocusChangeEvent*>(&event);
            // Filter out unwanted focusIn events.
            if (focusEvent.mode != NotifyUngrab ||
                focusEvent.detail != NotifyVirtual) {
                previousEventMode = focusEvent.mode;
                previousEventDetail = focusEvent.detail;
                continue;
            }
            if (previousEventMode == NotifyVirtual &&
                (previousEventDetail == NotifyPointer || previousEventDetail == NotifyInferior)) {
                continue;
            }

            XWindowAttributes attribs;
            XGetWindowAttributes(display, focusEvent.window, &attribs);
            // Move pointer to the middle of the newly focused window.
            XWarpPointer(display, None, focusEvent.window, 0,0,0,0, attribs.width/2, attribs.height/2);
        }
        else if (event.type == CreateNotify) {
            // A CreateNotify event reports when a window is created.
            // We need to add new windows to our list of watched windows.
            const XCreateWindowEvent createEvent = *reinterpret_cast<XCreateWindowEvent*>(&event);
            XSelectInput(display, createEvent.window, eventMask);
        }
    }

    return 0;
}
