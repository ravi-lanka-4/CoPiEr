/*****************************************************************

            Filename    :  TreeScalerDiag.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File.
	    
******************************************************************/


#include "TreeScalerDiag.h"
#include "TreeInterface.h"
#include <Xm/Form.h>
#include <Xm/Scale.h>
#include <Xm/SelectioB.h>



/*****************************************************************
                    TreeScalerDiag
******************************************************************/


TreeScalerDiag::TreeScalerDiag(char* name, Widget parent)
   : PopupManager(name,parent)   // call base-class constructor

   // Constructor 
{
   _T = theTreeInterface->Tree();
   _visual_graph = theTreeInterface->graph();

   _decimalFaktor = 10;
   _decimalPoints = 1;

   radius_max = 1;
}



/*****************************************************************
                    TreeScalerDiag
******************************************************************/


TreeScalerDiag::~TreeScalerDiag()

   // Destructor
{
}



/*****************************************************************
                    _setMaxValues
******************************************************************/


void TreeScalerDiag::_setMaxValues(int subtree,int sibling)

   // Set the maximal values of the scalers for the separation
   // values of subtree and sibling separation.
{
   subtree_max = subtree;
   sibling_max = sibling;
}



/*****************************************************************
                        cancel
******************************************************************/


void TreeScalerDiag::cancel()

   // A derived fundtion from PopupManager.
   // Controls the cancel button.
   // It simply closes the dialog window without any changes in the
   // drawing of the tree.
{
}



/*****************************************************************
                          ok
******************************************************************/


void TreeScalerDiag::ok()

   // A derived fundtion from PopupManager.
   // Controls the ok button.
   // It reads the input from the dialog window and makes the necessary
   // changes in the drawing of the tree.
   // Then it closes the dialog window.
{
   apply();
   cancel();
}



/*****************************************************************
                        appply
******************************************************************/


void TreeScalerDiag::apply()

   // A derived function from PopupManager.
   // Controls the apply button.
   // It reads the input from the dialog window and makes the necessary
   // changes in the drawing of the tree.
{
   int    lvalue = 0;
   int    svalue = 0;
   int    tvalue = 0;
   int    makeRepaint = FALSE;
   unsigned int maximum = 0;
   unsigned int helpmax = 0; 
   unsigned int minimum = 5;
   double x_max = 0.0;
   double y_max = 0.0;

                                   // Get the new Value for the Level
				   // Separation. That means, get the
				   // new distance that nodes between
				   // two adjacent level must have.
   XtVaGetValues(_levelScale,XmNvalue,&lvalue,NULL);
   lvalue = lvalue + 2;
   XtVaGetValues(_siblingScale,XmNvalue,&svalue,NULL);
   XtVaGetValues(_subtreeScale,XmNvalue,&tvalue,NULL);

   if ((lvalue != _T->get_actual_level_separation())
      && (svalue == _T->get_sib_separation())
      && (tvalue == _T->get_subtree_separation()))
                                   // The new level Separation really
				   // differs from the old one while
				   // all other values stay the same.
   {
                                   // Make sure a repaint is done after
				   // computing the new coordinates.
      makeRepaint = TRUE;
                                   // Compute the new Coordinates. This
				   // is done by simply changing the
				   // y coordinates of the nodes.
      _T->newPositionLevel(lvalue,&x_max,&y_max);
   }
   else if ((svalue != _T->get_sib_separation())
           || (tvalue != _T->get_subtree_separation()))
                                   // Either the new sibling or the new
				   // subtree separation value differ from
				   // their old ones.
   {
                                   // Make sure a repaint is done after
				   // computing the new coordinates.
      makeRepaint = TRUE;
                                   // Compute the new Coordinates. In this
				   // case it has to be done by completely
				   // running the algorithm of walker again.
      _T->set_separation_values(svalue,(lvalue-2),tvalue);
      _T->positiontree(&x_max,&y_max);
   }

   maximum = ((svalue+2) / 2 ) * _decimalFaktor;
   helpmax = (lvalue / 2)* _decimalFaktor;
   if (helpmax < maximum)
      maximum = helpmax;
   helpmax = ((tvalue+2) / 2) *_decimalFaktor;
   if (helpmax < maximum)
      maximum = helpmax;
   
				   
   if (makeRepaint)
   {		 
				   // Register nodes and edges in the
				   // GRAPH Class of the GraphInterface.
      for (int i = 1; i <= _T->get_node_nb(); i++)
      {
         _visual_graph->registerNode(_T->get_node_colour(i),i,
	                             _T->get_Xcoord(i),
	                             _T->get_Ycoord(i),TRUE);
      }
                                   // Changed after using gcc 2.7.0
      for (int j = 1; j < _T->get_node_nb(); j++)
         _visual_graph->drawEdge(STANDARDEDGECOLOUR,_T->getnode_edge_first(j),
                                 _T->getnode_edge_sec(j));

   }


                                   // Get the Value of the new radius
				   // that ALL nodes of ALL Node-Categories
				   // must have. That means, all nodes
				   // either they are painted green or red
				   // are going to have the same radius.
   int value = 0;
   XtVaGetValues(_radiusScale,XmNvalue,&value,NULL);
   double radius = (double) value/_decimalFaktor;

   if (radius != theTreeInterface->ActualRadius()
      || ((maximum > 0) && maximum < radius_max))
                                   // The new radius really differs from 
				   // the old one, or the maximal value
				   // that the radius can take, has been
				   // restricted by smaller separation
				   // values. In the first case, the radius
				   // of all node categories has to be reset.
				   // In the latter case, the new choices,
				   // that the user made, probably forces
				   // the algorithm to make the radius of
				   // all node categories smaller. This is
				   // done to obey the strict supposition
				   // that nodes are not allowed to
				   // intersect each other for the sake
				   // of clarity.
   {
                                   // Make sure a repaint is done after
				   // changing the radius.
      makeRepaint = TRUE;
      
      if ( (round(radius *_decimalFaktor)) > maximum)
                                   // The new separation values are smaller
				   // then doubble times the radius. So
				   // two nodes, adjacent to each other
				   // will intersect in the drawing.
				   // The value of the radius is reset to the
				   // new maximum value and the values
				   // of the widget _radiusScale are adapted.
      {
         setXtValueRadius(maximum,minimum,maximum);		 
         radius  = maximum/10;
	 radius_max = maximum;
                                   // Now reset the radius of all node
				   // categories.
         for (int i = 1; i <= COLOURCOUNT; i++)
         {
            _visual_graph->setRadius(i,radius);
         }
         theTreeInterface->NewActualRadius(radius);
      }
      else if (radius_max > maximum)
                                   // The new separation values are not smaller
				   // than doubble times the radius, but they
				   // are smaller than the maximal value
				   // that the user can choose in the widget
				   // _radiusScale. So this value has to be
				   // corrected, so that the user cannot
				   // choose a forbidden value.
      {
         setXtValueRadius(round(radius *_decimalFaktor),minimum,maximum);
	 radius_max = maximum;
      }
      else
                                   // Only the radius was changed without
				   // beeing affected by separation values
				   // reset the radius of all nodes.
				   
         for (int i = 1; i <= COLOURCOUNT; i++)
         {
            _visual_graph->setRadius(i,radius);
         }
         theTreeInterface->NewActualRadius(radius);
   }

   else if (radius_max <= (maximum-1))
                                   // The new separation values are larger
				   // than the maximal value that the user
				   // can choose for the radius. Therefore
				   // this vlaue is corrected in the widget
				   // _radiusScale, so that the user can
				   // choose the new possible values.
   {
      setXtValueRadius(round(radius *_decimalFaktor),minimum,maximum);
      radius_max = maximum;
   }

   
   
   if (makeRepaint)
                                    // If there have been any changes
				    // according to the layout of the
				    // tree, do a Repaint.
   {
				   // Initialize a new Coordinate System.
      if ((x_max == 0) || (y_max == 0))
         theTreeInterface->MaxCoordValues(&x_max,&y_max);
      Rect<CoordType> r;
      x_max = x_max + (2*radius);
      y_max = y_max + (2*radius);
      r.newInit(0,0,x_max,y_max);
      r.growPercent(10.0);
      theDrawArea->setCoordinateRange(&r,FALSE);
   }


}




/*****************************************************************
                         post
******************************************************************/


void TreeScalerDiag::post()

   // Post the Dialog Window on to the Screen, when the button Scaler
   // in the MenuBar is pressed. The Window contains two Scaler Widgets
   // and the values of the scalers are set here as well.
{

   PopupManager::post();

				     // Compute the maximal value of
				     // the Level Separation Scaler.
				     // Get the width of the Tree
				     // and devide it by the number of
				     // levels in the tree. If the User
				     // then chooses the maximal value,
				     // the tree hight is as large as its
				     // width. It is of no use to choose
				     // a higher value, since than the tree's
				     // hight will be larger than its width.
				     // As minimal value we choose the tree's
				     // Standard Level Separation (normally 4).
				     // The actual value of the scaler is
				     // set by the trees actual level
				     // separation.
				     // The values of the Scaler are shown
				     // as positiv integers.
   unsigned int maximum = int(_T->get_max_X_coord() / _T->get_level_count());
   maximum++;
   unsigned int minimum = 0;
   if (maximum  < (_T->get_actual_level_separation() -2))
      maximum = (_T->get_actual_level_separation() -2);

   XtVaSetValues(_levelScale,
                 XmNvalue, (_T->get_actual_level_separation() -2),
		 XmNminimum, minimum,
		 XmNmaximum, maximum,
		 XmNshowValue, TRUE,
		 XtVaTypedArg, XmNtitleString, XmRString,
		 "Level Separation",strlen("Level Separation")+1,
		 NULL);


   minimum = 0;
   XtVaSetValues(_siblingScale,
                 XmNvalue, (_T->get_sib_separation()),
		 XmNminimum, minimum,
		 XmNmaximum, sibling_max,
		 XmNshowValue, TRUE,
		 XtVaTypedArg, XmNtitleString, XmRString,
		 "Sibling Separation",strlen("Sibling Separation")+1,
		 NULL);

   XtVaSetValues(_subtreeScale,
                 XmNvalue, (_T->get_subtree_separation()),
		 XmNminimum, minimum,
		 XmNmaximum, subtree_max,
		 XmNshowValue, TRUE,
		 XtVaTypedArg, XmNtitleString, XmRString,
		 "Subtree Separation",strlen("Subtree Separation")+1,
		 NULL);

   minimum = 5;
   
                                     // Get the maximal values of the
				     // Radius Scaler. It depends on
				     // the value of the tree that is used to
				     // separate siblings in the tree and
				     // the actual level separation.
				     // The smaller value is choosen in order
				     // to draw the nodes in such a way, that
				     // they do not intersect each other.
				     // The minimum value is set to 0.5.
				     // Multiplication with the _decimalFaktor
				     // is necessarry, since XtVaSetValues
				     // only accepts unsigned integers.
				     // The actual value is set by the
				     // actually used radius in the tree.
   maximum = ((_T->get_sib_separation()+2) / 2 ) * _decimalFaktor;
   unsigned int helpmax = (_T->get_actual_level_separation() / 2)
                           * _decimalFaktor;
   if (helpmax < maximum)
      maximum = helpmax;
   helpmax = ((_T->get_subtree_separation()+2) / 2) *_decimalFaktor;
   if (helpmax < maximum)
      maximum = helpmax;
   radius_max = maximum;
   
   XtVaSetValues(_radiusScale,
                 XmNvalue, round(theTreeInterface->ActualRadius() *_decimalFaktor),
		 XmNminimum, minimum, 
		 XmNmaximum, maximum,
		 XmNdecimalPoints,_decimalPoints,
		 XmNshowValue, TRUE,
		 XtVaTypedArg, XmNtitleString, XmRString,
		 "Radius of Nodes",strlen("Radius of Nodes")+1,
		 NULL);
		 


}




/*****************************************************************
                   createControlArea
******************************************************************/


void TreeScalerDiag::createControlArea()

   // Creates a Control Area with two Scaler widgets.
{

			             // Create a father widget for
				     // the two Scaler widgets.
   _scalerControl = XtVaCreateManagedWidget("scalerControl",
                                             xmFormWidgetClass,
					     _w,
					     NULL);

                                     // Create a scaler widget for scaling
				     // the level separation value.
   _levelScale = XtVaCreateManagedWidget("levelScale",
                                          xmScaleWidgetClass,
					  _scalerControl,
				          XmNtopAttachment, XmATTACH_FORM,
				          XmNleftAttachment, XmATTACH_FORM,
				          XmNrightAttachment, XmATTACH_FORM,
					  XmNorientation, XmHORIZONTAL,
					  NULL);

				     // Create a scaler widget for scaling
				     // the level separation value.
   _siblingScale = XtVaCreateManagedWidget("siblingScale",
                                          xmScaleWidgetClass,
					  _scalerControl,
				          XmNtopAttachment, XmATTACH_WIDGET,
				          XmNtopWidget, _levelScale,
				          XmNleftAttachment, XmATTACH_FORM,
				          XmNrightAttachment, XmATTACH_FORM,
					  XmNorientation, XmHORIZONTAL,
					  NULL);

				     // Create a scaler widget for scaling
				     // the level separation value.
   _subtreeScale = XtVaCreateManagedWidget("subtreeScale",
                                          xmScaleWidgetClass,
					  _scalerControl,
				          XmNtopAttachment, XmATTACH_WIDGET,
				          XmNtopWidget, _siblingScale,
				          XmNleftAttachment, XmATTACH_FORM,
				          XmNrightAttachment, XmATTACH_FORM,
					  XmNorientation, XmHORIZONTAL,
					  NULL);


                                     // Create a scaler widget for scaling
				     // the node radius value.
   _radiusScale = XtVaCreateManagedWidget("radiusScale",
                                          xmScaleWidgetClass,
					  _scalerControl,
				          XmNtopAttachment, XmATTACH_WIDGET,
				          XmNtopWidget, _subtreeScale,
				          XmNleftAttachment, XmATTACH_FORM,
				          XmNrightAttachment, XmATTACH_FORM,
				          XmNbottomAttachment, XmATTACH_FORM,
					  XmNorientation, XmHORIZONTAL,
					  NULL);

}



/*****************************************************************
                    CreateActionArea
******************************************************************/


void TreeScalerDiag::createActionArea()

   // Creates the Action Area.
{
                                     // The Apply button is not optionally
				     // present by GFE. Manage the Apply
				     // Button, to make it visible.
   XtManageChild(XmSelectionBoxGetChild(_w, XmDIALOG_APPLY_BUTTON));
                                     // Make the Help button sensitive, so
				     // it is clear, that you cannot use it.
   XtSetSensitive(XmSelectionBoxGetChild(_w, XmDIALOG_HELP_BUTTON), FALSE);

}




/*****************************************************************
                    setXtValueRadius
******************************************************************/


void TreeScalerDiag::setXtValueRadius(int value,int min,int max)

   // Sets the three main values: XmNvalue, XmNminimum, XmNmaximum
   // of the widget _radiusScale.
{
   XtVaSetValues(_radiusScale,
                 XmNvalue, value,
		 XmNminimum, min, 
		 XmNmaximum, max,
		 XmNdecimalPoints,_decimalPoints,
		 XmNshowValue, TRUE,
		 XtVaTypedArg, XmNtitleString, XmRString,
		 "Radius of Nodes",strlen("Radius of Nodes")+1,
		 NULL);
}
