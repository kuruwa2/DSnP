/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
class GateKey
{
public:
    GateKey(CirGate* gate){
        if ((*gate -> fanin())[0].getGate() < (*gate -> fanin())[1].getGate()){
            in0 = (*gate -> fanin())[0].getGate();
            in1 = (*gate -> fanin())[1].getGate();
        }
        else{
            in0 = (*gate -> fanin())[1].getGate();
            in1 = (*gate -> fanin())[0].getGate();
        }
    }
    size_t operator() () const{ return (in0 << 4) + in1; }
    bool operator== (const GateKey& k) const{ return (in0 == k.in0) && (in1 == k.in1); }
private:
    size_t in0;
    size_t in1;
};
/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
    IdList rm;
    HashMap<GateKey, CirGate*>* gateHash = new HashMap<GateKey, CirGate*>(_dfsList.size());
    for (size_t i = 0, s = _dfsList.size(); i < s; ++i){
        _dfsList[i] -> resetDfs();
        if (_dfsList[i] -> getType() == AIG_GATE){
            GateKey key(_dfsList[i]);
            CirGate* gate;
            if (gateHash -> query(key, gate)){
                rpoi("Strashing: ", _dfsList[i], gate->id());
                _dfsList[i] -> rmio();
                rm.push_back(_dfsList[i] -> id());
            }
            else
                gateHash -> insert(key, _dfsList[i]);
        }
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
}

void
CirMgr::fraig()
{
    solver.initialize();
    genProofModel();
    IdList rm;
    for (size_t i = 0, s = _fecGrps.size(); i < s; ++i){
        for (size_t j = 0, t = _fecGrps[i]->size(); j < t; ++j){
            if ((*_fecGrps[i])[j] != -1){
                for (size_t k = j+1; k < t; ++k){
                    if ((*_fecGrps[i])[k] != -1){
                        bool inv = (_gate[(*_fecGrps[i])[j]] -> getSimInv() != _gate[(*_fecGrps[i])[k]] -> getSimInv());
                        if(!solve((*_fecGrps[i])[j], (*_fecGrps[i])[k], inv)){
                            rpoi("Fraig: ", _gate[(*_fecGrps[i])[k]], (*_fecGrps[i])[j], inv);
                            _gate[(*_fecGrps[i])[k]] -> rmio();
                            rm.push_back(_gate[(*_fecGrps[i])[k]] -> id());
                            (*_fecGrps[i])[k] = -1;
                        }
                    }
                }
            }
        }
    }
    for(size_t i=0, s = _fecGrps.size(); i < s; ++i){
        for(size_t j = 0, t = _fecGrps[i] -> size(); j < t; ++j)
            if((*_fecGrps[i])[j] != -1 && _gate[(*_fecGrps[i])[j]] != 0)
                _gate[(*_fecGrps[i])[j]]->setFec(0);
        delete _fecGrps[i];
    }
    vector<IdList*> n;
    _fecGrps.swap(n);
    for (size_t i = 0, s = rm.size(); i < s; ++i){
        delete _gate[rm[i]];
        _gate[rm[i]] = 0;
        --MILOA[4];
    }
    CirGate::setGlobal();
    for (size_t i = 0, s = _dfsList.size(); i < s; ++i)
        _dfsList[i] -> resetDfs();
    _dfsList.clear();
    dfsAigs.clear();
    for (int i = 0; i < MILOA[3]; ++i)
        _gate[MILOA[0]+i+1] -> dfsTraversal(_dfsList, dfsAigs);
    strash();
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
void
CirMgr::genProofModel()
{
    _gate[0] -> setVar(solver.newVar());
    for (size_t i = 0, s = _dfsList.size(); i < s; ++i){
        Var v = solver.newVar();
        _dfsList[i] -> setVar(v);
    }
    for (size_t i = 0, s = _dfsList.size(); i < s; ++i)
        if (_dfsList[i] -> getType() == AIG_GATE)
            solver.addAigCNF(_dfsList[i] -> getVar(),
                (*_dfsList[i] -> fanin())[0].gate() -> getVar(),
                (*_dfsList[i] -> fanin())[0].isInv(),
                (*_dfsList[i] -> fanin())[1].gate() -> getVar(),
                (*_dfsList[i] -> fanin())[1].isInv());
}

bool
CirMgr::solve(size_t id1,size_t id2,bool inv)
{
    solver.assumeRelease();
    if(id1 == 0)
        solver.assumeProperty(_gate[id2] -> getVar(), !inv);
    else{
        Var f = solver.newVar();
        solver.addXorCNF(f, _gate[id1]->getVar(), false, _gate[id2]->getVar(), inv);
        solver.assumeProperty(f, true);
    }
    bool result = solver.assumpSolve();
    return result;
}
