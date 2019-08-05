/*****************************************************************

            Filename    :  tree_input.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Contains all friends of the class tree.
	                   All Friends are used for reading File-Input.
			   Some of them are used as well for setting
			   new information of nodes, so they have
			   as well the abillity of reading String-Input.
			   
******************************************************************/


#include <iostream>
#include <fstream>
#include <cstring>
#include "tree.h"
#include "def.glb"

void get_info(char* input_ptr,char *information,char *shortinfo,
             tree &T);

int pushArray(char *infoarray,char infochar,int *count,int check,
              int warning);


/************************************************************************
                     operator>>
*************************************************************************/


char *operator>>(char *filename,tree &T)

   // This overloaded operator is a friend of the class tree.h.
   // It reads the Input from a file and stores all necessary
   // information in tree.
{
   int   head = 0;
   int   edgeCount = 0;                // Number of edges.
   int   position = 0;
   char  ident;                        // Line-identifier.
   char  inputline[INFOSIZE];          // Buffer for reading a line.
   char  *input_ptr;

   std::ifstream input(filename,std::ios::in);

   if (T.root != NULL)                 // There has been a tree stored in the
                                       // class before. Clean the rubbish.
      T.clean_tree();
   
   input.getline(inputline,INFOSIZE);  // Get the first line. It was used as
                                       // an identifier in the class
				       // TreeInterface.
      position = input.tellg();
   input.getline(inputline,INFOSIZE);  // Get the second line. It was used as
                                       // an identifier in the class
				       // TreeInterface.
      position = input.tellg();
   input.getline(inputline,INFOSIZE);  // Get the third line. It was used as
                                       // an identifier in the class
				       // TreeInterface.
      position = input.tellg();
   input.getline(inputline,INFOSIZE);  // Get the fourth line. 
   input_ptr = inputline;
   position = input.tellg();



   if (!strcmp(input_ptr,FOURTHLINE_1))
                                       // If the file contains standard
				       // information, continue.
   {
      T.standard = TRUE;
      T.root = allocate_tree(filename,T,position);
      input.seekg(position,std::ios::beg);

      int colour;       
      int i = 1;

      if (T.root != NULL)
      {
	                               // Get the next line-identifier      
         while ((input >> ident) && (i != 0) )
	                               // if i == 0 file is corrupt
         {
            if (ident == 'e')          // Line describes an edge.
	                               // Enter edge.
	       {
                  i = enter_edge(input,T,i);
//	          std::cout << "i = " << i << std::endl;
	       }
	    
            else if (ident == 'n')     // Line describes the information
	                               // of a node. Enter information.
	    {
	       char infochar;
	    
               input >> head;          // Get the node number.
	       if ( (infochar = input.peek()) == ' ')
	                               // If the first char of the information
				       // is a blanc, skip it.
	          infochar = input.get();
	                               // Get the rest of the line.
               input.getline(inputline,INFOSIZE);
	       input_ptr = inputline;
	                               // Place the line as information
				       // in node.
	       set_node_info(head,input_ptr,T);
	    }
            else if (ident == 'c')     // Line describes the colour of
	                               // a node. Enter colour.
	    {
	       input >> head;
	       input >> colour;
	       if (colour < 1 || colour > 20)
	          colour = STANDARDCOLOUR;
	       T.set_node_colour(head,colour);
	       T.set_under_colour(head,colour);
	    }
	    else
	       i = FALSE;
	 }
//	 std::cout << "i = " << i << std::endl;
//	 std::cout << "Root: " << T.root->get_nmber() << std::endl;
         if (i == 0)                   // File is corrupt
	    T.clean_tree();
      }
   }



   else if (!strcmp(input_ptr,FOURTHLINE_2))
                                       // The file contains a plain tree
				       // without any information. Get
				       // the edges and place them in
				       // the tree.
   {
      T.standard = FALSE;
      T.root = allocate_tree(filename,T,position);
      input.seekg(position,std::ios::beg);
      input.tellg();
      if (T.root != NULL)
      {
         int i = 1;
	 while ( (i > 0) && (i <= T.edge_nb) )
	 {
            i = enter_edge(input,T,i);
	    if (i == 0)                // File is corrupt.
	       T.clean_tree();
	 }
      }
   }


}






/************************************************************************
                   allocate_tree
*************************************************************************/


node* allocate_tree(char* filename,tree &T,int &position)

   // Allocate the size of the tree.
{
   int   corrupt = FALSE;              // If TRUE, the file is corrupt.
   int   edgeCount = 0;
   char  inputline[INFOSIZE];          // Buffer for reading a line.
   char  *input_ptr;
   char  ident;

   std::ifstream input(filename,std::ios::in);
   
   input.seekg(position,std::ios::beg);

   input.getline(inputline,INFOSIZE);  // Get the fifth line, to check if
                                       // the number of nodes is available.
   input_ptr = inputline;
   if (!strcmp(input_ptr,FIFTHLINE_2))
   {
      input >> T.node_nb >> T.edge_nb;
      position = input.tellg();

      T.node_array = new node*[T.node_nb+1];
      T.edge_array = new edge[T.node_nb];
    
      for (int i = 1;  i <= T.node_nb; i++)
         T.node_array[i] = new node(i);
      return T.node_array[1];
   }
   else if (!strcmp(input_ptr,FIFTHLINE_1))
   {
      position = input.tellg();
   
      if (T.standard)                  // The tree contains standard
                                       // information.
      {
         while (input >> ident)
         {
            if (ident == 'e')
	       edgeCount++;
	    else if ((ident != 'n') && (ident != 'c'))
	       corrupt = TRUE;
            input.getline(inputline,INFOSIZE);
         }
      }
      else                             // The tree contains no information.
      {
         int help = 0;
         while (input >> help)
	 {
	    edgeCount++;
	    input >> help;
	 }
      }
      if (!corrupt)
      {
         T.edge_nb = edgeCount;
         T.node_nb = edgeCount+1;
         T.node_array = new node*[T.node_nb+1];
         T.edge_array = new edge[T.node_nb];
    
         for (int i = 1;  i <= T.node_nb; i++)
            T.node_array[i] = new node(i);
         T.root = T.node_array[1];
         return T.node_array[1];
      }
      else
         return NULL;
   }
   return NULL;
}







/************************************************************************
                       enter_edge
*************************************************************************/


int   enter_edge(std::ifstream &input,tree &T,int i)

   // Places the next edge that is listed in the file into the tree.
   // The integer i is used to place the edge into the correct
   // position of edge_array, an array used by the tree T.
{
   int   head,tail;


   input >> tail >> head;
   if ( (0 < tail) && (tail <= T.get_node_nb()) &&
        (0 < head) && (head <= T.get_node_nb()))
   {
      if (tail < head)
                                       // Place it into the tree.
      {
         T.node_array[head]->set_parent(T.node_array[tail]);
         T.introduce_child(T.node_array[tail],T.node_array[head]);
         T.edge_array[i].set_first(tail);
         T.edge_array[i].set_second(head);
         return ++i;
      }
   }
   return 0;
}






/************************************************************************
                     set_node_info
*************************************************************************/


int set_node_info(int head,char* input_ptr,tree &T)

   // This procedure is used by the >> operator, friend of class Tree.
   // It is as well used by the TreeInterface procedure SetNodeInfo.
   // It will place information stored in a file or in an array
   // of characters in a node. The information is read character by
   // character, in order to find identifiers.
 
{
   if ((head >0) && (head <= T.get_node_nb()))
   {
      char information[INFOSIZE];      // Array containing normal information.
      char shortinfo[SHORTSIZE];       // Array containing important information. 

      get_info(input_ptr,information,shortinfo,T);
                                       // Set the information of the node.
      T.node_array[head]->set_information(information);
      T.node_array[head]->set_shortinfo(shortinfo);

      return TRUE;
   }
   else
      return FALSE;

}



/************************************************************************
                     add_node_info
*************************************************************************/


int add_node_info(int head,char* input_ptr,tree &T)

   // This procedure is used by the TreeInterface procedure AddNodeInfo.
   // It will place information stored in a file or in an array
   // of characters in a node. The information is read character by
   // character, in order to find identifiers.
 
{
   if ((head >0) && (head <= T.get_node_nb()))
   {
      char information[INFOSIZE];      // Array containing normal information.
      char shortinfo[SHORTSIZE];       // Array containing important information. 

      get_info(input_ptr,information,shortinfo,T);
                                       // Set the information of the node.
      T.node_array[head]->add_information(information);
      T.node_array[head]->add_shortinfo(shortinfo);

      return TRUE;
   }
   else
      return FALSE;

}



/************************************************************************
                         get_info
*************************************************************************/


void get_info(char* input_ptr,char *information,char *shortinfo,
             tree &T)

{

   char input[INFOSIZE];            // Array containing all information.
   char infochar;                   // Needed to check for identifiers
   char comparechar;                // Needed to check for the end.
   int  endline_found = FALSE;
   int  important = FALSE;
   int  lenght = strlen(input_ptr);
   int  i = 0;
   int  j = 0;
   int  k = 0;
   int  Warning = 0;

   
   strcpy(input,input_ptr);

   comparechar = '\0';

   while ( (infochar = input[i++]) != comparechar)
                                       // While not reached the end of the
				       // information, read the next character.
   {	    
      if (endline_found)
	                               // We have found the charcter: \
				       // We have to check what character
				       // comes next in order to analyse
				       // this information.
      {
	 switch (infochar)
	 {
	    case 'n':       infochar = '\n';
	                               // The user has defined an EndOfLine.
		            break;

  	    case 't':       infochar = '\t';
	                               // The user has defined a tabulator.
		            break;

	    case 'i':       infochar = '\a';
	                               // The user has defined an important
				       // information.
		            if (!important)
			               // Important information starts here.
		               important = TRUE;
			    else
			               // Important information is finished.
			       important = FALSE;
			    break;

  	 }
	 if (!important && infochar != '\a')
            Warning = pushArray(information,infochar,&j,INFOSIZE,Warning);
	 if (important && infochar != '\a')
            Warning = pushArray(shortinfo,infochar,&k,SHORTSIZE,Warning);
	 endline_found = FALSE;
      }
      else if (infochar == '\\')
	 endline_found = TRUE;
      else if (!important)
         Warning = pushArray(information,infochar,&j,INFOSIZE,Warning);
      else if (infochar != '\a')
         Warning = pushArray(shortinfo,infochar,&k,SHORTSIZE,Warning);
   }
                                       // Close the arrays that contain now
				       // the information and the important
				       // information in a correct way.
   information[j] = '\0';
   shortinfo[k] = '\0';
}




/************************************************************************
                         pushArray
*************************************************************************/


int pushArray(char *infoarray,char infochar,int *count,int check,int warning)
{

   if ((*count) < (check-1))
   {
      infoarray[(*count)++] = infochar;
      return 0;
   }
   else if (!warning)
   {
      printf("WARNING! Information Array bounds violated!\n");
      return (++warning);
   }
}

