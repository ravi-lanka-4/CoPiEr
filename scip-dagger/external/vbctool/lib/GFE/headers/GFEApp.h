/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	GFEApp.h
//
//	Version	 :	24.03.94
//
//	Author	 :	Martin Diehl
//
//	Compiler :	C++ (CC)
//
//	Purpose	 :   Header file for GFEApp.cc AND MORE IMPORTAND a
//		     header file for the client's ....App.c file which
//		     adds simply the instantiation of an Interface derived
//		     from ClientInterface.
//
////////////////////////////////////////////////////////////////////////	
#ifndef GFEAPP_H
#define GFEAPP_H

#include "Application.h"
#include "GFEWindow.h"
#include "ClientInterface.h"
#include "debug.h"
#include "DisplayArea.h"

class GFEApplication : public Application {

   private:

      int	      _argc;
      char	    **_argv;
      int	      _firstEvents; // 1 if any, otherwise 0

   protected:

      void initialize (int *argc, char **argv)
      {
	 Application::initialize(argc, argv);
	 if (*argc == 2) DebugInit(argv[1]);
	 _argc = *argc;
	 _argv = argv;
	 theDisplayArea->initialize();
	 _firstEvents = theClientInterface->handleFirstEvents();
      }

   public:

      GFEApplication(char *c) : Application(c) { _argc = 0; _argv = NULL;}
      int firstEvents() { return _firstEvents; }

      int    argc() const { return _argc;}
      char **argv() const { return _argv;}
   
};

#ifndef APPHEADER

GFEApplication *app		 = new GFEApplication(PROGRAM);
GFEWindow  *mainWindow		 = new GFEWindow(VERSION);

#else

extern GFEApplication *app;
extern GFEWindow  *mainWindow;
extern DisplayArea *theDisplayArea;

#endif

//extern ClientInterface *clientInterface;

#endif
