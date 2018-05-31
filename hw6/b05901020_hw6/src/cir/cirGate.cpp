/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr *cirMgr;

// TODO: Implement memeber functions for class(es) in cirGate.h
size_t CirGate::_globalRef = 0;
/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
    cout << "==================================================" << endl;
    ostringstream out;
    out << "= " << getTypeStr() << "(" << _id << ")";
    if(_symbol != "")
        out << "\"" << _symbol << "\"";
    out << ", line ";
    if(_lineNo == 0)
        out<<_lineNo;
    else
        out<<_lineNo+1;
    cout << setw(49) << left << out.str() << "=" <<endl;
    cout << "==================================================" << endl;
}

void
CirGate::reportFanin(int level)
{
   assert (level >= 0);
   CirGate::setGlobal();
   fiTraversal(level, 0, 0);
}

void
CirGate::reportFanout(int level)
{
   assert (level >= 0);
   CirGate::setGlobal();
   foTraversal(level, 0, 0);
}

void
CirGate::fiTraversal(int max, int lev, bool inv)
{
    for(int i = 0; i < lev; ++i)
        cout << "  ";
    if(inv)
        cout << "!";
    cout << getTypeStr() << " " << _id;
    if(lev == max || _faninList.empty()){
        cout << endl;
        return;
    }
    if(isGlobalRef()){
        cout << " (*)" << endl;
        return;
    }
    cout << endl;
    for(size_t i = 0; i < _faninList.size(); ++i)
        _faninList[i].gate() -> fiTraversal(max, lev+1, _faninList[i].isInv());
    setToGlobal();
}

void
CirGate::foTraversal(int max, int lev, bool inv)
{
    for(int i = 0; i < lev; ++i)
        cout << "  ";
    if(inv)
        cout << "!";
    cout << getTypeStr() << " " << _id;
    if(lev == max || _fanoutList.empty()){
        cout << endl;
        return;
    }
    if(isGlobalRef()){
        cout << " (*)" << endl;
        return;
    }
    cout << endl;
    for(size_t i = 0; i < _fanoutList.size(); ++i)
        _fanoutList[i].gate() -> foTraversal(max, lev+1, _fanoutList[i].isInv());
    setToGlobal();
}

void
CirGate::dfsTraversal(GateList& dfs, vector<unsigned>& v){
    for(size_t i = 0; i < _faninList.size(); ++i){
        CirGate* g = _faninList[i].gate();
        if(g != 0 && g -> getType() != UNDEF_GATE && !g -> isGlobalRef()){
            g -> setToGlobal();
            g -> dfsTraversal(dfs, v);
        }
    }
    if(getType() == AIG_GATE)
        v.push_back(dynamic_cast<CirAigGate*>(this)->getId());
    dfs.push_back(this);
}

void
CirGate::printGate() const
{
    if(_id == 0){
        cout << "CONST0";
        return;
    }
    cout << setw(4) << left << getTypeStr() << _id;
    for (size_t i = 0; i < _faninList.size(); ++i){
        cout << " ";
        if (_faninList[i].gate() -> getType() == UNDEF_GATE)
            cout << "*";
        if (_faninList[i].isInv())
            cout << "!";
        cout << _faninList[i].gate() -> _id;
    }
    if(_symbol != "")
        cout << " (" << _symbol << ")";
}

bool
CirGate::isFloating() const
{
    for (size_t i = 0; i < _faninList.size(); ++i){
        if(_faninList[i].gate() -> getType() == UNDEF_GATE)
            return true;
    }
    return false;
}
