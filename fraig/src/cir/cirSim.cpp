/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
class SimValue
{
public:
    SimValue(unsigned sim): _sim(sim) {}
    unsigned operator () () const { return _sim; }
    bool operator == (const SimValue& s) const { return _sim == s._sim; }
private:
    unsigned _sim;
};
struct{
    bool operator()(IdList* a, IdList* b){
        return a[0] < b[0];
    }
} customLess;
/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
    vector<unsigned> simvalue;
    simvalue.resize(MILOA[1], 0);
    int max = int(log2(MILOA[0])) * 20;
    int cycles = 0, patterns = 0;
    siminit();
    while(cycles < max){
        for (size_t i = 0, s = simvalue.size(); i < s; ++i)
            simvalue[i] = rnGen(INT_MAX);
        if (simulate(simvalue))
            cycles = 0;
        else
            ++cycles;
        ++patterns;
        if (_simLog != 0)
            recordSim(32);
    }
    std::sort(_fecGrps.begin(), _fecGrps.end(), customLess);
    cout << '\r' << patterns * 32 << " patterns simulated." << endl;
}

void
CirMgr::fileSim(ifstream& patternFile)
{
    vector<unsigned> simvalue;
    unsigned patterns = 0;
    simvalue.resize(MILOA[1], 0);
    string line;
    vector<string> lines;
    bool err = false, first = true;
    while (getline(patternFile, line) && !err){
        size_t begin = line.find_first_not_of(" ");
        size_t end;
        while (begin != string::npos){
            end = line.find_first_of(" ", begin);
            lines.push_back(line.substr(begin, end - begin));
            begin = line.find_first_not_of(" ", end);
        }
        for (size_t i = 0, s = lines.size(); i < s; ++i){
            if (lines[i].length() != size_t(MILOA[1])){
                cerr << "Error: Pattern(" << lines[i] << ") length(" << lines[i].length()
                     << ") does not match the number of inputs(" << MILOA[1]
                     << ") in a circuit!!" << endl;
                patterns = patterns / 32 * 32;
                err = true;
                break;
            }
            size_t pos = lines[i].find_first_not_of("01");
            if (pos != string::npos){
                cerr << "Error: Pattern(" << lines[i] << ") contains a non-0/1 character(‘"
                     << lines[i][pos] << "’)." << endl;
                patterns = patterns / 32 * 32;
                err = true;
                break;
            }
            for (int j = 0; j < MILOA[1]; ++j)
                if (lines[i][j] == '1')
                    simvalue[j] += (1 << (patterns % 32));
            ++ patterns;
            if (patterns % 32 == 0){
                if (first){
                    siminit();
                    first = false;
                }
                simulate(simvalue);
                if (_simLog != 0)
                    recordSim(32);
                for (int j = 0; j < MILOA[1]; ++j)
                    simvalue[j] = 0;
            }
        }
        lines.clear();
    }
    if (patterns % 32 != 0 && !err){
        if (first)
            siminit();
        simulate(simvalue);
        if (_simLog != 0)
            recordSim(patterns % 32);
        for (int j = 0; j < MILOA[1]; ++j)
            simvalue[j] = 0;
    }
    std::sort(_fecGrps.begin(), _fecGrps.end(), customLess);
    cout << '\r' << patterns << " patterns simulated." << endl;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
void
CirMgr::siminit()
{
    _fecGrps.clear();
    IdList* first = new IdList;
    first -> push_back(0);
    for (size_t i = 1, s = _gate.size(); i < s; ++i){
        if (_gate[i] != 0 && _gate[i] -> getType() == AIG_GATE && _gate[i] -> isdfs()){
            first -> push_back(_gate[i] -> id());
            _gate[i] -> setFec(first);
        }
    }
    _fecGrps.push_back(first);
}
bool
CirMgr::simulate(vector<unsigned>& simvalue)
{
    for (int i = 0, s = MILOA[1]; i < s; ++i)
        _gate[PIs[i]/2] -> setSim(simvalue[i]);
    CirGate::setGlobal();
    for (int i = 1; i <= MILOA[3]; ++i)
        _gate[MILOA[0]+i] -> simulate();
    vector<IdList*> fec;
    _fecGrps.swap(fec);
    for (size_t i = 0, s = fec.size(); i < s; ++i){
        HashMap<SimValue, IdList*> newFecGrps(fec[i] -> size());
        for (size_t j = 0, t = fec[i] -> size(); j < t; ++j){
            IdList* grp = 0;
            unsigned sim = _gate[(*(fec[i]))[j]] -> getSim();
            SimValue simV(sim);
            SimValue simVinv(~sim);
            if (newFecGrps.query(simV, grp)){
                grp -> push_back((*(fec[i]))[j]);
                _gate[(*fec[i])[j]] -> setSimInv(false);
            }
            else if(newFecGrps.query(simVinv, grp)){
                grp -> push_back((*(fec[i]))[j]);
                _gate[(*fec[i])[j]] -> setSimInv(true);
            }
            else{
                grp = new IdList;
                grp -> push_back((*(fec[i]))[j]);
                _gate[(*fec[i])[j]] -> setSimInv(false);
                newFecGrps.insert(simV, grp);
            }
            _gate[(*(fec[i]))[j]] -> setFec(grp);
        }
        HashMap<SimValue, IdList*>::iterator it = newFecGrps.begin();
        for (; it != newFecGrps.end(); ++it){
            if(( (*it).second) -> size() > 1)
                _fecGrps.push_back((*it).second);
            else{
                _gate[((*it).second -> front())] -> setFec(0);
                delete (*it).second;
            }
        }
    }
    for(size_t i = 0, s = fec.size(); i < s; ++i){
        delete fec[i];
        fec[i] = 0;
    }
    cout << "\rTotal #FEC Group = " << _fecGrps.size();
    return fec.size() != _fecGrps.size();
}

void
CirMgr::recordSim(int num)
{
    for(int i = 0; i < num; ++i){
        for(int j = 0; j < MILOA[1]; ++j){
            if((_gate[PIs[j]/2] -> getSim() & (0x1 << i)))
                *_simLog << '1';
            else
                *_simLog << '0';
        }
        *_simLog << " ";

        for(int j = 0; j < MILOA[3]; ++j){
            if((_gate[MILOA[0] + j + 1] -> getSim() & (0x1 << i)))
                *_simLog << '1';
            else
                *_simLog << '0';
        }
        *_simLog << endl;
    }
}
