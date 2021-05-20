#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bitset>

using namespace std;

class Console {
public:
	void printPCReg(unsigned int pc, unsigned int regList[]);
	void printMemory(string addr1, string addr2, vector<string> textList, vector<string> dataList);
};