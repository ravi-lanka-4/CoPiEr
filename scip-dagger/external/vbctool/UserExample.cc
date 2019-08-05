/*****************************************************************

            Filename    :  UserExample.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  
			   
******************************************************************/



#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/types.h>
#include <sys/times.h>
#ifdef LINUX
   #include <unistd.h>
#else
   #include <sys/unistd.h>
#endif
#include "TreeInterface.h"



void  clientmain(int argc, char *argv[])
{

   int  node_nb = 0;
   int  edge_nb = 0;
   int  tail = 0;
   int  head = 0;
   
   char *filename = argv[1];
   char inputline[INFOSIZE];
   char *inputptr;
   char infochar;
   char  bufshort2[SHORTSIZE];

   std::ifstream inClientFile(filename,std::ios::in);
   inClientFile.getline(inputline,INFOSIZE);
   inClientFile.getline(inputline,INFOSIZE);
   inClientFile >> node_nb >> edge_nb;

   int *father = new int [node_nb+1];
   father[1] = 0;

   int  j = 1;
   while (j <= edge_nb)
   {
      inClientFile >> tail >> head;
      if (tail < head)
         father[head] = tail;
      j++;
   }


   
   for ( j = 1; j <= node_nb; j++)
   {

//      sleep(1);


      for (int i = 1; i <= 100; i++)
      {
         for (int k = 1; k <= 100; k++);      
	 dispatchEvents();
      }

      printf("Node: %d\n",j);

      theTreeInterface->NewNode(father[j],j,5);
      sprintf(bufshort2,"\\iimportant node: %d\\iTestInformation",j);      
      theTreeInterface->SetNodeInfo(j,bufshort2);
      if (j > 20)
      {
         int node = j - 20;
         sprintf(bufshort2,"\\iimportant node: %d\\iNew TestInformation\\nContains one more line.",node);      
         theTreeInterface->SetNodeInfo(node,bufshort2);
      }
      dispatchEvents();
      if (j > 5)
         theTreeInterface->PaintNode(j-5,4);
      if (j > 1)
         theTreeInterface->PaintNode(j-1,6);
   }      

   delete[] father;

   theTreeInterface->FinishAlgorithm(TRUE,0);
}



   
   
