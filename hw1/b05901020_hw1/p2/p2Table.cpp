/****************************************************************************
  FileName     [ p2Table.cpp ]
  PackageName  [ p2 ]
  Synopsis     [ Define member functions of class Row and Table ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2016-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include "p2Table.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <climits>
#include <iomanip>

using namespace std;

// Implement member functions of class Row and Table here
Row::Row(int cols)
{
   _data = new int [cols];
}
bool Table::read(const string& csvFile)
{
   ifstream file;
   const char *cstr = csvFile.c_str();	
   file.open (cstr);
   if(!file)
	return false;
   else{
   string line;
   int rows=0, cols=0;
   vector<int> numbers;
   while (getline (file, line, '\n'))
   {
	rows+=1;
	istringstream templine (line);
	string data;
	while (getline (templine, data, ','))
	{
	   if(data.size()!=0){
		if(data.at(data.size()-1) == '\r')
		   data.erase(data.size()-1);
	   }
	   if(data.size()!=0)
		numbers.push_back(atoi(data.c_str()));
	   else
		numbers.push_back(INT_MAX);
	   cols+=1;
	}
   }
   _nRows=rows;
   _nCols=cols/rows;
   
   Row daata(nCols());
   for (size_t i=0; i<nRows(); i++){
	for(size_t j=0; j<nCols(); j++){
	   daata[j] = numbers[i*nCols()+j];
	}
	Row temprow(nCols());
	for(size_t k=0; k<nCols(); k++){
	   temprow[k]=daata[k];
	}
	_rows.push_back(temprow);
   }
   file.close();
   return true; // TODO
}}
void Table::print()
{
   for (size_t i=0; i<nRows(); i++){
	for(size_t j=0; j<nCols(); j++){
	   if(_rows[i][j] != INT_MAX)
		cout<<right<<setw(4)<<_rows[i][j];
	   else
		cout<<right<<setw(4)<<".";
	}
	cout<<endl;
   }
}
void Table::sum(int cols)
{
   int sum = 0;
   for (size_t i=0; i<nRows(); i++){
	if(_rows[i][cols] != INT_MAX)
	   sum += _rows[i][cols];
   }
   cout<<"The summation of data in column #"<<cols<<" is "<<sum<<"."<<endl;
}
void Table::max(int cols)
{
   int max = INT_MIN;
   for (size_t i=0; i<nRows(); i++){
	if(_rows[i][cols] != INT_MAX && _rows[i][cols] > max)
	   max = _rows[i][cols];
   }
   cout<<"The maximum of data in column #"<<cols<<" is "<<max<<"."<<endl;
}
void Table::min(int cols)
{
   int min = INT_MAX;
   for (size_t i=0; i<nRows(); i++){
	if(_rows[i][cols] != INT_MAX && _rows[i][cols] < min)
	   min = _rows[i][cols];
   }
   cout<<"The minimum of data in column #"<<cols<<" is "<<min<<"."<<endl;
}
void Table::dist(int cols)
{
   int count = 0;
   vector<int> used;
   bool a;
   for (size_t i=0; i<nRows(); i++){
	if(_rows[i][cols] != INT_MAX){
	   a = true;
	   for (size_t j=0; j<used.size(); j++){
		if(_rows[i][cols] == used[j])
		   a=false;
	   }
	   if(a){
		count++;
		used.push_back(_rows[i][cols]);
	   }
	}
   }
   cout<<"The distinct count of data in column #"<<cols<<" is "<<count<<"."<<endl;
}
void Table::ave(int cols)
{
   float sum=0, row=0, ave=0;
   for (size_t i=0; i<nRows(); i++){
	if(_rows[i][cols] != INT_MAX){
	   sum += _rows[i][cols];
	   row++;
	}
   }
   ave = sum/row;
   cout<<fixed<<setprecision(1);
   cout<<"The average of data in column #"<<cols<<" is "<<ave<<"."<<endl;
}
bool Table::checknull(int cols)
{
   for (size_t i=0; i<nRows(); i++){
	if(_rows[i][cols] != INT_MAX){
	   return false;
	}
   }  
   return true;
}
void Table::add(string& buf, size_t pos)
{
   Row newrow(nCols());
   string tempstr = buf.substr(pos);
   for (size_t i=0; i<nCols(); i++){
	if(tempstr.substr(0, tempstr.find(' ')) != "."){
	   int data = stoi(tempstr.substr(0, tempstr.find(' ')));
	   newrow[i] = data;
	}
	else
	   newrow[i] = INT_MAX;
	tempstr.erase(0, tempstr.find(' '));
	tempstr.erase(0, tempstr.find_first_of(".0123456789"));
   }
   _rows.push_back(newrow);
   _nRows+=1;
}
