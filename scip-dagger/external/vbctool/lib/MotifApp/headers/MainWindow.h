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


////////////////////////////////////////////////////////////////////
// MainWindow.h: Support a toplevel window
////////////////////////////////////////////////////////////////////
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "UIComponent.h"

class MainWindow : public UIComponent {
    
  protected:
    
    Widget   _main;        // The XmMainWindow widget
    Widget   _workArea;    // Widget created by derived class
    
    // Derived classes must define this function to 
    // create the application-specific work area.
    
    virtual Widget createWorkArea ( Widget ) = 0;
    
  public:
    
    MainWindow ( char * );   // Constructor requires only a name
    virtual ~MainWindow();
    
    // The Application class automatically calls initialize() 
    // for all registered main window objects
    
    virtual void initialize();
    
    virtual void manage();   // popup the window
    virtual void unmanage(); // pop down the window
    virtual void iconify();
};
#endif
