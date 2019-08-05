#ifndef GILIST_H
#define GILIST_H


template <class T> class Element
{
   public:

      T	       item;
      int      key;	// key has to be a positiv value, e.g. a Node-Number
      Element *next;

      Element(T con, int k = -1) { item = con; next = NULL; key = k;}
};

template <class T> class Stack
{
   private:

      Element<T>  *_top;
      int	  _numElements;

   public:

      Stack()
      {
	 _numElements = 0;
	 _top = NULL;
      }

      ~Stack()
      {
	 while(_top) {
	    Element<T> *next = _top->next;
	    delete _top;
	    _top = next;
	 }
      }

      void push(T item)
      {
	 Element<T> *n = new Element<T>(item, _numElements);
	 n->next = _top;
	 _top = n;
	 _numElements++;
      }

      T pop()
      {
	 if (_top) {
	    T value = _top->item;
	    Element<T> *next = _top->next;
	    delete _top;
	    _top = next;
	    _numElements--;
	    return value;
	 }
	 return ((T) 0);
      }

      Boolean isEmpty()
      {
	 return (_top == NULL);
      }

      int numElements()
      {
	 return _numElements;
      }

      Element<T> *top() { return _top; }
};

template <class T> class Queue
{
   private:

      Element<T> *_front;
      Element<T> *_end;
      int         _numElements;

   public:

      Queue()
      {
	 _numElements = 0;
	 _front = _end = NULL;
      }

      ~Queue()
      {
	 while(_front) {
	    Element<T> *next = _front->next;
	    delete _front;
	    _front = next;
	 }
      }

      void enqueue(T item)
      {
	 Element<T> *n = new Element<T>(item, _numElements);
	 if (_front == NULL) _front = n;
	 if (_end == NULL) _end = n;
	 else { _end->next = n; _end = n; }
	 _numElements++;
      }

      T dequeue()
      {
	 if (_front) {
	    T value = _front->item;
	    Element<T> *next = _front->next;
	    if (_end == _front)
	       _end = NULL; // last element and next = NULL
	    delete _front;
	    _front = next;
	    _numElements--;
	    return value;
	 }
	 return ((T) 0);
      }

      Boolean isEmpty()
      {
	 return (_front == NULL);
      }

      int numElements() { return _numElements; }

      Element<T> *front() { return _front; }
      Element<T> *end() { return _end; }
};

template <class T> class AdjazentList
{
   private:

      Element<T>  **_list;
      int	 _numElements;
      Boolean	 _directed;

   public:

      AdjazentList(int n, Boolean directed = False)
      {
	 _directed = directed;
	 _numElements = n;
	 _list = new Element<T>*[n+1];
	 
	 for (int i = 0; i <= n; i++)
	    _list[i] = NULL;
      }

      ~AdjazentList()
      {
	 Element<T> *actual;
	 Element<T> *theNext;
	 
	 for (int i = 0; i <= _numElements; i++)
	    if (actual = _list[i])
	       do {
		  theNext = actual->next;
		  delete actual;
		  actual = theNext;
	       } while(theNext);

	 delete[] _list;
      }
      
      Status insertContent(int pos, T item, int key = -2)
      {
	 if (pos < 0 || pos > _numElements) return ERROR;

	 Element<T> *actual;
	 Element<T> *thePrev;

	 if (!_directed && key >=0 && key <= _numElements)
	    if (actual = _list[key])
	       do {
		  thePrev = actual;
		  if (pos == actual->key || actual->item == item) return ERROR; 
		  actual = actual->next;
	       } while (actual);

	 if (actual = _list[pos])
	    do {
	       thePrev = actual;
	       if (actual->item == item || key == actual->key) return ERROR;
	       actual = actual->next;
	    } while(actual);

	 if (key == -2) key = -1;
	 actual = new Element<T>(item, key);

	 if (_list[pos])
	    thePrev->next = actual;
	 else
	    _list[pos] = actual;

	 return SUCCESS;
      }

      Status deleteContent(int pos, T item, int key = -2)
      {
	 if (pos < 0 || pos > _numElements) return ERROR;

	 Status	     status = ERROR;
	 Element<T> *actual;
	 Element<T> *thePrev;

	 if (!_directed && key >=0 && key <= _numElements)
	    if (actual = _list[key])
	       do {
		  if (actual->item == item || pos == actual->key) {
		     if (actual == _list[key])
			_list[key] = actual->next;
		     else
			thePrev->next = actual->next;

		     delete actual;
		     status = SUCCESS;
		     break;
		  }

		  thePrev = actual;
		  actual = actual->next;
	       } while(actual);

	 if (actual = _list[pos])
	    do {
	       if (actual->item == item || key == actual->key) {
		  if (actual == _list[pos])
		     _list[pos] = actual->next;
		  else
		     thePrev->next = actual->next;

		  delete actual;
		  return SUCCESS;
	       }

	       thePrev = actual;
	       actual = actual->next;
	    } while(actual);

	 return status;
      }

      T find(int pos, int key)
      {
	 Element<T> *actual;

	 actual = _list[pos];
	 while(actual)
	    if (actual->key == key) return actual->item;
	    else actual = actual->next;

	 if (!_directed) {
	    actual = _list[key];
	    while(actual)
	       if (actual->key == pos) return actual->item;
	       else actual = actual->next;
	 }

	 return (T) 0;
      }

      void print()
      {
	 Element<T> *actual;
	 
	 for (int i = 1; i <= _numElements; i++)
	    if (actual = _list[i])
	       do {
		  printf("Liste %d, Element: %d\n", i, (int) actual->item);
		  actual = actual->next;
	       } while (actual);
	    else
	       printf("Liste %d ist leer\n", i);

      }

      Stack<T> *getStack()
      {
	 Stack<T> *stack = new Stack<T>;
	 Element<T> *actual;
	 
	 for (int i = 1; i <= _numElements; i++)
	    if (actual = _list[i])
	       do {
		  stack->push(actual->item);
		  actual = actual->next;
	       } while (actual);

	 return stack;
      }

      int numElements() { return _numElements; }

      Boolean directed() { return _directed; }

};

#endif
