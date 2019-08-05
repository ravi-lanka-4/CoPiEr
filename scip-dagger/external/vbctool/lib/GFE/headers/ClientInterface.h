////////////////////////////////////////////////////////////////////////
//   Filename : ClientInterface.h
//
//   Version  : 
//
//   Author   : Martin Diehl
//
//   Language : C++
//
//   Purpose  : Interface-class to include easily other clients
//
////////////////////////////////////////////////////////////////////////        
#ifndef CLIENTINTERFACE_H
#define CLIENTINTERFACE_H

#include <stdio.h>
#include <X11/cursorfont.h>

#include "Geometric.h"
#include "GFE.inc"
#include "Application.h"

#define DISPLAYAREA  1
#define NODESBUTTON  2

class DrawArea;
class MenuBar;
class GFEWindow;
class LoadCmd;
class SaveCmd;
class DisplayArea;
class Cmd;
class CmdList;

class ClientInterface{

   private:

      LoadCmd	  *_loadCmd;
      SaveCmd	  *_saveCmd;

   protected:
         
   public:

      virtual void cmdInit();
      void init();
      virtual int  generalInit();
      virtual int  graphicInit();
      virtual int  menuInit(MenuBar *);
      virtual int  load(FILE *, char*);
      virtual int  save(FILE *, char*);

      virtual void button1Press(Point<CoordType>) {}
      virtual void button3Press(Point<CoordType> p) { button1Press(p); }

      virtual void button1Motion(Point<CoordType>*) {}

      LoadCmd	  *loadCmd()	 { return _loadCmd; }
      SaveCmd	  *saveCmd()	 { return _saveCmd; }

      ClientInterface();
      virtual ~ClientInterface();

      virtual char *prefix();
      virtual char *version();

      virtual unsigned long configuration()
      {
	 return (unsigned long) (DISPLAYAREA);
      }

      void addToMenuPoint(CmdList*, Cmd*, int pos);
      virtual void addToExistingMenu(CmdList*, CmdList*, CmdList*);

      virtual int handleFirstEvents();

};

extern ClientInterface *theClientInterface;

#endif

