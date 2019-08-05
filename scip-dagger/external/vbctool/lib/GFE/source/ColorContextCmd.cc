/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	ColorContextCmd.cc
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	Cmd for pressing Menu-Button Color/Context-Chooser
// 
//
////////////////////////////////////////////////////////////////////////	
#include "ColorContextCmd.h"
// #include "ColorContext.h"
#include "GFE.inc"
#include "DrawArea.h"
#include "ColorContext.h"

// test-includes

#include <Xm/Form.h>



ColorContextCmd::ColorContextCmd(char *name,
			int active)
	 : NoUndoCmd(name, active)

{}

void ColorContextCmd::doit()
{
   theDrawArea->colorContextPopup()->post();

}
