/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>

using namespace std;

//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//
template <class Data>
class HashSet
{
public:
   HashSet(size_t b = 0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashSet() { reset(); }

   // TODO: implement the HashSet<Data>::iterator
   // o An iterator should be able to go through all the valid Data
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashSet<Data>;

   public:
      iterator(const HashSet<Data>* d, size_t b, size_t s):_data((HashSet<Data>*)d), _bkt(b), _slt(s){}
      const Data& operator * () const { return (*_data)[_bkt][_slt]; }
      iterator& operator ++ () {
         size_t n = _data->_numBuckets;
         if(_slt + 1 < (*_data)[_bkt].size())
            ++_slt;
         else{
            ++_bkt;
            _slt = 0;
            for(; _bkt <= n; ++_bkt){
               if(_bkt == n){
                  _bkt = _slt = -1;
                  break;
               }
               if(!(*_data)[_bkt].empty())
                  break;
            }
         }
         return (*this);
      }
      iterator operator ++ (int){
         iterator temp = *this;
         ++(*this);
         return temp;
      }
      iterator& operator -- (){
         if(_slt > 0)
            --_slt;
         else{
            size_t b = _bkt;
            if(b == -1)
               b = _data.size();
            while(b > 0){
               --b;
               if(_data[b].size()){
                  _bkt = b;
                  _slt = _data[b].size()-1;
                  break;
               }
            }
         }
         return (*this);
      }
      iterator operator -- (int){
         iterator temp = *this;
         --(*this);
         return temp;
      }
      iterator& operator = (const iterator& i){
         _data = i._data; _bkt = i._bkt; _slt = i._slt;
         return (*this);
      }
      bool operator == (const iterator& i) const {
         if(_data == i._data && _bkt == i._bkt && _slt == i._slt)
            return true;
         return false;
      }
      bool operator != (const iterator& i) const {
         if(_data != i._data || _bkt != i._bkt || _slt != i._slt)
            return true;
         return false;
      }
   private:
      HashSet<Data>* _data;
      size_t _bkt;
      size_t _slt;
   };

   void init(size_t b) { _numBuckets = b; _buckets = new vector<Data>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<Data>& operator [] (size_t i) { return _buckets[i]; }
   const vector<Data>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
    iterator begin() const {
        iterator it(this, 0, 0);
        if(_buckets[0].empty())
            ++it;
        return it;
    }
   // Pass the end
    iterator end() const { return iterator(this, -1, -1); }
   // return true if no valid data
    bool empty() const {
        if(size())
            return false;
        return true;
    }
   // number of valid data
    size_t size() const {
        size_t s = 0;
        for(size_t b = 0; b < _numBuckets; ++b)
            s += _buckets[b].size();
        return s;
    }

   // check if d is in the hash...
   // if yes, return true;
   // else return false;
    bool check(const Data& d) const {
        size_t b = bucketNum(d);
        size_t n = _buckets[b].size();
        for (size_t s = 0; s < n; ++s){
            if(d == _buckets[b][s])
                return true;
        }
        return false;
    }

   // query if d is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
    bool query(Data& d) const {
        size_t b = bucketNum(d);
        size_t n = _buckets[b].size();
        for (size_t s = 0; s < n; ++s){
            if(d == _buckets[b][s]){
                d = _buckets[b][s];
                return true;
            }
        }
        return false;
    }

   // update the entry in hash that is equal to d (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
    bool update(const Data& d) {
        size_t b = bucketNum(d);
        size_t n = _buckets[b].size();
        for (size_t s = 0; s < n; ++s){
            if(d == _buckets[b][s]){
                _buckets[b][s] = d;
                return true;
            }
        }
        _buckets[bucketNum(d)].push_back(d);
        return false;
    }

   // return true if inserted successfully (i.e. d is not in the hash)
   // return false is d is already in the hash ==> will not insert
    bool insert(const Data& d) {
        if(check(d))
            return false;
        _buckets[bucketNum(d)].push_back(d);
        return true;
    }

   // return true if removed successfully (i.e. d is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
    bool remove(const Data& d) {
        size_t b = bucketNum(d);
        size_t n = _buckets[b].size();
        for(size_t s = 0; s < n; ++s){
            if(d == _buckets[b][s]){
                _buckets[b].erase(_buckets[b].begin() + s);
                return true;
            }
        }
        return false;
    }

private:
   // Do not add any extra data member
   size_t            _numBuckets;
   vector<Data>*     _buckets;

   size_t bucketNum(const Data& d) const {
      return (d() % _numBuckets); }
};

#endif // MY_HASH_SET_H
