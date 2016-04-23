#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main() {

	string inputFile = "input.txt";
	
	ifstream fin;
	char exit;
	int operand1;
	int operand2;
	
	cout << "Running Floating Point Multiplication Emulator" << endl;
	cout << "Sam Justice - CSIS 342-001, Spring 2016 - Floating Point Multiplication Emulator" << endl << endl;

	fin.open("input.txt");

	if (!fin) {
		cout << "ERROR: Input file \"" << inputFile << "\" could not be opened." << endl;
		
		cout << "Enter any key to end execution of this program   . . .   ";
		cin >> exit;									 //to pause program
		return 0;
	}

	for (; fin >> hex >> operand1 >> operand2;)
	{
		cout << hex << operand1 << " " << operand2;
	}

	cout << endl << endl << "Enter any key to end execution of this program   . . .   ";
	cin >> exit;                                             //to pause program

	return 0;
}