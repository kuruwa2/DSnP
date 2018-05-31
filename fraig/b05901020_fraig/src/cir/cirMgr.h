/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"
#include "cirGate.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr() {}
   ~CirMgr() {
       for(size_t i = 0; i < _gate.size(); ++i){
            if(_gate[i] != 0)
                delete _gate[i];
            _gate[i] = 0;
        }
   }

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const {
       if(size_t(gid) >= _gate.size() || _gate[gid] == 0 || _gate[gid]->getType() == UNDEF_GATE)
            return 0;
        return _gate[gid];
   }

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

private:
   ofstream           *_simLog;

   int MILOA[5];

   IdList PIs;
   IdList POs;
   vector<vector<unsigned>> Aigs;
   IdList dfsAigs;

   GateList _gate;
   GateList _dfsList;

   vector<IdList*> _fecGrps;
   SatSolver solver;
   bool simulate(vector<unsigned>&);
   void siminit();
   void recordSim(int);

   bool parseheader(ifstream&);
   bool parsepi(ifstream&);
   bool parsepo(ifstream&);
   bool parseaig(ifstream&);
   bool parsesym(ifstream&);
   void connect();
   string::iterator cant_print(string&);

   bool check(CirGate*);
   void rpoi(string, CirGate*, unsigned gid, bool inv = 0);

   void genProofModel();
   bool solve(size_t, size_t, bool);
};

#endif // CIR_MGR_H
