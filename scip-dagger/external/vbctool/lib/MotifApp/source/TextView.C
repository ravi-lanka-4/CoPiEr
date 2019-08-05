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
// TextView.C: Abstract base class for all text (numerical)
//             views of a ColorModel
/////////////////////////////////////////////////////////////
#include "TextView.h"
#include <Xm/TextF.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>

TextView::TextView ( Widget  parent, 
		    char   *name ) : ColorView ( name )
{
    int n;
    Arg args[10];
    
    // A RowColumn widget manages a 3 by 2 grid of
    // labels and text widgets.
    
    _w = XtVaCreateManagedWidget ( _name, 
				  xmRowColumnWidgetClass, 
				  parent, 
				  XmNorientation,    XmHORIZONTAL,
				  XmNpacking,        XmPACK_COLUMN,
				  XmNnumColumns,     3,
				  XmNentryAlignment, XmALIGNMENT_END,
				  XmNadjustLast,     FALSE,
				  NULL );
    installDestroyHandler(); 
    
    // All text widgets need the same arguments, so set up one
    // arg list to be used by all three
    
    n = 0;
    XtSetArg ( args[n], XmNcolumns,     5 ); n++;
    XtSetArg ( args[n], XmNeditable,    FALSE );n++;
    XtSetArg ( args[n], XmNcursorPositionVisible, FALSE ); n++;
    
    // Create the labels and text output areas. Order is 
    // important if the widgets are to appear as:
    //   label    text
    //   label    text
    
    _label1 = XtCreateManagedWidget ( "label1", 
				     xmLabelWidgetClass, _w, 
				     NULL, 0 );
    _field1 = XtCreateManagedWidget ( "field1",
				     xmTextFieldWidgetClass, _w,
				     args, n );
    
    _label2 = XtCreateManagedWidget ( "label2", 
				     xmLabelWidgetClass, _w, 
				     NULL, 0 );
    _field2 = XtCreateManagedWidget ( "field2",
				     xmTextFieldWidgetClass, _w,
				     args, n );
    
    _label3 = XtCreateManagedWidget ( "label3", 
				     xmLabelWidgetClass, _w, 
				     NULL, 0 );
    _field3 = XtCreateManagedWidget ( "field3",
				     xmTextFieldWidgetClass, _w,
				     args, n );
}
