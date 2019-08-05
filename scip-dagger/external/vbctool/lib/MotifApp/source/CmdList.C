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
// CmdList.C: Maintain a list of Cmd objects
////////////////////////////////////////////////////////////
#include "CmdList.h"

class Cmd;

CmdList::CmdList()
{
    // The list is initially empty
    
    _contents    = 0;
    _numElements = 0;
}

CmdList::~CmdList()
{
    // free the list
    
    delete []_contents;
}

void CmdList::add ( Cmd *cmd )
{
    int i;
    Cmd **newList;
    
    // Allocate a list large enough for one more element
    
    newList = new Cmd*[_numElements + 1];
    
    // Copy the contents of the previous list to
    // the new list
    
    for( i = 0; i < _numElements; i++)
	newList[i] = _contents[i];
    
    // Free the old list
    
    delete []_contents;
    
    // Make the new list the current list
    
    _contents =  newList;
    
    // Add the command to the list and update the list size.
    
    _contents[_numElements] = cmd;
    
    _numElements++;
}

Cmd *CmdList::operator[] ( int index )
{
    // Return the indexed element
    
    return _contents[index];
}
