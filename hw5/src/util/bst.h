/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BST_H
#define BST_H

#include <cassert>
#include <vector>
#include<iostream>

using namespace std;

template <class T> class BSTree;

// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class BSTreeNode
{
   // TODO: design your own class!!
    friend class BSTree<T>;
    friend class BSTree<T>::iterator;

    BSTreeNode(const T& d, int h = 1, BSTreeNode<T>* l = 0, BSTreeNode<T>* r = 0):
        _data(d), height(h), _left(l), _right(r) {}

    T _data;
    int height;
    BSTreeNode<T>* _left;
    BSTreeNode<T>* _right;
};


template <class T>
class BSTree
{
   // TODO: design your own class!!
public:
    BSTree() { _root = 0; }
    ~BSTree() {}

    class iterator {
        friend class BSTree;
    public:
        iterator(BSTreeNode<T>* root, bool end = false) : _root(root) { if(!end && root) findleft(root); }
        iterator(const iterator& i) : _node(i._node), _left(i._left), _root(i._root) {}
        ~iterator() {}

        const T& operator * () const { return *(this); }
        T& operator * () { return _node.back()->_data; }
        iterator& operator ++ () {
            if(_node.empty()) return *(this);
            BSTreeNode<T>* top = _node.back();
            if(top->_right){
                _left.push_back(0);
                findleft(top->_right);
            }
            else{
                while(!_left.empty() && !_left.back()){
                    _node.pop_back();
                    _left.pop_back();
                }
                if(!_left.empty()){
                    _node.pop_back();
                    _left.pop_back();
                }
                else
                    _node.pop_back();
            }
            return *(this);
        }
        iterator operator ++ (int) {
            if(_node.empty()) return *(this);
            iterator temp = *(this);
            BSTreeNode<T>* top = _node.back();
            if(top->_right){
                _left.push_back(0);
                findleft(top->_right);
            }
            else{
                while(!_left.empty() && !_left.back()){
                    _node.pop_back();
                    _left.pop_back();
                }
                if(!_left.empty()){
                    _node.pop_back();
                    _left.pop_back();
                }
                else
                    _node.pop_back();
            }
            return temp;
        }
        iterator& operator -- () {
            if(_node.empty())
                findright(_root);
            else{
                BSTreeNode<T>* top = _node.back();
                if(top->_left){
                    _left.push_back(1);
                    findright(top->_left);
                }
                else{
                    while(!_left.empty() && _left.back()){
                        _node.pop_back();
                        _left.pop_back();
                    }
                    if(!_left.empty()){
                        _node.pop_back();
                        _left.pop_back();
                    }
                    else
                        _node.pop_back();
                }
            }
            return *(this);
        }
        iterator operator -- (int) {
            iterator temp = *(this);
            if(_node.empty())
                findright(_root);
            else{
                BSTreeNode<T>* top = _node.back();
                if(top->_left){
                    _left.push_back(1);
                    findright(top->_left);
                }
                else{
                    while(!_left.empty() && _left.back()){
                        _node.pop_back();
                        _left.pop_back();
                    }
                    if(!_left.empty()){
                        _node.pop_back();
                        _left.pop_back();
                    }
                    else
                        _node.pop_back();
                }
            }
            return temp;
        }

        iterator& operator = (const iterator& i) {
            _node = i._node;
            _left = i._left;
            _root = i._root;
            return *(this);
        }

        bool operator != (const iterator& i) const {
            if (_node.empty() && i._node.empty())
                return false;
            else if (!_node.empty() && !i._node.empty())
                return _node.back() != i._node.back();
            return true;
        }
        bool operator == (const iterator& i) const {
            if (_node.empty() && i._node.empty())
                return true;
            else if (!_node.empty() && !i._node.empty())
                return _node.back() == i._node.back();
            return false;
        }
    private:
        vector<BSTreeNode<T>*> _node;
        vector<bool> _left;
        BSTreeNode<T>* _root;

        void findleft(BSTreeNode<T>* r){
            _node.push_back(r);
            r = r->_left;
            while(r){
                _left.push_back(1);
                _node.push_back(r);
                r = r->_left;
            }
        }
        void findright(BSTreeNode<T>* r){
            _node.push_back(r);
            r = r->_right;
            while(r){
                _left.push_back(0);
                _node.push_back(r);
                r = r->_right;
            }
        }
    };

    iterator begin() const { return iterator(_root); }
    iterator end() const { return iterator(_root, true); }
    bool empty() const { return _root == 0; }
    size_t size() const {
        size_t size = 0;
        iterator li = begin();
        for (; li != end(); ++li)
            ++size;
        return size;
    }
    void insert(const T& x){ _root = insertw(x, _root); }
    BSTreeNode<T>* insertw(const T& x, BSTreeNode<T>* root) {
        if(!root) return (new BSTreeNode<T>(x));
        if(x < root->_data)
            root->_left = insertw(x, root->_left);
        else
            root->_right = insertw(x, root->_right);

        root->height = 1 + max(height(root->_left), height(root->_right));
        int bala = balance(root);
        if(bala > 1 && x < root->_left->_data)
            return rightRotate(root);
        if(bala < -1 && x >= root->_right->_data)
            return leftRotate(root);
        if(bala > 1 && x >= root->_left->_data){
            root->_left = leftRotate(root->_left);
            return rightRotate(root);
        }
        if(bala < -1 && x < root->_right->_data){
            root->_right = rightRotate(root->_right);
            return leftRotate(root);
        }
        return root;
    }
    void pop_front() {
        if(empty()) return;
        BSTreeNode<T>* head = _root;
        while(head->_left)
            head = head->_left;
        _root = del(_root, head->_data);
    }
    void pop_back() {
        if(empty()) return;
        BSTreeNode<T>* tail = _root;
        while(tail->_right)
            tail = tail->_right;
        _root = del(_root, tail->_data);
    }

    bool erase(iterator pos) {
        if(empty()) return false;
        _root = del(_root, *pos);
        return true;
    }
    bool erase(const T& x) {
        T a = x;
        if(find(_root, a)){
            _root = del(_root, a);
            return true;
        }
        return false;
    }

    void clear() {
        for(iterator it = begin(); it != end(); ++it)
            delete it._node.back();
        _root = 0;
    }

    void sort() const {}

    void print() const{
        if(_root)
            printv(_root, 0);
    }
private:
    BSTreeNode<T>* _root;

    BSTreeNode<T>* minn(BSTreeNode<T>* node){
        if(node->_left == 0)
            return node;
        return minn(node->_left);
    }
    bool find(BSTreeNode<T>* root, T& x){
        if(!root)
            return false;
        if(x < root->_data)
            return find(root->_left, x);
        if(x > root->_data)
            return find(root->_right, x);
        return true;
    }
    BSTreeNode<T>* del(BSTreeNode<T>* root, T& x){
        if(!root) return root;
        if(x < root->_data)
            root->_left = del(root->_left, x);
        else if(x > root->_data)
            root->_right = del(root->_right, x);
        else{
            if(root->_left && root->_right){
                BSTreeNode<T>* temp = minn(root->_right);
                root->_data = temp->_data;
                root->_right = del(root->_right, temp->_data);
            }
            else if (root->_right){
                BSTreeNode<T>* temp = root;
                root = root->_right;
                delete temp;
            }
            else if (root->_left){
                BSTreeNode<T>* temp = root;
                root = root->_left;
                delete temp;
            }
            else{
                delete root;
                root = 0;
                return root;
            }
        }

        root->height = 1 + max(height(root->_left), height(root->_right));
        int bala = balance(root);

        if(bala > 1 && balance(root->_left) >= 0)
            return rightRotate(root);
        if(bala < -1 && balance(root->_right) <= 0)
            return leftRotate(root);
        if(bala > 1 && balance(root->_left) < 0){
            root->_left = leftRotate(root->_left);
            return rightRotate(root);
        }
        if(bala < -1 && balance(root->_right) > 0){
            root->_right = rightRotate(root->_right);
            return leftRotate(root);
        }
        return root;
    }
    int max(int i, int j){
        return (i > j)? i : j;
    }
    int balance(BSTreeNode<T>* n){
        if(!n)
            return 0;
        return height(n->_left) - height(n->_right);
    }
    int height(BSTreeNode<T>* n){
        if(!n)
            return 0;
        return n->height;
    }
    BSTreeNode<T>* rightRotate(BSTreeNode<T>* root){
        BSTreeNode<T>* x = root->_left;
        BSTreeNode<T>* y = x->_right;

        x->_right = root;
        root->_left = y;

        root->height = 1 + max(height(root->_left), height(root->_right));
        x->height = 1 + max(height(x->_left), height(x->_right));

        return x;
    }
    BSTreeNode<T>* leftRotate(BSTreeNode<T>* root){
        BSTreeNode<T>* x = root->_right;
        BSTreeNode<T>* y = x->_left;

        x->_left = root;
        root->_right = y;

        root->height = 1 + max(height(root->_left), height(root->_right));
        x->height = 1 + max(height(x->_left), height(x->_right));

        return x;
    }
    void printv(BSTreeNode<T>* n, int s) const{
        int space = s;
        while(space--)
            cout << "  ";
        if(!n){
            cout << "[0]" << endl;
            return;
        }
        cout << n->_data << endl;
        ++s;
        printv(n->_left, s);
        printv(n->_right, s);
    }
};
#endif // BST_H
