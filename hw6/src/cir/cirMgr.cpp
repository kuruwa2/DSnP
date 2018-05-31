/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <algorithm>

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
//static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()+1
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine constant (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
    ifstream file(fileName);
    if(!file){
        cerr << "Cannot open design \"" << fileName << "\"!!" << endl;
        return false;
    }
    lineNo = 0; colNo = 0;
    if(!parseheader(file)) return false;
    ++lineNo;
    colNo = 0;
    if(!parsepi(file)) return false;
    if(!parsepo(file)) return false;
    if(!parseaig(file)) return false;
    colNo = 0;
    if(!parsesym(file)) return false;

    connect();
    CirGate::setGlobal();
    for(int i = 0; i < MILOA[3]; ++i){
        _gate[MILOA[0]+i+1] -> dfsTraversal(_dfsList, dfsAigs);
    }

    return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
    cout << "\nCircuit Statistics\n"
         << "==================\n"
         << "  PI  " << setw(10) << right << MILOA[1] << endl
         << "  PO  " << setw(10) << right << MILOA[3] << endl
         << "  AIG " << setw(10) << right << MILOA[4] << endl
         << "------------------\n"
         << "  Total  " << setw(7) << MILOA[1] + MILOA[3] + MILOA[4] << endl;
}

void
CirMgr::printNetlist() const
{
    cout << endl;
    for(size_t i = 0; i < _dfsList.size(); ++i){
        cout << "[" << i << "] ";
        _dfsList[i] -> printGate();
        cout << endl;
    }
}

void
CirMgr::printPIs() const
{
    cout << "PIs of the circuit:";
    for (size_t i = 0; i < PIs.size(); ++i)
        cout << " " << PIs[i]/2;
    cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (size_t i = 0; i < POs.size(); ++i)
       cout << " " << MILOA[0] + i + 1;
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
    bool first = true;
    for(size_t i = 1; i < _gate.size(); ++i){
        if(_gate[i] != 0 && _gate[i] -> isFloating()){
            if (first){
                cout << "Gates with floating fanin(s):";
                first = false;
            }
            cout << " " << i;
        }
    }
    if(!first)
        cout << endl;
    first = true;
    for(int i = 1; i <= MILOA[0]; ++i){
        if(_gate[i] != 0 && _gate[i]->getType() != UNDEF_GATE && !_gate[i] -> isGlobalRef() && _gate[i]->nofanout()){
            if (first){
                cout << "Gates defined but not used  :";
                first = false;
            }
            cout << " " << i;
        }
    }
    if(!first)
        cout<<endl;
}

void
CirMgr::writeAag(ostream& outfile) const
{
    outfile << "aag";
    for(int i = 0; i < 4; ++i)
        outfile << " " << MILOA[i];
    outfile << " " << dfsAigs.size() << endl;
    for(size_t i = 0; i < PIs.size(); ++i)
        outfile << PIs[i] << endl;
    for(size_t i = 0; i < POs.size(); ++i)
        outfile << POs[i] << endl;
    for(size_t i = 0; i < dfsAigs.size(); ++i){
        unsigned id = dfsAigs[i];
        outfile << Aigs[id][0] << " " << Aigs[id][1] << " " << Aigs[id][2] << endl;
    }
    for(size_t i = 0; i < PIs.size(); ++i){
        if(_gate[PIs[i]/2]->getSymbol() != "")
            outfile << "i" << i << " " << _gate[PIs[i]/2] -> getSymbol() << endl;
    }
    for(int i = 0; i < MILOA[3]; ++i){
        if(_gate[MILOA[0]+i+1]->getSymbol() != "")
            outfile << "o" << i << " " << _gate[MILOA[0]+i+1]->getSymbol() << endl;
    }
    outfile << "c" << endl;
    outfile << "How do you turn this on" << endl;
}

bool
CirMgr::parseheader(ifstream& file)
{
    string line;
    getline(file, line, '\n');
    size_t found = line.find(' ');
    if(found == string::npos){
        errMsg = "aag";
        return parseError(MISSING_IDENTIFIER);
    }
    string token = line.substr(0, found);
    line  = line.substr(found+1);
    if(token != "aag"){
        errMsg = token;
        return parseError(ILLEGAL_IDENTIFIER);
    }
    colNo += 3;

    int i = 0;
    string headerr[] = {"variables", "PIs", "latches", "POs", "AIGs"};
    while(i < 5){
        if(line.empty()){
            errMsg = "number of " + headerr[i];
            return parseError(MISSING_NUM);
        }
        ++colNo;
        found = line.find(' ');
        token = line.substr(0, found);
        if(token.empty())
            return parseError(EXTRA_SPACE);
        if(!myStr2Int(token, MILOA[i])){
            errMsg = "number of " + headerr[i] + "(" + token + ")";
            return parseError(ILLEGAL_NUM);
        }
        if(found == string::npos)
            line.clear();
        else
            line = line.substr(found+1);
        colNo += token.size();
        ++i;
    }
    if(found != string::npos){
        return parseError(MISSING_NEWLINE);
    }
    if(MILOA[0] < MILOA[1] + MILOA[2] + MILOA[4]){
        errMsg = "Number of " + headerr[0];
        errInt = MILOA[0];
        return parseError(NUM_TOO_SMALL);
    }
    if(MILOA[2] > 0){
        errMsg = "latches";
        return parseError(ILLEGAL_NUM);
    }

    _gate.resize(MILOA[0] + MILOA[3] + 1, 0);
    _gate[0] = new CirConstGate;
    return true;
}

bool
CirMgr::parseaig(ifstream& file)
{
    string io, token;
    size_t found;
    vector<int> temp (3, 0);
    for(int i = 0; i < MILOA[4]; ++i){
        colNo = 0;
        getline(file, io);
        int k = 0;
        while(k < 3){
            if(io.empty())
                return parseError(MISSING_SPACE);
            found = io.find(' ');
            token = io.substr(0, found);
            if(token.empty())
                return parseError(EXTRA_SPACE);
            if(!myStr2Int(token, temp[k])){
                errMsg = "AIG input literal ID(" + token + ")";
                return parseError(ILLEGAL_NUM);
            }
            if(found == string::npos)
                io.clear();
            else
                io = io.substr(found+1);
            colNo += token.size()+1;
            ++k;
        }
        /*vector<unsigned> aigid;
        for(int j = 0; j < 3; ++j){
            found = io.find(' ');
            aigid.push_back(stoi(io.substr(0, found)));
            io = io.substr(found+1);
        }*/
        if(found != string::npos){
            return parseError(MISSING_NEWLINE);
        }
        vector<unsigned> aigid;
        for (size_t j = 0; j < 3; ++j){
            aigid.push_back(unsigned(temp[j]));
            if(temp[j]/2 > MILOA[0]){
                errInt = temp[j];
                return parseError(MAX_LIT_ID);
            }
        }
        if(_gate[aigid[0]/2] != 0){
            errGate = _gate[aigid[0]/2];
            errInt = aigid[0];
            return parseError(REDEF_GATE);
        }
        _gate[aigid[0]/2] = new CirAigGate(aigid[0]/2, lineNo);
        dynamic_cast<CirAigGate*>(_gate[aigid[0]/2]) -> setId(i);
        Aigs.push_back(aigid);
        ++lineNo;
    }
    return true;
}

bool
CirMgr::parsepi(ifstream& file)
{
    string io;
    for(int i = 0; i < MILOA[1]; ++i){
        getline(file, io);
        int test;
        if (!myStr2Int(io, test)){
            errMsg = "PI";
            return parseError(MISSING_DEF);
        }
        unsigned id = stoi(io);
        if(id%2){
            errMsg = "PI";
            errInt = id;
            return parseError(CANNOT_INVERTED);
        }
        if(find(PIs.begin(), PIs.end(), id) != PIs.end()){
            vector<unsigned>::iterator it;
            it = find(PIs.begin(), PIs.end(), id);
            errInt = id;
            errGate = _gate[*it/2];
            return parseError(REDEF_GATE);
        }
        if(int(id)/2 > MILOA[0]){
            errInt = id;
            return parseError(MAX_LIT_ID);
        }
        if(id == 0){
            errInt = 0;
            return parseError(REDEF_CONST);
        }
        PIs.push_back(id);
        _gate[id/2] = new CirPiGate(id/2, lineNo);
        ++lineNo;
    }
    return true;
}

bool
CirMgr::parsepo(ifstream& file)
{
    string io;
    for(int i = 0; i < MILOA[3]; ++i){
        getline(file, io);
        int test;
        if (!myStr2Int(io, test)){
            errMsg = "PO";
            return parseError(MISSING_DEF);
        }
        unsigned id = stoi(io);
        if(int(id)/2 > MILOA[0]){
            errInt = id;
            return parseError(MAX_LIT_ID);
        }
        if(!id/2){
            errInt = id;
            return parseError(REDEF_CONST);
        }
        POs.push_back(id);
        _gate[MILOA[0]+i+1] = new CirPoGate(MILOA[0]+i+1, lineNo);
        ++lineNo;
    }
    return true;
}

bool
CirMgr::parsesym(ifstream& file)
{
    string io;
    while(getline(file, io)){
        if(io[0] == 'c'){
            ++colNo;
            if(io[1] != 0)
                return parseError(MISSING_NEWLINE);
            return true;
        }
        if(io[0] != 'i' && io[0] != 'l' && io[0] != 'o'){
            errMsg = io[0];
            return parseError(ILLEGAL_SYMBOL_TYPE);
        }
        int id;
        string idna = io.substr(1, io.find(' ')-1);
        if(idna.empty())
            return parseError(EXTRA_SPACE);
        if(!myStr2Int(idna, id)){
            errMsg = idna;
            return parseError(ILLEGAL_SYMBOL_TYPE);
        }
        if(io.find(' ') == string::npos)
            idna.clear();
        else
            idna = io.substr(io.find(' ')+1);

        if(idna.empty()){
            errMsg = "symboic name";
            return parseError(MISSING_IDENTIFIER);
        }
        string::iterator it = cant_print(idna);
        if(it != idna.end()){
            errInt = *it;
            return parseError(ILLEGAL_SYMBOL_NAME);
        }
        if(io[0] == 'i'){
            if(size_t(id) > PIs.size()){
                errInt = id;
                errMsg = "PI index";
                return parseError(NUM_TOO_BIG);
            }
            if(_gate[PIs[id]/2] -> getSymbol() != ""){
                errMsg = "i";
                errInt = id;
                return parseError(REDEF_SYMBOLIC_NAME);
            }
            _gate[PIs[id]/2] -> setsymbol(idna);
        }
        else if(io[0] == 'o'){
            if(size_t(id) > POs.size()){
                errInt = id;
                errMsg = "PO index";
                return parseError(NUM_TOO_BIG);
            }
            if(_gate[MILOA[0]+id+1] -> getSymbol() != ""){
                errMsg = "o";
                errInt = id;
                return parseError(REDEF_SYMBOLIC_NAME);
            }
            _gate[MILOA[0]+id+1] -> setsymbol(idna);
        }
        ++lineNo;
    }
    return true;
}

void
CirMgr::connect()
{
    for(int i = 0; i < MILOA[3]; ++i){
        unsigned id = POs[i] / 2;
        bool v = POs[i] % 2;
        if(_gate[id] != 0)
            _gate[id] -> setfanout(AigGateV(_gate[MILOA[0]+i+1], v));
        else{
            _gate[id] = new CirUndefGate(id);
            _gate[id] -> setfanout(AigGateV(_gate[MILOA[0]+i+1], v));
        }
        _gate[MILOA[0]+i+1] -> setfanin(AigGateV(_gate[id], v));
    }
    for(int i = 0; i < MILOA[4]; ++i){
        unsigned id = Aigs[i][0] / 2;
        unsigned in1 = Aigs[i][1] / 2;
        bool v1 = Aigs[i][1] % 2;
        unsigned in2 = Aigs[i][2] / 2;
        bool v2 = Aigs[i][2] % 2;

        if(_gate[in1] != 0)
            _gate[in1] -> setfanout(AigGateV(_gate[id], v1));
        else{
            _gate[in1] = new CirUndefGate(in1);
            _gate[in1] -> setfanout(AigGateV(_gate[id], v1));
        }
        if(_gate[in2] != 0)
            _gate[in2] -> setfanout(AigGateV(_gate[id], v2));
        else{
            _gate[in2] = new CirUndefGate(in2);
            _gate[in2] -> setfanout(AigGateV(_gate[id], v2));
        }
        _gate[id] -> setfanin(AigGateV(_gate[in1], v1));
        _gate[id] -> setfanin(AigGateV(_gate[in2], v2));
    }
}

string::iterator
CirMgr::cant_print(string& s)
{
    return std::find_if(s.begin(), s.end(), [](unsigned char c){return !std::isprint(c);});
}
