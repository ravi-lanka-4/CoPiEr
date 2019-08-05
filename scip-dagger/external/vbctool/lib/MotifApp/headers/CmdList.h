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
// CmdList.h: Maintain a list of Cmd objects
////////////////////////////////////////////////////////////

class Cmd;

class CmdList {
    
private:
    
    Cmd **_contents;    // The list of objects
    int   _numElements; // Current size of list
    
public:
    
    CmdList();           // Construct an empty list
    virtual ~CmdList();  // Destroys list, but not objects in list
    
    void add ( Cmd * );  // Add a single Cmd object to list
    
    Cmd **contents() { return _contents; } // Return the list
    int size() { return _numElements; }    // Return list size
    Cmd *operator[]( int );            // Return an element of the list
};
