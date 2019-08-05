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


///////////////////////////////////////////////////////
// CmdInterface.C
/////////////////////////////////////////////////////////
#include "CmdInterface.h"
#include "Cmd.h"

CmdInterface::CmdInterface ( Cmd *cmd ) : UIComponent( cmd->name() )
{
    _active = TRUE;
    _cmd    = cmd;
    cmd->registerInterface ( this );
}

void CmdInterface::executeCmdCallback ( Widget, 
				       XtPointer clientData,
				       XtPointer )
{
    CmdInterface *obj = (CmdInterface *) clientData;
    
    obj->_cmd->execute();     
}

void CmdInterface::activate()
{
    if ( _w )
	XtSetSensitive ( _w, TRUE );
    _active = TRUE;
}

void CmdInterface::deactivate()
{
    if ( _w )
	XtSetSensitive ( _w, FALSE );
    _active = FALSE;
}
