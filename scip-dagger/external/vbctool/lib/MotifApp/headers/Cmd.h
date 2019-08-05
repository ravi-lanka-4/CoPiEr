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


/////////////////////////////////////////////////////////
// Cmd.h: A base class for all command objects
/////////////////////////////////////////////////////////
#ifndef CMD_H
#define CMD_H

class CmdList;
class CmdInterface;

class Cmd {
    
    friend class CmdInterface;
    
  private:
    
    // Lists of other commands to be activated or deactivated
    // when this command is executed or "undone"
    
    CmdList       *_activationList;
    CmdList       *_deactivationList;
    void            revert();   // Reverts object to previous state
    int            _active;     // Is this command currently active?
    int            _previouslyActive; // Previous value of _active
    char          *_name;             // Name of this Cmd
    CmdInterface **_ci;            
    int            _numInterfaces;
    
  protected:
    
    int           _hasUndo;    // True if this object supports undo
    static Cmd   *_lastCmd;    // Pointer to last Cmd executed
    
    virtual void doit()   = 0;  // Specific actions must be defined
    virtual void undoit() = 0;  // by derived classes
    

  public:
    
    Cmd ( char *,  int ); // Protected constructor
    
    virtual ~Cmd ();                 // Destructor
    
    // public interface for executing and undoing commands
    
    virtual void execute();  
    void    undo();
    
    void    activate();   // Activate this object
    void    deactivate(); // Deactivate this object
    
    // Functions to register dependent commands
    
    void    addToActivationList ( Cmd * );
    void    addToDeactivationList ( Cmd * );
    
    // Register an UIComponent used to execute this command
    
    void    registerInterface ( CmdInterface * );
    
    // Access functions 
    
    int active () { return _active; }
    int hasUndo() { return _hasUndo; }
    const char *const name () { return _name; }
    virtual const char *const className () { return "Cmd"; }
};
#endif
