/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp

bool
initMemCmd()
{
   if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
         cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
         cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
         cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
      )) {
      cerr << "Registering \"mem\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token))
      return CMD_EXEC_ERROR;
   if (token.size()) {
      int b;
      if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj)))) {
         cerr << "Illegal block size (" << token << ")!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
      }
      #ifdef MEM_MGR_H
      mtest.reset(toSizeT(b));
      #else
      mtest.reset();
      #endif // MEM_MGR_H
   }
   else
      mtest.reset();
   return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{
   os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{
   cout << setw(15) << left << "MTReset: "
        << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string& option)
{
   // TODO
    vector<string> options;
    CmdExec::lexOptions(option, options);
    size_t size = options.size();
    if(size == 0)
        return CmdExec::errorOption(CMD_OPT_MISSING, "");
    int n, s;
    if(!myStr2Int(options[0], n) || n <= 0){
        if(myStrNCmp("-Array", options[0], 2))
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
        if(size == 1)
            return CmdExec::errorOption(CMD_OPT_MISSING, options[0]);
        if(!myStr2Int(options[1], s) || s <= 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
        if(size == 2)
            return CmdExec::errorOption(CMD_OPT_MISSING, "");
        if(!myStr2Int(options[2], n) || n <= 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
        if(size > 3)
            return CmdExec::errorOption(CMD_OPT_EXTRA, options[3]);
        try{
            mtest.newArrs(n, s);
        }
        catch(bad_alloc){}
    }
    else{
        if(size == 1){
            try{
                mtest.newObjs(n);
            }
            catch(bad_alloc){}
        }
        else{
            if(myStrNCmp("-Array", options[1], 2))
                return CmdExec::errorOption(CMD_OPT_EXTRA, options[1]);
            if(size == 2)
                return CmdExec::errorOption(CMD_OPT_MISSING, options[1]);
            if(!myStr2Int(options[2], s) || s <= 0)
                return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
            if(size > 3)
                return CmdExec::errorOption(CMD_OPT_EXTRA, options[3]);
            try{
                mtest.newArrs(n, s);
            }
            catch(bad_alloc){}
        }
    }
    return CMD_EXEC_DONE;
}

void
MTNewCmd::usage(ostream& os) const
{
   os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{
   cout << setw(15) << left << "MTNew: "
        << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
   // TODO
    vector<string> options;
    int idx;
    bool a = false;
    bool i = false;
    bool r = false;
    CmdExec::lexOptions(option, options);
    size_t size = options.size();
    if(size == 3){
        if(!myStrNCmp("-Array", options[0], 2)){
            a = true;
            options.erase(options.begin());
        }
        else if(!myStrNCmp("-Array", options[2], 2)){
            a = true;
            options.pop_back();
        }
    }
    size = options.size();
    if(size == 0)
        return CmdExec::errorOption(CMD_OPT_MISSING, "");
    if(!myStrNCmp("-Index", options[0], 2)){
        if(size == 1)
            return CmdExec::errorOption(CMD_OPT_MISSING, options[0]);
        i = true;
    }
    else if(!myStrNCmp("-Random", options[0], 2)){
        if(size == 1)
            return CmdExec::errorOption(CMD_OPT_MISSING, options[0]);
        r = true;
    }
    else
        return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
    if(!myStr2Int(options[1], idx))
        return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
    if(size > 2)
        return CmdExec::errorOption(CMD_OPT_EXTRA, options[2]);

    if(i && !a){
        if(idx < 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
        if(idx >= (int)mtest.getObjListSize()){
            cerr << "Size of object list (" << mtest.getObjListSize() << ") is <= " << idx << "!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
        }
        mtest.deleteObj(idx);
    }
    else if(i && a){
        if(idx < 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
        if(idx >= (int)mtest.getArrListSize()){
            cerr << "Size of array list (" << mtest.getArrListSize() << ") is <= " << idx << "!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
        }
        mtest.deleteArr(idx);
    }
    else if(r && !a){
        if(idx <= 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
        if(mtest.getObjListSize() == 0){
            cerr << "Size of object list is 0!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
        }
        while(idx--)
            mtest.deleteObj(rnGen(mtest.getObjListSize()));
    }
    else if(r && a){
        if(idx <= 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
        if(mtest.getArrListSize() == 0){
            cerr << "Size of array list is 0!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
        }
        while(idx--)
            mtest.deleteArr(rnGen(mtest.getArrListSize()));
    }
    return CMD_EXEC_DONE;
}

void
MTDeleteCmd::usage(ostream& os) const
{
   os << "Usage: MTDelete <-Index (size_t objId) | "
      << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{
   cout << setw(15) << left << "MTDelete: "
        << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string& option)
{
   // check option
   if (option.size())
      return CmdExec::errorOption(CMD_OPT_EXTRA, option);
   mtest.print();

   return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{
   os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{
   cout << setw(15) << left << "MTPrint: "
        << "(memory test) print memory manager info" << endl;
}
