#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bitset>

using namespace std;

class Format {// 산술 명령어 형식
public:
	string rform2bin(string inst, string op, string rs, string rt, string rd, string shamt, string funct);
	string iform2bin(string inst, string op, string rs, string rt, string immediate);
	string jform2bin(string inst, string op, string jump_target);
};