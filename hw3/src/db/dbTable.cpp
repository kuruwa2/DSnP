/****************************************************************************
  FileName     [ dbTable.cpp ]
  PackageName  [ db ]
  Synopsis     [ Define database Table member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iomanip>
#include <string>
#include <cctype>
#include <cassert>
#include <set>
#include <algorithm>
#include "dbTable.h"
#include "util.h"
#include <sstream>

using namespace std;

/*****************************************/
/*          Global Functions             */
/*****************************************/
ostream& operator << (ostream& os, const DBRow& r)
{
   // TODO: to print out a row.
   // - Data are seperated by a space. No trailing space at the end.
   // - Null cells are printed as '.'
    DBTable temp;
    size_t i;
    for(i = 0; i < r.size() - 1; ++i){
        temp.printData(os, r[i]);
        cout << ' ';
    }
    temp.printData(os, r[i]);
    cout << endl;
    return os;
}

ostream& operator << (ostream& os, const DBTable& t)
{
   // TODO: to print out a table
   // - Data are seperated by setw(6) and aligned right.
   // - Null cells are printed as '.'
    for (size_t i = 0; i < t.nRows(); ++i){
        for(size_t j = 0; j < t.nCols(); ++j){
            os << right << setw(6);
            t.printData(os, t._table[i][j]);
        }
        os << endl;
    }
    return os;
}

ifstream& operator >> (ifstream& ifs, DBTable& t)
{
   // TODO: to read in data from csv file and store them in a table
   // - You can assume the input file is with correct csv file format
   // - NO NEED to handle error file format
    string line;
    int rows=0, cols=0;
    vector<int> numbers;
    while(getline(ifs, line)){
        rows += 1;
        istringstream templine(line);
        string data;
        while (getline(templine, data, ',')){
            if(data.size() != 0){
	            if(data.at(data.size()-1) == '\r')
	                data.erase(data.size()-1);
            }
            if(data.size() != 0)
                numbers.push_back(atoi(data.c_str()));
            else
                numbers.push_back(INT_MAX);
            cols += 1;
        }
    }
    t._nCols = (rows)? cols / rows : 0;
    DBRow temprow;
    for(int i = 0; i < rows; ++i){
        temprow.reset();
        for(size_t j = 0; j < t._nCols; ++j){
            temprow.addData(numbers[i * t._nCols + j]);
        }
        t._table.push_back(temprow);
    }
    ifs.clear();
    return ifs;
}

/*****************************************/
/*   Member Functions for class DBRow    */
/*****************************************/
void
DBRow::removeCell(size_t c)
{
   // TODO
    _data.erase(_data.begin() + c);
}

/*****************************************/
/*   Member Functions for struct DBSort  */
/*****************************************/
bool
DBSort::operator() (const DBRow& r1, const DBRow& r2) const
{
   // TODO: called as a functional object that compares the data in r1 and r2
   //       based on the order defined in _sortOrder
    for(size_t i = 0; i < _sortOrder.size(); ++i){
        if(r1[_sortOrder[i]] > r2[_sortOrder[i]])
            return false;
        else if(r1[_sortOrder[i]] < r2[_sortOrder[i]])
            return true;
    }
    return true;
}

/*****************************************/
/*   Member Functions for class DBTable  */
/*****************************************/
void
DBTable::reset()
{
   // TODO
    _table.clear();
}

void
DBTable::addCol(const vector<int>& d)
{
   // TODO: add a column to the right of the table. Data are in 'd'.
    for (size_t i = 0, n = _table.size(); i < n; ++i){
        if (i < d.size())
            _table[i].addData(d[i]);
        else
            _table[i].addData(INT_MAX);
    }
}

void
DBTable::delRow(int c)
{
   // TODO: delete row #c. Note #0 is the first row.
    _table.erase(_table.begin() + c);
}

void
DBTable::delCol(int c)
{
   // delete col #c. Note #0 is the first row.
   for (size_t i = 0, n = _table.size(); i < n; ++i)
      _table[i].removeCell(c);
}

// For the following getXXX() functions...  (except for getCount())
// - Ignore null cells
// - If all the cells in column #c are null, return NAN
// - Return "float" because NAN is a float.
float
DBTable::getMax(size_t c) const
{
   // TODO: get the max data in column #c
    float max = INT_MIN;
    for(size_t i = 0; i < nRows(); ++i){
        if(max < _table[i][c] && _table[i][c] != INT_MAX)
            max = _table[i][c];
    }
    if (max == INT_MIN)
        return NAN;
    else
        return max;
}

float
DBTable::getMin(size_t c) const
{
   // TODO: get the min data in column #c
    float min = INT_MAX;
    for(size_t i = 0; i < nRows(); ++i){
        if(min > _table[i][c] && _table[i][c] != INT_MAX)
            min = _table[i][c];
    }
    if (min == INT_MAX)
        return NAN;
    else
        return min;
}

float
DBTable::getSum(size_t c) const
{
   // TODO: compute the sum of data in column #c
    float sum = 0;
    bool _nan = true;
    for(size_t i = 0; i < nRows(); ++i){
        if(_table[i][c] != INT_MAX){
            _nan = false;
            sum += _table[i][c];
        }
    }
    if (_nan)
        return NAN;
    else
        return sum;
}

int
DBTable::getCount(size_t c) const
{
   // TODO: compute the number of distinct data in column #c
   // - Ignore null cells
    vector<int> used;
    int count = 0;
    bool a;
    for(size_t i = 0; i < nRows(); ++i){
        a = true;
        if (_table[i][c] == INT_MAX)
            a = false;
        else{
            for(size_t j = 0; j < used.size(); ++j){
                if(_table[i][c] == used[j])
                    a = false;
            }
        }
        if(a){
            used.push_back(_table[i][c]);
            ++count;
        }
    }
    return count;
}

float
DBTable::getAve(size_t c) const
{
   // TODO: compute the average of data in column #c
    int n = 0;
    for(size_t i = 0; i < nRows(); ++i){
        if(_table[i][c] != INT_MAX)
            ++n;
    }
    if (!n)
        return NAN;
    else
        return getSum(c) / n;
}

void
DBTable::sort(const struct DBSort& s)
{
   // TODO: sort the data according to the order of columns in 's'
    for(size_t i = 1; i < nRows(); i += i){
        DBTable a;
        for(size_t s0 = 0; s0 < nRows(); s0 += i + i){
            int s1 = s0, e1 = min(s0 + i, nRows());
            int s2 = e1, e2 = min(s0 + i + i, nRows());
            while(s1 < e1 && s2 < e2)
                a.addRow( s(_table[s1], _table[s2]) ? _table[s1++] : _table[s2++]);
            while(s1 < e1)
                a.addRow(_table[s1++]);
            while(s2 < e2)
                a.addRow(_table[s2++]);
        }
        _table = a._table;
    }
}

void
DBTable::printCol(size_t c) const
{
   // TODO: to print out a column.
   // - Data are seperated by a space. No trailing space at the end.
   // - Null cells are printed as '.'
    size_t i;
    for(i = 0; i < nRows() - 1; ++i){
        printData(cout, _table[i][c]);
        cout << ' ';
    }
    printData(cout, _table[i][c]);
    cout << endl;
}

void
DBTable::printSummary() const
{
   size_t nr = nRows(), nc = nCols(), nv = 0;
   for (size_t i = 0; i < nr; ++i)
      for (size_t j = 0; j < nc; ++j)
         if (_table[i][j] != INT_MAX) ++nv;
   cout << "(#rows, #cols, #data) = (" << nr << ", " << nc << ", "
        << nv << ")" << endl;
}
