/****************************************************************************
  FileName     [ dlist.h ]
  PackageName  [ util ]
  Synopsis     [ Define doubly linked list package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef DLIST_H
#define DLIST_H

#include <cassert>

template <class T> class DList;

// DListNode is supposed to be a private class. User don't need to see it.
// Only DList and DList::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class DListNode
{
   friend class DList<T>;
   friend class DList<T>::iterator;

   DListNode(const T& d, DListNode<T>* p = 0, DListNode<T>* n = 0):
      _data(d), _prev(p), _next(n) {}

   // [NOTE] DO NOT ADD or REMOVE any data member
   T              _data;
   DListNode<T>*  _prev;
   DListNode<T>*  _next;
};


template <class T>
class DList
{
public:
   // TODO: decide the initial value for _isSorted
   DList() {
      _head = new DListNode<T>(T());
      _head->_prev = _head->_next = _head; // _head is a dummy node
   }
   ~DList() { clear(); delete _head; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class DList;

   public:
      iterator(DListNode<T>* n= 0): _node(n) {}
      iterator(const iterator& i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return *(this); }
      T& operator * () { return _node->_data; }
      iterator& operator ++ () {
          _node = _node->_next;
          return *(this);
      }
      iterator operator ++ (int) {
          iterator temp = *(this);
          _node = _node->_next;
          return temp;
      }
      iterator& operator -- () {
          _node = _node->_prev;
          return *(this);
      }
      iterator operator -- (int) {
          iterator temp = *(this);
          _node = _node->_prev;
          return temp;
      }

      iterator& operator = (const iterator& i) {
          _node = i._node;
          return *(this);
      }

      bool operator != (const iterator& i) const { return _node != i._node; }
      bool operator == (const iterator& i) const { return _node == i._node; }

   private:
      DListNode<T>* _node;
   };

   // TODO: implement these functions
   iterator begin() const { return _head; }
   iterator end() const { return _head -> _prev; }
   bool empty() const { return _head -> _next == _head; }
   size_t size() const {
       size_t size = 0;
       iterator li = begin();
       for (; li != end(); ++li)
            ++size;
       return size;
   }

   void push_back(const T& x) {
       DListNode<T>* e = _head -> _prev;
       DListNode<T>* t = new DListNode<T> (x, e -> _prev, e);
       if (!empty())
          e -> _prev -> _next = t;
       else{
          _head = t;
          e -> _next = _head;
       }
       e -> _prev = t;

   }
   void pop_front() {
       if (empty()) return;
       DListNode<T>* e = _head -> _prev;
       DListNode<T>* t = _head -> _next;
       t -> _prev = e;
       e -> _next = t;
       delete _head;
       _head = t;
   }
   void pop_back() {
       if (empty()) return;
       DListNode<T>* e = _head -> _prev;
       DListNode<T>* t = e -> _prev;
       if (t == _head)
          _head = e;
       t -> _prev -> _next = e;
       e -> _prev = t -> _prev;
       delete t;
   }

   // return false if nothing to erase
   bool erase(iterator pos) {
       if(empty())
          return false;
       else{
          if (pos._node == _head)
            _head = pos._node -> _next;
          pos._node -> _prev -> _next = pos._node -> _next;
          pos._node -> _next -> _prev = pos._node -> _prev;
          delete pos._node;
          return true;
       }
   }
   bool erase(const T& x) {
       DListNode<T>* t = _head;
       DListNode<T>* e = _head -> _prev;
       while(t != e){
           if(t -> _data == x){
               if(t == _head)
                  _head = t -> _next;
               t -> _prev -> _next = t -> _next;
               t -> _next -> _prev = t -> _prev;
               delete t;
               return true;
           }
           t = t -> _next;
       }
       return false;
   }

   void clear() {
       DListNode<T>* t = _head -> _next;
       DListNode<T>* d = _head;
       DListNode<T>* e = _head -> _prev;
       while (d != e){
           delete d;
           d = t;
           t = t -> _next;
       }
       e->_next = e->_prev = e;
       _head = e;
   }  // delete all nodes except for the dummy node

   void sort() const {
       if(empty()) return;
       iterator i = begin(); ++i;
       iterator j, k;
       for(; i != end(); ++i){
           k = j = i;
           --k;
           while(j != begin() && *k > *j){
               T temp = *j;
               *j = *k;
               *k = temp;
               --j;--k;
           }
       }
   }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   DListNode<T>*  _head;     // = dummy node if list is empty
   mutable bool   _isSorted; // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] helper functions; called by public member functions
};

#endif // DLIST_H
