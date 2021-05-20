#include "format.h"

string Format::rform2bin(string inst, string op, string rs, string rt, string rd, string shamt, string funct) {
    string strInst; // 00100100001000000100010010010000 32bit

    strInst = bitset<6>(stoi(op)).to_string() + bitset<5>(stoi(rs)).to_string() + bitset<5>(stoi(rt)).to_string()
        + bitset<5>(stoi(rd)).to_string() + bitset<5>(stoi(shamt)).to_string() + bitset<6>(stoi(funct)).to_string();

    return strInst;
}

string Format::iform2bin(string inst, string op, string rs, string rt, string immediate) {
    string strInst; // 00100100001000000100010010010000 32bit

    if (immediate.find("x") == 1)
        strInst = bitset<6>(stoi(op)).to_string() + bitset<5>(stoi(rs)).to_string()
        + bitset<5>(stoi(rt)).to_string() + bitset<16>(stoi(immediate, nullptr, 16)).to_string();
    else
        strInst = bitset<6>(stoi(op)).to_string() + bitset<5>(stoi(rs)).to_string()
        + bitset<5>(stoi(rt)).to_string() + bitset<16>(stoi(immediate)).to_string();

    return strInst;
}

string Format::jform2bin(string inst, string op, string jump_target) {
    string strInst; // 00100100001000000100010010010000 32bit

    strInst = bitset<6>(stoi(op)).to_string() + bitset<26>(stoi(jump_target, nullptr, 16)).to_string();

    return strInst;
}