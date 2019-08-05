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


///////////////////////////////////////////////////////////////
// BasicComponent.h: First version of a class to define 
//                    a protocol for all components
///////////////////////////////////////////////////////////////
#ifndef BASICCOMPONENT_H
#define BASICCOMPONENT_H
#include <Xm/Xm.h>

class BasicComponent {
    
  protected:
    
    char    *_name;
    Widget   _w;    
    
    // Protected constructor to prevent instantiation
    
    BasicComponent ( const char * );   
    
  public:
    
    virtual ~BasicComponent();
    virtual void manage();   // Manage and unmanage widget tree
    virtual void unmanage();
    const Widget baseWidget() { return _w; }
};
#endif

