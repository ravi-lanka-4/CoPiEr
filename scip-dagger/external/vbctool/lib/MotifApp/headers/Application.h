///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//         This example code is from the book:
//
//           Object-Oriented Programming with C++ and OSF/Motif
//         by
//           Douglas Young
//           Prentice Hall, 1992
//           ISBN 0-13-630252-1	
//
//         Copyright 1991 by Prentice Hall
//         All Rights Reserved
//
//  Permission to use, copy, modify, and distribute this software for 
//  any purpose except publication and without fee is hereby granted, provided 
//  that the above copyright notice appear in all copies of the software.
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Application.h: 
////////////////////////////////////////////////////////////
#ifndef APPLICATION_H
#define APPLICATION_H
#include "UIComponent.h"
#include "MainWindow.h"

class Application : public UIComponent {
    
    // Allow main and MainWindow to access protected member functions

#if (XlibSpecificationRelease>=5)
    friend int main ( int, char ** );            // changed to int (JK)
#else
    friend int main ( unsigned int, char ** );   // changed to int (JK)
#endif
    
    friend class MainWindow;  // MainWindow needs to call 

    // private functions for registration

  private:    
    
    // Functions for registering and unregistering toplevel windows
    
    void registerWindow ( MainWindow * );
    void unregisterWindow ( MainWindow * );
    
  protected:
    
    // Support commonly needed data structures as a convenience
    
    Display     *_display;
    XtAppContext _appContext;
    
    // Functions to handle Xt interface
#if (XlibSpecificationRelease>=5)   
    virtual void initialize ( int *, char ** );  
#else
    virtual void initialize ( unsigned int *, char ** );  
#endif
    virtual void handleEvents();
    
    char   *_applicationClass;    // Class name of this application
    MainWindow  **_windows;       // top-level windows in the program
    int           _numWindows;
    
  public:
    
    Application ( char * );
    virtual ~Application();     
    
    // Functions to manipulate application's top-level windows
    
    void manage();
    void unmanage();
    void iconify();
    
    // Convenient access functions
    
    Display      *display()     { return _display; }
    XtAppContext  appContext()  { return _appContext; }
    const char   *applicationClass()  { return _applicationClass; }
    MainWindow	 *mainWindow() { return _windows[0];}
    
    virtual const char *const className() { return "Application"; }
};

// Pointer to single global instance

extern Application *theApplication; 

#endif
