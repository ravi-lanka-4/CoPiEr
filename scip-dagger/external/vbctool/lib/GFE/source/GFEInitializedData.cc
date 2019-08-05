#include "CCChanger.h"
#include "Color.h"
#include "GFEResourceManager.h"
#include "Postscript.h"
#include "PrintPopup.h"
#include "GFEWindow.h"


#define PERCENT	  -5.0

#define DINA4X    592
#define DINA4Y	  840

#define DINA5X	  DINA4Y/2
#define DINA5Y	  DINA4X
#define DINA6X	  DINA5Y/2
#define DINA6Y	  DINA5X

#define DINA3X	  DINA4Y
#define	DINA3Y	  DINA4X*2
#define DINA2X	  DINA3Y
#define DINA2Y	  DINA3X*2

char *CCChanger::line[] = {"Line solid", "Line on/off dash", "Line double dash"};
char *CCChanger::join[] = {"Join miter", "Join round", "Join bevel" };
char *CCChanger::cap[] = {"Cap not last", "Cap butt", "Cap round", "Cap projecting"};

int  CCChanger::_lwsnov = 1000;

Colormap Color::colorMap = 0;
ColorInformation *Color::_colorInformation = NULL;


char GFEResourceManager::_prefix[20] = { EOF };

char GFEResourceManager::_path[500];

static Rect<int> p1(0, 0, DINA2X, DINA2Y);
static Rect<int> p2(0, 0, DINA3X, DINA3Y);
static Rect<int> p3(0, 0, DINA4X, DINA4Y);
static Rect<int> p4(0, 0, DINA5X, DINA5Y);
static Rect<int> p5(0, 0, DINA6X, DINA6Y);

static Rect<int> l1(0, 0, DINA2Y, DINA2X);
static Rect<int> l2(0, 0, DINA3Y, DINA3X);
static Rect<int> l3(0, 0, DINA4Y, DINA4X);
static Rect<int> l4(0, 0, DINA5Y, DINA5X);
static Rect<int> l5(0, 0, DINA6Y, DINA6X);

Rect<int> *Postscript::_DinAPortrait[]  = { &p1, &p2, &p3, &p4, &p5 };

Rect<int> *Postscript::_DinALandscape[] = { &l1, &l2, &l3, &l4, &l5 };
				   

int Postscript::_leftMargin = 43;
int Postscript::_rightMargin = 43;
int Postscript::_topMargin = 43;
int Postscript::_bottomMargin = 43;

char *Postscript::_landscape = "Landscape";
char *Postscript::_portrait  = "Portrait";

char *Postscript::_prefix = NULL;

char *PrintPopup::_multipleLabels[] = {"Left:", "Right:",
				       "Top:", "Bottom:" };

char *PrintPopup::dins[] = {"Din A2", "Din A3", "Din A4", "Din A5", "Din A6" };
char *PrintPopup::orie[] = {"Portrait", "Landscape"};
char *PrintPopup::posi[] = {"Top", "Bottom"};


// Global Pointers:

ClientInterface *theClientInterface = NULL;
DisplayArea *theDisplayArea = NULL;
DrawArea *theDrawArea = NULL;
GFEWindow *theGFEWindow = NULL;

