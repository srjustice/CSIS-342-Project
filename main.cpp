#include <iostream>
#include <iomanip>
#include <fstream>
#include <bitset>
#include <string>

using namespace std;

#define SIGNSHIFT 31
#define EXPONENTMASK 0x7F800000
#define EXPONENTSHIFT 23
#define SIGNIFICANDMASK 0x007FFFFF
#define LEADINGONE 0x800000

unsigned int getSign(unsigned int operand);
unsigned int getExponent(unsigned int operand);
unsigned int getSignificand(unsigned int operand);
unsigned int multiply(unsigned int operand1, unsigned int operand2);
unsigned long long normalize(unsigned long long significand, bool& addOneToExponent);
unsigned int getSignificandProduct(unsigned int operand1, unsigned int operand2, bool& addOneToExponent);
unsigned int getExponentSum(unsigned int operand1, unsigned int operand2, bool addOneToExponent);
int getSignedExponentSum(unsigned int exponent1, unsigned int exponent2, bool addOneToExponent);
unsigned int getNewSign(unsigned int operand1, unsigned int operand2);
bool isZero(unsigned int operand);

int main() 
{

	string inputFile = "input.txt";
	
	ifstream fin;
	char exit;
	
	unsigned int operand1;
	unsigned int operand2;

	unsigned int signOp1;
	unsigned int exponentOp1;
	unsigned int significandOp1;

	unsigned int signOp2;
	unsigned int exponentOp2;
	unsigned int significandOp2;
	
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
		unsigned int product;

		cout << hex << uppercase << operand1 << " is equivalent to IEEE: " << scientific << setprecision(7) 
			<< *(reinterpret_cast<float*>(&operand1)) << endl;
		cout << hex << uppercase << operand2 << " is equivalent to IEEE: " << scientific << setprecision(7) 
			<< *(reinterpret_cast<float*>(&operand2)) << endl;

		try
		{
			product = multiply(operand1, operand2);
			
			cout << "The product is " << hex << uppercase << product << ", which is equivalent to " << scientific << setprecision(7)
				<< *(reinterpret_cast<float*>(&product)) << endl << endl;
		}
		catch (exception& error) 
		{
			cout << error.what() << endl << endl;
		}
	}

	cout << endl << "Enter any key to end execution of this program   . . .   ";
	cin >> exit;                                             //to pause program

	return 0;
}

unsigned int getSign(unsigned int operand) 
{
	return operand >> SIGNSHIFT;
}

unsigned int getExponent(unsigned int operand)
{
	int masked;
	
	masked = operand & EXPONENTMASK;

	return masked >> EXPONENTSHIFT;
}

unsigned int getSignificand(unsigned int operand)
{
	int masked;

	masked = operand & SIGNIFICANDMASK;

	return masked;
}

unsigned int multiply(unsigned int operand1, unsigned int operand2)
{
	bool addOneToExponent = false;
	
	unsigned int newSign;
	unsigned int significandProduct;
	unsigned int exponentSum;

	if (isZero(operand1) || isZero(operand2))
		return 0;

	significandProduct = getSignificandProduct(operand1, operand2, addOneToExponent);
	exponentSum = getExponentSum(operand1, operand2, addOneToExponent);
	newSign = getNewSign(operand1, operand2);


	return ((newSign << 31) | (exponentSum << 23) | significandProduct);
}


unsigned long long normalize(unsigned long long significand, bool& addOneToExponent)
{
	significand = significand << ((sizeof(long long) * 8) - 48);

	if (significand >> ((sizeof(long long) * 8) - 1) == 1)
	{
		significand = significand << 1;
		significand = significand >> 25 + ((sizeof(long long) * 8) - 48);
		addOneToExponent = true;
	}
	else
	{
		significand = significand << 2;
		significand = significand >> 25 + ((sizeof(long long) * 8) - 48);
		addOneToExponent = false;
	}

	return significand;
}

unsigned int getSignificandProduct(unsigned int operand1, unsigned int operand2, bool& addOneToExponent)
{
	unsigned int significandOp1;
	unsigned int significandOp2;
	unsigned long long significandProduct;

	significandOp1 = getSignificand(operand1);
	significandOp1 = significandOp1 | LEADINGONE;

	significandOp2 = getSignificand(operand2);
	significandOp2 = significandOp2 | LEADINGONE;

	significandProduct = unsigned long long(significandOp1) * unsigned long long(significandOp2);
	significandProduct = normalize(significandProduct, addOneToExponent);

	return significandProduct;
}

unsigned int getExponentSum(unsigned int operand1, unsigned int operand2, bool addOneToExponent)
{
	unsigned int exponentOp1;
	unsigned int exponentOp2;
	unsigned int exponentSum;
	int signedExponentSum;
	
	exponentOp1 = getExponent(operand1);
	exponentOp2 = getExponent(operand2);

	signedExponentSum = getSignedExponentSum(exponentOp1, exponentOp2, addOneToExponent);

	if (signedExponentSum > 254)
		throw exception("Error: Overflow occurred in the result!");
	else if (signedExponentSum < 0)
		throw exception("Error: Underflow occurred in the result!");

	if (addOneToExponent)
		exponentSum = exponentOp1 + (exponentOp2 - 127) + 1;
	else
		exponentSum = exponentOp1 + (exponentOp2 - 127);

	return exponentSum;
}

int getSignedExponentSum(unsigned int exponent1, unsigned int exponent2, bool addOneToExponent)
{
	int signedExpOp1 = exponent1;
	int signedExpOp2 = exponent2;
	int signedSum;

	if (addOneToExponent)
		signedSum = signedExpOp1 + (signedExpOp2 - 127) + 1;
	else
		signedSum = signedExpOp1 + (signedExpOp2 - 127);
	
	return signedSum;
}

unsigned int getNewSign(unsigned int operand1, unsigned int operand2)
{
	if (getSign(operand1) == getSign(operand2))
		return 0;
	else
		return 1;
}

bool isZero(unsigned int operand)
{
	if (getExponent(operand) == 0 && getSignificand(operand) == 0)
		return true;
	else
		return false;

}