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

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;
size_t CirGate::_globalRef = 0;
/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
    cout << "================================================================================" << endl;
    ostringstream out;
    out << "= " << getTypeStr() << "(" << _id << ")";
    if(_symbol != "")
        out << "\"" << _symbol << "\"";
    out << ", line ";
    if(_lineNo == 0)
        out<<_lineNo;
    else
        out<<_lineNo+1;
    cout << out.str() << endl;
    out.str("");
    out << "= FECs: ";
    if(_fecGrp != NULL){
        for(size_t i = 0, s = _fecGrp -> size(); i < s; ++i){
            if((cirMgr -> getGate((*_fecGrp)[i]) -> getSimInv()) != getSimInv())
                out << '!';
            if((*_fecGrp)[i] != _id)
                out << (*_fecGrp)[i] << " ";
        }
    }
    cout << out.str() << right << endl;
    cout << "= Value: ";
    for(size_t i = 1; i <= 32; ++i){
        cout << ((_simV & (0x1 << (32-i)))? '1':'0');
        if(i % 8 == 0 && i != 32)
            cout <<'_';
    }
    cout << "\n================================================================================" << endl;
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
    for(size_t i = 0, s = _faninList.size(); i < s; ++i)
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
    for(size_t i = 0, s = _fanoutList.size(); i < s; ++i)
        _fanoutList[i].gate() -> foTraversal(max, lev+1, _fanoutList[i].isInv());
    setToGlobal();
}

void
CirGate::dfsTraversal(GateList& dfs, vector<unsigned>& v){
    for(size_t i = 0, s = _faninList.size(); i < s; ++i){
        CirGate* g = _faninList[i].gate();
        if(g != 0  && !g -> isGlobalRef()){
            if (g -> getType() != UNDEF_GATE){
                g -> setToGlobal();
                g -> dfsTraversal(dfs, v);
                g -> _dfs = true;
            }
            else
                g -> _dfs = true;
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
    for (size_t i = 0, s = _faninList.size(); i < s; ++i){
        if(_faninList[i].gate() -> getType() == UNDEF_GATE)
            return true;
    }
    return false;
}

void
CirGate::rmio()
{
    for (size_t i = 0, s = _faninList.size(); i < s; ++i){
        vector<AigGateV>* v = &_faninList[i].gate() -> _fanoutList;
        for (size_t j = 0, t = v -> size(); j < t; ++j){
            if ((*v)[j].gate() == this)
                v -> erase(v -> begin() + j);
        }
    }
}

void
CirGate::simulate()
{
    for(size_t i = 0, s = _faninList.size(); i < s; ++i){
        CirGate* g = _faninList[i].gate();
        if(g != 0  && !g -> isGlobalRef()){
            if (g -> getType() != UNDEF_GATE){
                g -> setToGlobal();
                g -> simulate();
            }
        }
    }
    if (getType() == AIG_GATE){
        size_t in0 = _faninList[0].gate() -> getSim();
        if (_faninList[0].isInv())
            in0 = ~in0;
        size_t in1 = _faninList[1].gate() -> getSim();
        if (_faninList[1].isInv())
            in1 = ~in1;
        setSim (in0 & in1);
    }
    else if (getType() == PO_GATE){
        size_t in0 = _faninList[0].gate() -> getSim();
        if (_faninList[0].isInv())
            in0 = ~in0;
        setSim (in0);
    }
}

unsigned
CirGate::getCone(vector<string>* fanin, vector<string>* aigs)
{
    if (isGlobalRef() || getType() == CONST_GATE || getType() == UNDEF_GATE)
        return _id;
    setToGlobal();
	if (getType() == PI_GATE) {
		fanin -> push_back(std::to_string(2 * _id));
		return _id;
	}
	else if (getType() == AIG_GATE) {
		unsigned v1 = _faninList[0].gate() -> getCone(fanin, aigs);
        unsigned v2 = _faninList[1].gate() -> getCone(fanin, aigs);
		if (v2 > v1)
            v1 = v2;
		aigs -> push_back(std::to_string(2 * _id) + ' ' +
				std::to_string(2 * _faninList[0].gate() -> id() + _faninList[0].isInv()) + ' ' +
				std::to_string(2 * _faninList[1].gate() -> id() + _faninList[1].isInv()));
		if (_id < v1)
            return v1;
		return _id;
	}
    return _id;
}
