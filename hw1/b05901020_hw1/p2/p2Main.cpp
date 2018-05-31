/****************************************************************************
  FileName     [ p2Main.cpp ]
  PackageName  [ p2 ]
  Synopsis     [ Define main() function ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2016-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <string>
#include <cstdlib>
#include "p2Table.h"

using namespace std;

int main()
{
   Table table;

   // Read in the csv file. Do NOT change this part of code.
   string csvFile;
   cout << "Please enter the file name: ";
   cin >> csvFile;
   if (table.read(csvFile))
      cout << "File \"" << csvFile << "\" was read in successfully." << endl;
   else {
      cerr << "Failed to read in file \"" << csvFile << "\"!" << endl;
      exit(-1); // csvFile does not exist.
   }

   // TODO read and execute commands
   string buf;
   while (true) {
	getline(cin, buf);
	if (!buf.compare("PRINT"))
	   table.print();
	else if (!buf.compare("EXIT"))
	   exit(0);
	else if (buf.size() > 1 && isalpha(buf[0]))
	{
	   size_t pos = 0;
	   if((pos = buf.find_first_of(".0123456789")) != string::npos)
	   {
		if (buf[0] == 'A' && buf[1] == 'D' && buf[2] == 'D')
		   table.add(buf, pos);
		else{
		string tempstr = buf;
		tempstr = tempstr.erase(0, pos);
		int x = stoi(tempstr.substr(0, tempstr.find(' ')));
		if (table.checknull(x))
		   cout<<"Error: This is a NULL column!!"<<endl;
		else if (buf[0] == 'S' && buf[1] == 'U' && buf[2] == 'M')
		   table.sum(x);
		else if (buf[0] == 'M' && buf[1] == 'I' && buf[2] == 'N')
		   table.min(x);
		else if (buf[0] == 'M' && buf[1] == 'A' && buf[2] == 'X')
		   table.max(x);
		else if (buf[0] == 'D' && buf[1] == 'I' && buf[2] == 'S' && buf[3] == 'T')
		   table.dist(x);
		else if (buf[0] == 'A' && buf[1] == 'V' && buf[2] == 'E')
		   table.ave(x);
		}
	   }
	}
   }
}
