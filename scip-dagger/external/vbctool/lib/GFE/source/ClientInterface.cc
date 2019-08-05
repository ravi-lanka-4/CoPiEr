////////////////////////////////////////////////////////////////////////
//   Filename : ClientInterface.cc
//
//   Version  : 
//
//   Author   : 
//
//   Language : C++
//
//   Purpose  : 
//
////////////////////////////////////////////////////////////////////////        

#include "GFE.inc"
#include "Application.h"
#include "Cmd.h"
#include "CmdList.h"

#include "GFEWindow.h"
#include "LoadCmd.h"
#include "SaveCmd.h"
#include "DisplayArea.h"

#include "ClientInterface.h"
#include "Version.h"

ClientInterface::ClientInterface()
{
   theClientInterface = this;
}

void ClientInterface::cmdInit()
{

   _loadCmd = new LoadCmd("Load", False);
   _saveCmd = new SaveCmd("Save", False);
}

void ClientInterface::init()
{
   if (generalInit() == ERROR) printf("ERROR in init!\n");
   if (graphicInit() == ERROR) printf("ERROR in graphicInit!\n");
   if (menuInit(theGFEWindow->menuBar()) == ERROR) printf("ERROR in menuInit!\n");

}

int ClientInterface::generalInit()
{ 
   return SUCCESS;
}

int ClientInterface::graphicInit()
{
   return SUCCESS;
}

int ClientInterface::menuInit(MenuBar *dummy)
{
   return SUCCESS;
}

int ClientInterface::load(FILE *dummy, char *s)
{
   return SUCCESS;
}

int ClientInterface::save(FILE *dummy, char *s)
{
   return SUCCESS;
}

ClientInterface::~ClientInterface()
{
}


void ClientInterface::addToExistingMenu(CmdList* file, CmdList* edit, CmdList *view)
{}

void ClientInterface::addToMenuPoint(CmdList* list, Cmd* cmd, int pos)
{
   list->add(cmd);

   if (pos < 1) pos = 1;
   if (pos-- > list->size()) return;

   Cmd **contents = list->contents();
   int size = list->size() - 2;

   for (; size >=  pos; size--)
      contents[size+1] = contents[size];

   contents[size+1] = cmd;
}


// must not be declared inline, because you cannot overwrite it later
char *ClientInterface::prefix()
{
   return PREFIX;
}
char *ClientInterface::version()
{
   return PROGRAM;
}


int ClientInterface::handleFirstEvents()
{
   return 0;
}
