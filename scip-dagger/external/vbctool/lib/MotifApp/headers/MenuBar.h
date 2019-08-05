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


//////////////////////////////////////////////////////////
// MenuBar.h: A menu bar, whose panes support items
//            that execute Cmd's
//////////////////////////////////////////////////////////
#ifndef MENUBAR_H
#define MENUBAR_H
#include "UIComponent.h"

class Cmd;
class CmdList;

class MenuBar : public UIComponent {
    
  public:
    
    MenuBar ( Widget, char * );
    
    // Create a named menu pane from a list of Cmd objects
    
    virtual void addCommands ( CmdList *, char * );
    
    virtual const char *const className() { return "MenuBar"; }
};
#endif   
