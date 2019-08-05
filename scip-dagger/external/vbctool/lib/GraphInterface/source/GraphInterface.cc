
#include "Status.h"
#include "GraphInterface.h"
#include "NodeInformation.h"
#include "NodePopup.h"
#include "NodeSelection.h"
#include "EdgeInformation.h"
#include "GFEWindow.h"
#include "NodeEdgePopup.h"
#include "Interrupt.h"
#include "SingleStep.h"
#include "MenuBar.h"
#include "Version.h"

//#include <unistd.h>
#include <assert.h>


// for counting the nodes in the Changer in a regular way
#define REGULARNODECOUNT

GraphInterface::GraphInterface(int maxnc, int maxec) : ClientInterface()
{
   theGraphInterface = this;

   _nodeSelection = NULL;
   _graph = NULL;

   _maxNodeCategories = maxnc;
   _maxEdgeCategories = maxec;
   _nodeCategorie = NULL;
   _edgeCategorie = NULL;
   _nodeListPopup = NULL;
   _edgeListPopup = NULL;
   _interrupt = NULL;
   _singleStep = NULL;
}

GraphInterface::~GraphInterface()
{
   kill(_nodeSelection);
   kill(_graph);
   kill(_nodeListPopup);
   kill(_edgeListPopup);
   kill(_interrupt);
   kill(_singleStep);

   if (_nodeCategorie) {
      for (int i = 1; i <= _maxNodeCategories; i++)
	 delete _nodeCategorie[i];
      delete[] _nodeCategorie;
   }

   if (_edgeCategorie) {
      for (int i = 1; i <= _maxEdgeCategories; i++)
	 delete _edgeCategorie[i];
      delete[] _edgeCategorie;
   }
}

void GraphInterface::cmdInit()
{
   ClientInterface::cmdInit();

   initCategories();

   _graph = new Graph();

   _nodeListPopup = new NodeListPopup("Nodes", True, "Look of Nodes", theGFEWindow->baseWidget());
   _edgeListPopup = new EdgeListPopup("Edges", True, "Look of Edges", theGFEWindow->baseWidget());

   _nodeSelection = new NodeSelection("Node Selection",
				      theGFEWindow->menuBar()->baseWidget());

   _interrupt = new Interrupt();
   _singleStep = new SingleStep();
}

int GraphInterface::forkProcess(int (*func)(char*, int, int), char *f, int a, int b)
{
/*   int pid = fork();

   if (pid == -1) return pid;

   if (pid == 0)
      func(f, a, b);
   else
      return pid;
*/
return 0;
}

void GraphInterface::killProcess(int pid)
{
   //kill(pid, -9);
}

void GraphInterface::addToExistingMenu(CmdList *file, CmdList *edit, CmdList *view)
{
   view->add((Cmd *) _nodeListPopup);
   view->add((Cmd *) _edgeListPopup);
}


inline
void GraphInterface::initCategories()
{
      char buffer[5];

      char input1[128];
      char input2[128];
      char input3[128];

      char input4[512];

      char input5[128];
      char input6[128];
      char input7[128];
      char input8[128];

      char text[256];
      char text1[256];
      char text2[256];

      char *xName1;
      char *xName2;
      char *font;
      char *customName;

      int number = 1;

      assert(_maxNodeCategories>=1); // register at least one categorie
   
      _nodeCategorie = new NodeCategorie*[_maxNodeCategories+1];
   
      GFEResourceManager rsc(STANDARDRESOURCE);

      for (int i = 1; i <= _maxNodeCategories; i++) {
	 strcpy(text, "Nodes ");
	 sprintf(buffer, "%d", i);
	 strcat(text, buffer);

	 char *line = rsc.getstring(text);
	 int count;
	 input1[0] = input2[0] = input3[0] = input4[0] =
	 input5[0] = input6[0] = input7[0] = '\0';
	 input4[1] = '\0';
	 strcpy(input8, "---------");
	 if(line)
	    count = sscanf(line, "%s%s%s%s%s%s%s%s", input1, input2, input3, input8,
			   input4, input5, input6, input7);
	 else
	    count = 0;

	 if (count > 5) {
	    strcat(input4, " ");
	    strcat(input4, input5);
	 }
	 if (count > 6) {
	    strcat(input4, " ");
	    strcat(input4, input6);
	 }
	 if (count > 7) {
	    strcat(input4, " ");
	    strcat(input4, input7);
	 }

	 xName1 = input1+1;
	 if (count<1 || (input1[0]=='-' && (input1[1]=='\0' || input1[1]=='\n')))
	    xName1 = nodeColor(i);
	 if (!xName1) xName1 = "black";

	 xName2 = input2+1;
	 if (count<2 || (input2[0]=='-' && (input2[1]=='\0' || input2[1]=='\n')))
	    xName2 = nodeFontColor(i);
	 if (!xName2) xName2 = "gray50";

	 font = input3+1;
	 if (count<3 || (input3[0]=='-' && (input3[1]=='\0' || input3[1]=='\n')))
	    font = nodeFont(i);
	 if (!font) font = "TimesMedium";

	 customName = input4+1;
	 if (count<5 || (input4[0]=='-' && (input4[1]=='\0' || input4[1]=='\n')))
	    customName = nodeCustomName(i);

	 if (!customName) {
#ifndef  REGULARNODECOUNT
	    strcpy(text, "Nodes ");
	    sprintf(buffer, "%d", number++);
	    strcat(text, buffer);
#endif
	    customName = text;
	 }

	 Boolean locked = False;
	 if (input4[1] == '-') locked = True;
	 if (customName[0] == '-') customName++;

	 strcpy(text1, "Node-Color ");
	 strcat(text1, customName);
	 strcpy(text2, "Font-Color ");
	 strcat(text2, customName);

	 Boolean displayNumbers = False;
	 Boolean fillNodes = False;
	 Boolean nodesAreCircles = True;

	 if (input8[1] == 'y' || input8[1] == 'Y') displayNumbers = True;
	 if (input8[2] == 'y' || input8[2] == 'Y') fillNodes = True;
	 if (input8[3] == 'n' || input8[3] == 'N') nodesAreCircles = False;
	 
	 _nodeCategorie[i] = new NodeCategorie(customName, xName1, text1,
					       xName2, text2, font, 0.5,
					       displayNumbers, fillNodes, nodesAreCircles, locked);
      }

      assert(_maxEdgeCategories>=1); // register at least one categorie
   
      _edgeCategorie = new EdgeCategorie*[_maxEdgeCategories+1];

      number = 1;
      
      for (int i = 1; i <= _maxEdgeCategories; i++) {
	 strcpy(text, "Edges ");
	 sprintf(buffer, "%d", i);
	 strcat(text, buffer);

	 char *line = rsc.getstring(text);
	 int count;
	 input1[0] = input2[0] = input3[0] = input4[0] =
	 input5[0] = input6[0] = input7[0] = '\0';
	 input4[1] = '\0';
	 if(line)
	    count = sscanf(line, "%s%s%s%s%s", input1, input4,
			   input5, input6, input7);
	 else
	    count = 0;

	 if (count > 2) {
	    strcat(input4, " ");
	    strcat(input4, input5);
	 }
	 if (count > 3) {
	    strcat(input4, " ");
	    strcat(input4, input6);
	 }
	 if (count > 4) {
	    strcat(input4, " ");
	    strcat(input4, input7);
	 }

	 xName1 = input1+1;
	 if (count<1 || (input1[0]=='-' && (input1[1]=='\0' || input1[1]=='\n')))
	    xName1 = edgeColor(i);
	 if (!xName1) xName1 = "blue";

	 customName = input4+1;
	 if (count<2 || (input4[0]=='-' && (input4[1]=='\0' || input4[1]=='\n')))
	    customName = edgeCustomName(i);

	 if (!customName) {
#ifndef  REGULARNODECOUNT
	    strcpy(text, "Edges ");
	    sprintf(buffer, "%d", number++);
	    strcat(text, buffer);
#endif
	    customName = text;
	 }

	 Boolean locked = False;
	 if (input4[1] == '-') locked = True;
	 if (customName[0] == '-') customName++;

	 strcpy(text1, "Edge-Color ");
	 strcat(text1, customName);
	 
	 _edgeCategorie[i] = new EdgeCategorie(customName, xName1, text1, 0.0, False, locked);
	 CoordType ew;
	 if ((ew = edgeWidth(i)) != (CoordType)(-1))
	    theDrawArea->getContext(_edgeCategorie[i]->outer())->
	       setLineWidth(ew);
      }

}


char *GraphInterface::version()
{
   return PROGRAM;
}
char *GraphInterface::prefix()
{
   return PREFIX;
}
