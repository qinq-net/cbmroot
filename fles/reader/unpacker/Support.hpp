#ifndef SUPPORT_HPP
#define SUPPORT_HPP

#include <string>
//#include <cstdio>
#include <iostream>

std::string GetBinaryRepresentation(size_t const size, void const * const ptr)
{
	std::string outString;

	unsigned char *b = (unsigned char*) ptr;
	unsigned char byte;

	char cStr[2];
	cStr[1] = '\0';

	for (int i=size-1; i>=0; i--) {
		for (int j=7; j>=0; j--) {
			byte = (b[i] >> j) & 1;
			sprintf(cStr, "%u", byte);
			outString.append(cStr);
		}
	}

	return outString;
}

/**
 * size in bytes
 */
std::string GetHexRepresentation(size_t const size, void const * const ptr)
{
	std::string outString;

	unsigned char *b = (unsigned char*) ptr;
	unsigned char byte;

	char cStr[3];
	cStr[2] = '\0';

	for (int i=size-1; i>=0; i--) {
		byte = b[i] & 0xff;
		sprintf(cStr, "%02x", byte);
		outString.append(cStr);
	}

	return outString;
}

std::string GetWordHexRepr(void const * const ptr)
{
	std::string outString;

	unsigned char *b = (unsigned char*) ptr;
	unsigned char byte[4];
	byte[0] = b[3] & 0xff;
	byte[1] = b[2] & 0xff;
	byte[2] = b[1] & 0xff;
	byte[3] = b[0] & 0xff;

	char cStr[10];
	cStr[9] = '\0';

	sprintf(cStr, "%02x%02x %02x%02x", byte[0], byte[1], byte[2], byte[3]);

	outString.append(cStr);

	return outString;
}

std::string GetWordHexReprInv(void const * const ptr)
{
	std::string outString;

	unsigned char *b = (unsigned char*) ptr;
	unsigned char byte[4];
	byte[0] = b[0] & 0xff;
	byte[1] = b[1] & 0xff;
	byte[2] = b[2] & 0xff;
	byte[3] = b[3] & 0xff;

	char cStr[10];
	cStr[9] = '\0';

	sprintf(cStr, "%02x%02x %02x%02x", byte[0], byte[1], byte[2], byte[3]);

	outString.append(cStr);

	return outString;
}

void SwapBytes(size_t const size, void const * ptr)
{
	unsigned char *b = (unsigned char*) ptr;
	unsigned char byte[4];
	byte[0] = b[3] & 0xff;
	byte[1] = b[2] & 0xff;
	byte[2] = b[1] & 0xff;
	byte[3] = b[0] & 0xff;

	b[0] = byte[0];
	b[1] = byte[1];
	b[2] = byte[2];
	b[3] = byte[3];
}

void PrintRaw(size_t const size, void const * const ptr)
{
	size_t nWords = size/4;
	size_t nRestBytes = size%4;

	for (size_t iWord=0; iWord<nWords; iWord++) {
		//std::cout << GetHexRepresentation(4, ptr+iWord*4) << " ";
		
		std::cout << GetWordHexReprInv(ptr+iWord*4) << " ";
	}
	/*if (nRestBytes > 0) {
		std::cout << GetHexRepresentation(nRestBytes, ptr+nWords*4) << " " << std::endl;
	} else {
		std::cout << std::endl;
	}*/
	std::cout << std::endl;
}

#endif // SUPPORT_HPP
