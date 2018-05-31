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

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes

class AigGateV
{
public:
    #define NEG 0x1
    AigGateV(CirGate* g, size_t phase):_gateV(size_t(g) + phase) {}
    CirGate* gate() const { return (CirGate*) (_gateV & ~size_t(NEG)); }
    bool isInv() const { return (_gateV & NEG); }
private:
    size_t _gateV;
};

class CirGate
{
public:
   CirGate(unsigned id, unsigned ln = 0): _id(id), _lineNo(ln), _ref(0) {}
   virtual ~CirGate() {}

   // Basic access methods
   virtual string getTypeStr() const = 0;
   virtual GateType getType() const = 0;
   unsigned getLineNo() const { return _lineNo; }

   // Printing functions
   virtual void printGate() const;
   void reportGate() const;
   void reportFanin(int level);
   void reportFanout(int level);

   string getSymbol() const { return _symbol; }
   void setsymbol (string s) { _symbol = s; }

   void setfanin(AigGateV g) { _faninList.push_back(g); }
   void setfanout(AigGateV g) { _fanoutList.push_back(g); }

   bool isGlobalRef() { return _ref == _globalRef; }
   void setToGlobal() { _ref = _globalRef; }
   static void setGlobal() { ++_globalRef; }
   void dfsTraversal(GateList&, vector<unsigned>&);

   bool isFloating() const;

   void fiTraversal(int, int, bool);
   void foTraversal(int, int, bool);

   bool nofanout() { return _fanoutList.empty(); }

protected:
    unsigned _id;
    unsigned _lineNo;
    unsigned _colNo;
    string _symbol;

    size_t _ref;
    static size_t _globalRef;

    vector<AigGateV> _faninList;
    vector<AigGateV> _fanoutList;
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
