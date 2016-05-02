//Sam Justice - CSIS 342-001 - Floating Point Multiplication Emulator

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

using namespace std;

//CONSTANTS
#define SIGN_SHIFT 31
#define EXPONENT_MASK 0x7F800000
#define EXPONENT_SHIFT 23
#define SIGNIFICAND_MASK 0x007FFFFF
#define LEADING_ONE 0x800000
#define MAX_EXPONENT 254
#define MIN_EXPONENT 0
#define BIAS 127
#define POSITIVE 0
#define NEGATIVE 1
#define NEGATIVE_ZERO 0x80000000;
#define POSITIVE_ZERO 0x00000000;
#define PRECISION 6
#define WIDTH 8

//FUNCTION PROTOTYPES
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
	
	ifstream fin;		//Stream for input file
	char exit;			//Char used to hold user input when exiting the program
	
	unsigned int operand1;		//First operand
	unsigned int operand2;		//Second operand
	unsigned int product;		//Product of the operands
	
	cout << "Running Floating Point Multiplication Emulator" << endl;
	cout << "Sam Justice - CSIS 342-001, Spring 2016 - Floating Point Multiplication Emulator" << endl << endl;

	//Open the input file
	fin.open(inputFile);

	//Close the program if the input file cannot be opened
	if (!fin) {
		cout << "ERROR: Input file \"" << inputFile << "\" could not be opened." << endl << endl;
		
		cout << "Enter any key to end execution of this program   . . .   ";
		cin >> exit;									 //to pause program
		return 0;
	}

	for (; fin >> hex >> operand1 >> operand2;)
	{
		//Display the decimal equivalent of operand1
		if (operand1 == 0)
		{
			//Display 0x0 with 8 zeroes
			cout << hex << setfill('0') << setw(WIDTH) << "0" << " is equivalent to IEEE: " << scientific 
				<< setprecision(PRECISION) << *(reinterpret_cast<float*>(&operand1)) << endl;
		}
		else
		{
			cout << hex << uppercase << setfill('0') << setw(WIDTH) << operand1 << " is equivalent to IEEE: "
				<< scientific << setprecision(PRECISION) << *(reinterpret_cast<float*>(&operand1)) << endl;
		}

		//Display the decimal equivalent of operand2
		if (operand2 == 0)
		{
			//Display 0x0 with 8 zeroes
			cout << hex << setfill('0') << setw(WIDTH) << "0" << " is equivalent to IEEE: " << scientific
				<< setprecision(PRECISION) << *(reinterpret_cast<float*>(&operand2)) << endl;
		}
		else
		{
			cout << hex << uppercase << setfill('0') << setw(WIDTH) << operand2 << " is equivalent to IEEE: "
				<< scientific << setprecision(PRECISION) << *(reinterpret_cast<float*>(&operand2)) << endl;
		}

		try
		{
			product = multiply(operand1, operand2);

			//Display the hexidecimal product and its decimal equivalent
			if (product == 0)
			{
				//Display 0x0 with 8 zeroes
				cout << "The product is " << hex << setfill('0') << setw(WIDTH) << "0" << ", which is equivalent to "
					<< scientific << setprecision(PRECISION) << *(reinterpret_cast<float*>(&product)) 
					<< endl << endl;
			}
			else
			{
				cout << "The product is " << hex << uppercase << setfill('0') << setw(WIDTH) << product
					<< ", which is equivalent to " << scientific << setprecision(PRECISION) 
					<< *(reinterpret_cast<float*>(&product)) << endl << endl;
			}
		}
		catch (exception& error)
		{
			//Print to the console that an overflow or underflow error occurred
			cout << error.what() << endl << endl;
		}
	}

	cout << endl << "Enter any key to end execution of this program   . . .   ";
	cin >> exit;                                             //to pause program

	return 0;
}

unsigned int multiply(unsigned int operand1, unsigned int operand2)
{
	bool addOneToExponent = false;

	unsigned int newSign;
	unsigned int significandProduct;
	unsigned int exponentSum;

	//If either operand is zero, return 0 as the product
	if (isZero(operand1) || isZero(operand2))
	{
		//If one of the operands is negative, return negative zero (0x80000000)
		if (getNewSign(operand1, operand2) == NEGATIVE)
			return NEGATIVE_ZERO;
		
		return POSITIVE_ZERO;	//Otherwise, return positive zero (0x00000000)
	}

	significandProduct = getSignificandProduct(operand1, operand2, addOneToExponent);	//Multiply the significands
	exponentSum = getExponentSum(operand1, operand2, addOneToExponent);			//Add the exponents
	newSign = getNewSign(operand1, operand2);									//Determine the sign of the product

	//Combine the new significand, new exponent, and new sign to obtain the product
	return ((newSign << SIGN_SHIFT) | (exponentSum << EXPONENT_SHIFT) | significandProduct);
}

unsigned int getSignificandProduct(unsigned int operand1, unsigned int operand2, bool& addOneToExponent)
{
	unsigned int significandOp1;
	unsigned int significandOp2;
	unsigned long long significandProduct;

	significandOp1 = getSignificand(operand1);			//Get operand1's significand
	significandOp1 = significandOp1 | LEADING_ONE;		//Add a 1 to the front of the significand

	significandOp2 = getSignificand(operand2);			//Get operand2's significand
	significandOp2 = significandOp2 | LEADING_ONE;		//Add a 1 to the front of the significand

	//Multiply the significands and put the product into an unsigned long long
	significandProduct = unsigned long long(significandOp1) * unsigned long long(significandOp2);	
	
	//Normalize the product of multiplying the significands
	significandProduct = normalize(significandProduct, addOneToExponent);

	return significandProduct;
}

unsigned long long normalize(unsigned long long significand, bool& addOneToExponent)
{
	//Check if the 48th bit is a one
	if ((significand & 0x800000000000) >> 47 == 1)
	{
		//Mask off the 48th bit
		significand = significand & 0x7FFFFFFFFFFF;

		//If the bits beyond the 23 that can be stored amount to more than one-half of the last
		//representable bit, add one to the last representable bit
		if ((significand & 0xFFFFFF) > 0x800000)
		{
			significand = significand >> 24;
			significand++;
		}
		//If the bits beyond the 23 that can be stored amount to exactly half of the last 
		//representable bit
		else if ((significand & 0xFFFFFF) == 0x800000)
		{
			//Round the value up if the last representable bit is currently 1
			if ((significand & 0x1000000) == 1)
			{
				significand = significand >> 24;
				significand++;
			}
			//Do not round the value up if the last representable bit is a 0
			else
				significand = significand >> 24;
		}
		//If the bits beyond the 23 that can be stored amount to less than half of the last 
		//representable bit, truncate the extra bits
		else
		{
			significand = significand >> 24;
		}
		
		//The 48th bit is added to the exponent
		addOneToExponent = true;
	}
	else
	{
		//Mask off the 48th and 47th bit
		significand = significand & 0x3FFFFFFFFFFF;
		
		//If the bits beyond the 23 that can be stored amount to more than one-half of the last
		//representable bit, add one to the last representable bit
		if ((significand & 0x7FFFFF) > 0x400000)
		{
			significand = significand >> 23;
			significand++;
		}
		//If the bits beyond the 23 that can be stored amount to exactly half of the last 
		//representable bit
		else if ((significand & 0x7FFFFF) == 0x400000)
		{
			//Round the value up if the last representable bit is currently 1
			if ((significand & 0x800000) == 1)
			{
				significand = significand >> 23;
				significand++;
			}
			//Do not round the value up if the last representable bit is a 0
			else
				significand = significand >> 23;
		}
		//If the bits beyond the 23 that can be stored amount to less than half of the last 
		//representable bit, truncate the extra bits
		else
		{
			significand = significand >> 23;
		}
		
		//The 48th bit is not added to the exponent
		addOneToExponent = false;
	}

	return significand;
}

unsigned int getExponentSum(unsigned int operand1, unsigned int operand2, bool addOneToExponent)
{
	unsigned int exponentOp1;
	unsigned int exponentOp2;
	unsigned int exponentSum;
	int signedExponentSum;
	
	exponentOp1 = getExponent(operand1);	//Get operand1's exponent
	exponentOp2 = getExponent(operand2);	//Get operand2's exponent

	//Get the signed sum of the exponents to determine if overflow or underflow has occurred
	signedExponentSum = getSignedExponentSum(exponentOp1, exponentOp2, addOneToExponent);

	//Throw an error if overflow or underflow occurs
	if (signedExponentSum > MAX_EXPONENT)
		throw exception("Error: Overflow occurred in the result!");
	else if (signedExponentSum < MIN_EXPONENT)
		throw exception("Error: Underflow occurred in the result!");

	//If the 48th bit of the product of the significands was a 1, add it to the sum of the exponents
	if (addOneToExponent)
		exponentSum = exponentOp1 + (exponentOp2 - BIAS) + 1;
	else
		exponentSum = exponentOp1 + (exponentOp2 - BIAS);

	return exponentSum;
}

int getSignedExponentSum(unsigned int exponent1, unsigned int exponent2, bool addOneToExponent)
{
	int signedExpOp1 = exponent1;
	int signedExpOp2 = exponent2;
	int signedSum;

	//If the 48th bit of the product of the significands was a 1, add it to the sum of the exponents
	if (addOneToExponent)
		signedSum = signedExpOp1 + (signedExpOp2 - BIAS) + 1;
	else
		signedSum = signedExpOp1 + (signedExpOp2 - BIAS);
	
	return signedSum;
}

unsigned int getNewSign(unsigned int operand1, unsigned int operand2)
{
	if (getSign(operand1) == getSign(operand2))
		return POSITIVE;			//If the signs are the same, return a positive sign
	else
		return NEGATIVE;			//Otherwise, return a negative sign
}

unsigned int getSign(unsigned int operand)
{
	return operand >> SIGN_SHIFT;	//Get the sign of operand
}

unsigned int getExponent(unsigned int operand)
{
	int masked;

	masked = operand & EXPONENT_MASK;

	return masked >> EXPONENT_SHIFT;		//Get the exponent of operand
}

unsigned int getSignificand(unsigned int operand)
{
	int masked;

	masked = operand & SIGNIFICAND_MASK;

	return masked;		//Get the significand of operand
}

bool isZero(unsigned int operand)
{
	//Determine if the operand has the value of zero
	if (getExponent(operand) == 0 && getSignificand(operand) == 0)
		return true;
	else
		return false;
}