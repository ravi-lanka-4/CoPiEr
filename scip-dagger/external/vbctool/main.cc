/*****************************************************************

            Filename    :  main.cc

	    Version     :  01.1995, test

	    Author      :  Sebastian Leipert

	    Language    :  C++

******************************************************************/


#include <iostream>
#include <fstream.h>
#include <string.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "def.glb"


main(int argc, char *argv[])
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

   std::cout << "#TYPE: COMPLETE TREE" << std::endl
        << "#TIME: SET" << std::endl
	<< "#BOUNDS: NONE" << std::endl
        << "#INFORMATION: STANDARD" << std::endl
        << "#NODE_NUMBER: NONE" <<  std::endl;
					   
   
   for ( j = 1; j <= node_nb; j++)
   {

      for (int i = 1; i <= 100; i++)
      {
         for (int k = 1; k <= 100; k++);      
//	 dispatchEvents();
      }

      //printf("Node: %d\n",j);

//      theTreeInterface->NewNode(father[j],j,5);
      std::cout << 'N' << ' ' << father[j] << ' '
           << j << ' ' << 5 << std::endl;

//      sprintf(bufshort2,"\\iimportant node: %d\\iTestInformation",j);      
//      theTreeInterface->SetNodeInfo(j,bufshort2);
      std::cout  << 'I' << ' ' << j << ' '
            << "\\iimportant node: " << j << "\\iTestInformation" << std::endl;

      if (j > 20)
      {
         int node = j - 20;
//         sprintf(bufshort2,"\\iimportant node: %d\\iNew TestInformation\\nContains one more line.",node);      
//         theTreeInterface->SetNodeInfo(node,bufshort2);

         std::cout << 'I' << ' ' << node << ' ' << "\\iimportant node: " << node
	      << "\\iNew TestInformation\\nContains one more line."
	      << std::endl;
      }
      if (j > 5)
//         theTreeInterface->PaintNode(j-5,4);
         std::cout << 'P' << ' ' << (j-5) << ' ' << 4 << std::endl;
      if (j > 1)
//         theTreeInterface->PaintNode(j-1,6);
         std::cout << 'P' << ' ' << (j-1) << ' ' << 4 << std::endl;
   }      

   delete[] father;

//   theTreeInterface->FinishAlgorithm(TRUE,0);
   std::cout << "#END_OF_OUPUT" << std::endl;
}



   
   
