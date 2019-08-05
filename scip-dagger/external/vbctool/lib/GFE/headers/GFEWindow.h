/////////////////////////////////////////////////////////////////////////
//
//
//    Filename :  GFEWindow.h
//
//    Version  :  27.10.93
//
//    Author   :  Martin Diehl & Joachim Kupke
//
//    Language :  C++
//
//    Purpose  :  Describes the MainWindow class of GFE
//		  for OSF/Motif.
//
////////////////////////////////////////////////////////////////////////
	    
#ifndef GFEWINDOW_H
#define GFEWINDOW_H

#include "MenuWindow.h"

class DrawArea;
class DisplayArea;
class ClientInterface;

class GFEWindow : public MenuWindow 
{
   private:
	DrawArea	*_drawArea;
	DisplayArea	*_displayArea;
	
    protected:

	virtual Widget  createWorkArea (Widget);
	virtual void	createMenuPanes();

   public:
    
   GFEWindow(char *);
   ~GFEWindow();
   
   DrawArea *drawArea(){return _drawArea;}
   DisplayArea *displayArea() { return _displayArea; }
   MenuBar  *menuBar() {return _menuBar;}
};

extern GFEWindow *theGFEWindow;

#endif
