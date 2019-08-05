#include "NodePopup.h"
#include "GraphInterface.h"
#include "NodeEdge.h"
#include "NodeInformation.h"
#include "EdgeInformation.h"
#include "Interrupt.h"

char *NodePopup::yesno[2] = {"yes", "no"};
char *NodePopup::circleString[2] = {"Circles", "Squares" };

int NodePopup::_lwsnov = 1000;

NodeCategorie **NodeInformation::_nodeCategorie = NULL;
int	        NodeInformation::_maxcat = 0;

EdgeCategorie **EdgeInformation::_edgeCategorie = NULL;
int	        EdgeInformation::_maxcat = 0;

// Global Pointers:

GraphInterface *theGraphInterface = NULL;
Interrupt* theInterrupt = NULL;
SingleStep* theSingleStep = NULL;
