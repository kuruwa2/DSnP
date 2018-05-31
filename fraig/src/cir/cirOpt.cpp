/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
    IdList rm;
    for (size_t i = 1, s = MILOA[0]; i <= s; ++i){
        if (_gate[i] != 0 && _gate[i] -> getType() != PI_GATE && !(_gate[i] -> isdfs())){
            cout << "Sweeping: " << _gate[i] -> getTypeStr() << "("
                 << _gate[i] -> id() << ") removed..." << endl;
            _gate[i] -> rmio();
            rm.push_back(i);
        }
    }
    for (size_t i = 0, s = rm.size(); i < s; ++i){
        delete _gate[rm[i]];
        _gate[rm[i]] = 0;
    }
    MILOA[4] = dfsAigs.size();
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
    IdList rm;
    for (size_t i = 0, s = _dfsList.size(); i < s; ++i){
        _dfsList[i] -> resetDfs();
        if (_dfsList[i] -> getType() == AIG_GATE)
            if (check(_dfsList[i]))
                rm.push_back(_dfsList[i]->id());
    }
    for (size_t i = 0, s = rm.size(); i < s; ++i){
        delete _gate[rm[i]];
        _gate[rm[i]] = 0;
        --MILOA[4];
    }
    CirGate::setGlobal();
    _dfsList.clear();
    dfsAigs.clear();
    for (int i = 0; i < MILOA[3]; ++i)
        _gate[MILOA[0]+i+1] -> dfsTraversal(_dfsList, dfsAigs);
    for (int i = 0; i < MILOA[0]; ++i){
        if (_gate[i] != 0 && _gate[i] -> getType() == UNDEF_GATE
                          && _gate[i] -> fanout() -> empty()){
            delete _gate[i];
            _gate[i] = 0;
        }
    }
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/

bool
CirMgr::check(CirGate* gate)
{
    vector<AigGateV>* v = gate->fanin();
    unsigned in0 = (*v)[0].gate() -> id();
    bool v0 = (*v)[0].isInv();
    unsigned in1 = (*v)[1].gate() -> id();
    bool v1 = (*v)[1].isInv();
    if (in0 == 0){
        if (v0)
            rpoi("Simplifying: ", gate, in1, v1);
        else
            rpoi("Simplifying: ", gate, 0);
        gate -> rmio();
        return true;
    }
    if (in1 == 0){
        if (v1)
            rpoi("Simplifying: ", gate, in0, v0);
        else
            rpoi("Simplifying: ", gate, 0);
        gate -> rmio();
        return true;
    }
    if (in0 == in1){
        if (v0 == v1)
            rpoi("Simplifying: ", gate, in0, v0);
        else
            rpoi("Simplifying: ", gate, 0);
        gate -> rmio();
        return true;
    }
    return false;
}

void
CirMgr::rpoi(string sim, CirGate* gate, unsigned gid, bool inv)
{
    cout << sim << gid << " merging ";
    if (inv) cout << "!";
    cout << gate -> id() << "...\n";
    vector<AigGateV>* mv = gate->fanout();
    for (size_t i = 0, s = mv -> size(); i < s; ++i){
        vector<AigGateV>* v = (*mv)[i].gate() -> fanin();
        for (size_t j = 0, t = v -> size(); j < t; ++j){
            if ((*v)[j].gate() == gate){
                bool vv = (*v)[j].isInv();
                (*v)[j] = AigGateV(_gate[gid], inv);
                if (vv) (*v)[j].inverse();
                _gate[gid] -> setfanout(AigGateV((*mv)[i].gate(), (*v)[j].isInv()));

                if ((*mv)[i].gate() -> getType() == PO_GATE){
                    unsigned id = dynamic_cast<CirPoGate*>((*mv)[i].gate()) -> getId();
                    POs[id] = 2 * (_gate[gid] -> id()) + (*v)[j].isInv();
                }
                else if ((*mv)[i].gate() -> getType() == AIG_GATE){
                    unsigned id = dynamic_cast<CirAigGate*>((*mv)[i].gate()) -> getId();
                    if (Aigs[id][1] / 2 == gate -> id())
                        Aigs[id][1] = 2 * (_gate[gid] -> id()) + (*v)[j].isInv();
                    else
                        Aigs[id][2] = 2 * (_gate[gid] -> id()) + (*v)[j].isInv();
                }
            }
        }
    }
}
