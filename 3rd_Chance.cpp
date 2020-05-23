
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <iomanip>

using namespace std;

//global files
ifstream input_file;
ofstream output_file;

struct node {
	string page_number;
	int frame_number;
	bool valid, modified, referenced;

	node* next;
	//nxt will default to null if no value it present 
	node(string pg_num, int refe, node* nxt = nullptr) {
		page_number = pg_num;
		referenced = refe;
		next = nxt;
		modified = 0;
		//valid bit is always 1, there will never be a situation where it is 0
		//even if there only 4 pages, the table will not create empty frames
		//and the only way the valid bit will be 0 is if there are empty frames
		valid = 1;
	}
};

struct Page_Table {
	node* head;
	node* tail;
	int size, last_position, pageFaults;

	Page_Table() {
		head = tail = nullptr;
		size = 0;
		// -1 means list is empty
		last_position = -1;
		pageFaults = 0;
	}
	bool in_memory(int code, string pg_num) {
		node* p = head;
		while (p != nullptr) {
			//if the page number matches a page number in memory
			if (p->page_number == pg_num) {
				p->referenced = 1;
				if (code == 1) {
					p->modified = 1;
				}
				//dont insert again
				return true;
			}
			p = p->next;
		}
		return false;
	}
	void insert(int code, string pg_num) {
		if (in_memory(code, pg_num))
			return;
		//every time we need to insert something into memory, there is a fault
		pageFaults++;
		if (head == nullptr) {
			head = new node(pg_num, 1);
			//if its a write 
			if (code == 1) {
				head->modified = 1;
			}
			//node next defaults to null
			size++;
			last_position++;
			//frame number is 0 (position 0 in list)
			head->frame_number = last_position;
			tail = head;
		}
		else {
			tail->next = new node(pg_num, 1);
			tail = tail->next;
			//if its a write 
			if (code == 1) {
				tail->modified = 1;
			}
			size++;
			last_position++;
			//store frame number
			tail->frame_number = last_position;
			node* p = head;
			//set all other reference bits to 0 (except tail)
			while (p != nullptr && p != tail) {
				p->referenced = 0;
				p = p->next;
			}
		}
	}
	void remove_insert() {

		while (head != nullptr) {
			//remove node from front of list
			node* temp = head;
			head = head->next;
			temp->next = nullptr;
			//adjust all frame numbers
			node* p = head;
			while (p != nullptr) {
				p->frame_number--;
				p = p->next;
			}
			//if there is another chance
			if (temp->referenced == 1 || temp->modified == 1) {
				//if reference bit is 1 change it to zero
				if (temp->referenced == 1) {
					temp->referenced = 0;
				}
				//if modified bit is 1 change it to zero
				else if (temp->modified == 1) {
					temp->modified = 0;
				}
				//reinsert node at the rear
				tail->next = temp;
				tail = tail->next;
				tail->frame_number = last_position;
			}
			//no more chances
			else if (temp->referenced == 0 && temp->modified == 0) {
				//actually delete the node from list
				delete(temp);
				size--;
				last_position--;
				//stop
				return;
			}
		}
	}
	void print() {
		node* p = head;
		cout << endl;
		while (p != nullptr) {
			int num = strtol(p->page_number.c_str(), 0, 16);
			output_file << left << " page:" << "(" << p->page_number << ")"
				<< setw(6) << num << " frame:" << setw(2) 
				<< p->frame_number << " ref:" << p->referenced << " mod:" 
				<< p->modified << " valid:" << p->valid <<endl;
			p = p->next;
		}
		output_file << "  \nTotal Page Faults:" << pageFaults << endl;
	}
};
//*****************************************************************************
int main() {
	int code;
	string address, page_num="";
	Page_Table memory;
	input_file.open("spice.txt");
	output_file.open("output.txt");
	while(!input_file.eof()) {
		input_file >> code;
		input_file >> address;
		// if address size is 24bits, make it 32 bits
		if (address.size() == 6) {
			page_num = "00";
		}
		else {
			page_num = "";
		}
		//find the page number (remove the offset)
		for (int y = 0; y < address.size() - 3; y++) {
			page_num += address[y];
		}
		//if the memory is not full
		if (memory.size < 17)
		{
			//insert page into memory/frame
			memory.insert(code, page_num);
		}
		else {
			//find frame to remove
			memory.remove_insert();
			//insert at end of list
			memory.insert(code, page_num);
			//cout << "removed something" << endl;
		}
	}
	memory.print();
	input_file.close();
	output_file.close();
	system("Pause");
	return 0;
}