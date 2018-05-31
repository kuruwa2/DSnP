/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class AigGateV
{
public:
    #define NEG 0x1
    AigGateV(CirGate* g, size_t phase):_gateV(size_t(g) + phase) {}
    CirGate* gate() const { return (CirGate*) (_gateV & ~size_t(NEG)); }
    bool isInv() const { return (_gateV & NEG); }
    void inverse() { _gateV ^= NEG; }
    size_t getGate() { return _gateV; }
private:
    size_t _gateV;
};

class CirGate
{
public:
   CirGate(unsigned id, unsigned ln = 0): _id(id), _lineNo(ln), _ref(0), _dfs(false), _simV(0) {}
   virtual ~CirGate() {}

   // Basic access methods
   virtual string getTypeStr() const = 0;
   virtual GateType getType() const = 0;
   unsigned getLineNo() const { return _lineNo; }
   bool isAig() const { return getType() == AIG_GATE; }

   unsigned id() { return _id; }

   // Printing functions
   virtual void printGate() const;
   void reportGate() const;
   void reportFanin(int level);
   void reportFanout(int level);

   string getSymbol() const { return _symbol; }
   void setsymbol (string s) { _symbol = s; }

   void setfanin(AigGateV g) { _faninList.push_back(g); }
   void setfanout(AigGateV g) { _fanoutList.push_back(g); }
   vector<AigGateV>* fanin() { return &_faninList; }
   vector<AigGateV>* fanout() { return &_fanoutList; }

   bool isGlobalRef() { return _ref == _globalRef; }
   void setToGlobal() { _ref = _globalRef; }
   static void setGlobal() { ++_globalRef; }
   void dfsTraversal(GateList&, vector<unsigned>&);
   void resetDfs() { _dfs = false; }
   bool isdfs() { return _dfs; }

   bool isFloating() const;

   void fiTraversal(int, int, bool);
   void foTraversal(int, int, bool);

   bool nofanout() { return _fanoutList.empty(); }

   void rmio();

   void setFec(IdList* g) { _fecGrp = g; }
   IdList* getFec() const { return _fecGrp; }
   void setVar(Var v) { _satVar = v; }
   Var getVar() { return _satVar; }
   void setSim(unsigned s) { _simV = s; }
   unsigned getSim () const { return _simV; }
   void setSimInv(bool b) { _simInv = b; }
   bool getSimInv() const { return _simInv; }
   void simulate();

   unsigned getCone(vector<string>*, vector<string>*);
protected:
    unsigned _id;
    unsigned _lineNo;
    string _symbol;

    size_t _ref;
    static size_t _globalRef;
    bool _dfs;

    vector<AigGateV> _faninList;
    vector<AigGateV> _fanoutList;

    IdList* _fecGrp;
    unsigned _simV;
    Var _satVar;
    bool _simInv;
};

class CirAigGate: public CirGate
{
public:
    CirAigGate(unsigned id, unsigned ln): CirGate(id, ln) {}
    ~CirAigGate() {}
    string getTypeStr() const { return "AIG"; }
    GateType getType() const { return AIG_GATE; }

    void setId(unsigned i) { aigid = i; }
    unsigned getId() const{ return aigid; }
private:
    unsigned aigid;
};

class CirPiGate: public CirGate
{
public:
    CirPiGate(unsigned id, unsigned ln): CirGate(id, ln) {}
    ~CirPiGate() {}
    string getTypeStr() const { return "PI"; }
    GateType getType() const { return PI_GATE; }
};

class CirPoGate: public CirGate
{
public:
    CirPoGate(unsigned id, unsigned ln): CirGate(id, ln) {}
    ~CirPoGate() {}
    string getTypeStr() const { return "PO"; }
    GateType getType() const { return PO_GATE; }

    void setId(unsigned i) { poid = i; }
    unsigned getId() const{ return poid; }
private:
    unsigned poid;
};

class CirConstGate: public CirGate
{
public:
    CirConstGate(unsigned id = 0, unsigned ln = 0): CirGate(id, ln) {}
    ~CirConstGate() {}
    string getTypeStr() const { return "CONST"; }
    GateType getType() const { return CONST_GATE; }
};

class CirUndefGate: public CirGate
{
public:
    CirUndefGate(unsigned id): CirGate(id) {}
    ~CirUndefGate() {}
    string getTypeStr() const { return "UNDEF"; }
    GateType getType() const { return UNDEF_GATE; }
};

#endif // CIR_GATE_H
