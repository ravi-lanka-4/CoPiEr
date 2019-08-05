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
// UIComponent.C: Base class for all C++/Motif UI components
///////////////////////////////////////////////////////////////
#include "UIComponent.h"
#include <assert.h>
#include <stdio.h>

UIComponent::UIComponent ( const char *name ) : BasicComponent ( name )
{
    // Empty
}


void UIComponent::widgetDestroyedCallback ( Widget, 
					   XtPointer clientData, 
					   XtPointer )
{
    UIComponent * obj = (UIComponent *) clientData;	
    
    obj->widgetDestroyed();
}

void UIComponent::widgetDestroyed()
{
    _w = NULL;
}

void UIComponent::installDestroyHandler()
{
    assert ( _w != NULL );
    XtAddCallback ( _w, 
		   XmNdestroyCallback,
		   &UIComponent::widgetDestroyedCallback, 
		   (XtPointer) this );
}

void UIComponent::manage()
{
    assert ( _w != NULL );
    assert ( XtHasCallbacks ( _w, XmNdestroyCallback ) ==
	    XtCallbackHasSome );
    XtManageChild ( _w );
}

UIComponent::~UIComponent()
{
    // Make sure the widget hasn't already been destroyed
    
    if ( _w ) 
    {
	// Remove destroy callback so Xt can't call the callback
	// with a pointer to an object that has already been freed
	
	XtRemoveCallback ( _w, 
			  XmNdestroyCallback,
			  &UIComponent::widgetDestroyedCallback,
			  (XtPointer) this );	
    }
}

void UIComponent::getResources ( const XtResourceList resources, 
				const int numResources )
{
    // Check for errors
    
    assert ( _w != NULL );
    assert ( resources != NULL );
    
    // Retrieve the requested resources relative to the 
    // parent of this object's base widget
    
    XtGetSubresources ( XtParent( _w ), 
		       (XtPointer) this, 
		       _name,
		       className(),
		       resources, 
		       numResources,
		       NULL, 
		       0 );
}


void UIComponent::setDefaultResources ( const Widget w, 
                                         const String *resourceSpec )
{
   int         i;	
   Display    *dpy = XtDisplay ( w );	  // Retrieve the display pointer
   XrmDatabase rdb = NULL;             // A resource data base

   // Create an empty resource database

   rdb = XrmGetStringDatabase ( "" );

   // Add the Component resources, prepending the name of the component

   i = 0;
   while ( resourceSpec[i] != NULL )
   {
       char buf[1000];

       sprintf(buf, "*%s%s", _name, resourceSpec[i++]);
       XrmPutLineResource( &rdb, buf );
   }

   // Merge them into the Xt database, with lowest precendence

   if ( rdb )
   {
#if (XlibSpecificationRelease>=5)
        XrmDatabase db = XtDatabase(dpy);
	XrmCombineDatabase(rdb, &db, FALSE);
#else
        XrmMergeDatabases ( dpy->db, &rdb );
        dpy->db = rdb;
#endif
    }
}

