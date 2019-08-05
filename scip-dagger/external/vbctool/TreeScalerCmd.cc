/*****************************************************************

            Filename    :  TreeScalerCmd.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File.
	    
******************************************************************/

#include "TreeScalerCmd.h"
#include "GFEWindow.h"
#include "TreeInterface.h"



/*****************************************************************
                    TreeScalerCmd
******************************************************************/


TreeScalerCmd::TreeScalerCmd(char* name,int active)
             : NoUndoCmd(name,active)

   // Constructor
{
   _scalerDialog = new TreeScalerDiag("Coordinates-Range-Scaler",
                                      theGFEWindow->baseWidget());
}



/*****************************************************************
                    ~TreeScalerCmd
******************************************************************/


TreeScalerCmd::~TreeScalerCmd()

   // Destructor
{
   if (_scalerDialog != NULL)
      delete _scalerDialog;
}



/*****************************************************************
                       cleanup
******************************************************************/


void TreeScalerCmd::cleanup()

   // If a new graph or problem is loaded, while the Scaler Dialog Window
   // is still posted, the Window has to be removed from the screen.
{
   _scalerDialog->unpost();
}



/*****************************************************************
                  set_separation_values
******************************************************************/


void TreeScalerCmd::set_separation_values(int sibling,int subtree)

   // When starting the Tree Interface, this command is called
   // in order to set the maximal values of the scaler.
   // It considers only sibling and subtree separation values.
   // Their values are depend on the users selections in the
   // GRAPHStandardResource.rsc file.
   // The level separation value is not considered, since its
   // maximal value is dependen on the width of the trees drawing.
{
   int sep_sib = sibling * sibling * sibling * sibling;
   int sep_tree = subtree * subtree * subtree * subtree;

   if (sep_sib < 10)
      sep_sib = 10;
   else if (sep_sib > MAXSEPARATION)
      sep_sib = MAXSEPARATION;

   if (sep_tree < 10)
      sep_tree = 10;
   else if (sep_tree > MAXSEPARATION)
      sep_tree = MAXSEPARATION;
   _scalerDialog->_setMaxValues(sep_sib,sep_tree);
}



/*****************************************************************
                         doit
******************************************************************/


void TreeScalerCmd::doit()

   // If the Command button Scaler in the menuebar is pressed,
   // this procedure is called. It pops up a window, where the user
   // can choose new scaling parameters
{
   if (theTreeInterface->graph() != NULL)
   {
      _scalerDialog->post();
   }

}


