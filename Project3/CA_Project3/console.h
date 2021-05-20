#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bitset>

using namespace std;

class Console {
public:
	unsigned int signExtention(unsigned int number);
	
	unsigned int ifStage(unsigned int pc, vector<string> textList, unsigned int ifidRegList[]);
	unsigned int idStage(unsigned int ifidRegList[], unsigned int controlUnit[], unsigned int regList[], unsigned int idexRegList[]);
	unsigned int exStage(unsigned int idexRegList[], unsigned int exmemRegList[], unsigned int regList[]);
	unsigned int memStage(unsigned int exmemRegList[], vector<string> textList, vector<string> dataList, unsigned int memwbRegList[], unsigned int idexRegList[], unsigned int regList[]);
	unsigned int wbStage(unsigned int controlUnit[], unsigned int memwbRegList[], unsigned int regList[]);

	unsigned int* controlHarzard(unsigned int pc, unsigned int ifidRegList[], unsigned int controlUnit[], unsigned int regList[], int takenFlag, unsigned int idexRegList[]);

	void printStatePC(unsigned int pcList[]);
	void printPCReg(unsigned int pc, unsigned int regList[]);
	void printMemory(string addr1, string addr2, vector<string> textList, vector<string> dataList);
};