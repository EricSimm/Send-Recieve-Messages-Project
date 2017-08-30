


#include <stdio.h>      /* printf */
#include <time.h>       /* time_t, struct tm, difftime, time, mktime */
#include <iostream>
#include<stdio.h>
#include<time.h>
#include<fstream>
#include<vector>
using namespace std;


//structure for our processes
struct proc
{
	int id;
	int admittance_time;
	int completion_time;
	int num_pages;		//number of pages
	vector<int> mem_pages;	//how much memory each page takes
	
	
}

//used admit processes into our input queue
//holds process information temporarily for processes that are waiting to be put into the input queue
class admit_processes
{
Public:

admit(); //set _read to false
~admit();

void read(&istream, int, vector<proc>, int); //checks

proc _next;

private:
bool _read;


};

//probably should use arguments to set the istream and total memory size
int main () {
	admit_processes bouncer; //prevents processes from entering the input queue too early
	vector<proc> in_queue;   //our input queue
	istream in_file;	//inputstream for our in file
	in_file.open("in1.txt");

	int to_be_read; //total number of processes that have yet to be read
	int total_memory_size = 2000;

	clock_t start;	//start keeps track of the time since the program started in milliseconds
	start = clock(); 

	istream >> to_be_read; //find the total # of procceses

	while(to_be_read > 0 || clock()-start <= 100000)
	{
		bouncer.admit(in_file, start, in_queue, to_be_read);
	}

}

admit::admit(){_read = false;};
~admit::admit(){};

void admit::read(&istream instream, int start, vector<proc> in_que, int to_be_read)
{
	if(_read == false)
	{
		_next.mem_pages.clear();	//clear vector that holds memory pages
		instream >> _next.id >> _next.id;
		instream >> _next.admittence_time;
		instream >> _next.num_pages;
		for(int i = 0; i < _next.num_pages; i++)	//assign new pages
		{
			int temp;
			instream >> temp;
			_next.mem_pages.assign(1,temp);
		}
	}
	if(clock()-start >= _next.admittence_time)
	{
		_next.admittence_time = clock()-start;
		in_que.assgin(1,_next);
		_read = false;
		to_be_read--;
	}
	else
		_read = true;
		
	
}
