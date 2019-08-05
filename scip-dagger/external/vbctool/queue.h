/************************************************************************

         Filename   :   queue.h

         Version    :   09.1994

	 Author     :   Sebastian Leipert

	 Language   :   C++

************************************************************************/ 


// File containing a class queue, a first-in first-out stack.


#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include "def.glb"



template<class T>
class T_elem{
 public:
   T      key;
   T_elem *next;
   T_elem *pre;
};



template <class T>
class queue {
     T_elem<T>  *first;     // pointer to the first element of the queue
     T_elem<T>  *last;      // pointer to the last element of the queue
     int        size;       // number of elements in the queue


  public:
     queue()                // constructor of the class queue
     {
        first = NULL;
        last = NULL;
        size = 0;
     };


     ~queue()               // destructor of the class queue
     {
        T_elem<T>  *helpptr;

        while (last != NULL)
        {  helpptr = last;
           last = last->next;
           delete helpptr;
        }
     };


     int queue_empty()      // true if the queue is empty, false otherwise
     {
        if (size == 0)
           return TRUE;
        else
           return FALSE;
     };

     
     void empty_queue()     // empties the queue
     {
        T_elem<T>   *helpptr;

        while (last != NULL)
        {  helpptr = last;
           last = last->next;
           delete helpptr;
        }
        first = NULL;
        size = 0;
     };


     void enqueue(T m)      // pushes an elemt T on to the last position of
                            // the stack
     {
        T_elem<T>   *helpptr;

        size++;
        helpptr = new T_elem<T>;
        helpptr->key = m;
        helpptr->next = NULL;
        if (last == NULL)
        {  last = helpptr;
           first = helpptr;
           helpptr->pre = NULL;
        }
        else
        {  first->next = helpptr;
           helpptr->pre = first;
           first = helpptr;
        }
     };


     T dequeue()            // gets the first element out of the stack
     {
        T          helpelem;
        T_elem<T>  *helpptr;

        if (last != NULL)
        {  if (first == last)
              first = NULL;
           helpelem = last->key;
           helpptr = last;
           last = last->next;
           if (last != NULL)
              last->pre = NULL;
           size--;
           delete helpptr;
           return helpelem;
        }
        else
        {  printf("template queue: queue empty. wrong access\n");
           return 0;
        }
     };

     
     T front()              // reads the first element of the stack without
                            // removing i
     {
        if (last != NULL)
           return last->key;
        else
        {  printf("template queue: queue empty. wrong access when reading element.\n");
           return 0;
        }
     };

     
     int queue_size()       // gets the number of elements stored in the
			    // stack
     {
        return size;
     };
};



/*
template<class T> queue<T>::queue()
{
   first = NULL;
   last = NULL;
   size = 0;
}



template<class T> queue<T>::~queue()
{
  T_elem<T>  *helpptr;

  while (last != NULL)
  {  helpptr = last;
     last = last->next;
     delete helpptr;
  }
}




template<class T> int queue<T>::queue_empty()
{
   if (size == 0)
      return TRUE;
   else
      return FALSE;
}



template<class T> void queue<T>::empty_queue()
{
   T_elem<T>   *helpptr;

   while (last != NULL)
   {  helpptr = last;
      last = last->next;
      delete helpptr;
   }
   first = NULL;
   size = 0;
}



template<class T> void queue<T>::enqueue(T m)
{
  T_elem<T>   *helpptr;

  size++;
  helpptr = new T_elem<T>;
  helpptr->key = m;
  helpptr->next = NULL;
  if (last == NULL)
  {  last = helpptr;
     first = helpptr;
     helpptr->pre = NULL;
  }
  else
  {  first->next = helpptr;
     helpptr->pre = first;
     first = helpptr;
  }
}



template<class T> T queue<T>::dequeue()
{
    T          helpelem;
    T_elem<T>  *helpptr;

    if (last != NULL)
    {  if (first == last)
          first = NULL;
       helpelem = last->key;
       helpptr = last;
       last = last->next;
       if (last != NULL)
          last->pre = NULL;
       size--;
       delete helpptr;
       return helpelem;
    }
    else
    {  printf("template queue: queue empty. wrong access\n");
       return 0;
    }
}

    
    
template<class T> T queue<T>::front()
{
   if (last != NULL)
      return last->key;
   else
   {  printf("template queue: queue empty. wrong access when reading element.\n");
      return 0;
   }
}


template<class T> int queue<T>::queue_size()
{
   return size;
}

*/


#endif


