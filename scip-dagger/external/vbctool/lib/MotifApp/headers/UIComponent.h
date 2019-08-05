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
// UIComponent.h: Base class for all C++/Motif UI components
///////////////////////////////////////////////////////////////
#ifndef UICOMPONENT_H
#define UICOMPONENT_H
#include <Xm/Xm.h>
#include "BasicComponent.h"

class UIComponent : public BasicComponent {
    
  private:
    
    // Interface between XmNdestroyCallback and this class
    
    static void widgetDestroyedCallback ( Widget, 
					 XtPointer, 
					 XtPointer );
    
  protected:
    
    // Protect constructor to prevent direct instantiation
    
    UIComponent ( const char * );
    
    void installDestroyHandler(); // Easy hook for derived classes
    
    // Called by widgetDestroyedCallback() if base widget is destroyed
    
    virtual void widgetDestroyed(); 
    
    // Loads component's default resources into database
    
    void setDefaultResources ( const Widget , const String *);
    
    // Retrieve resources for this clsss from the resource manager
    
    void getResources ( const XtResourceList, const int );
    
  public:
    
    virtual ~UIComponent();  // Destructor
    
    // Manage the entire widget subtree represented
    // by this component. Overrides BasicComponent method
    
    virtual void manage();
    
    // Public access functions
    
    virtual const char *const className() { return "UIComponent"; }
};

#endif
