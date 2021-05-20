#include "console.h"

int main(int argc, char* argv[]) {
    Console cons;

    ifstream inputFile;

    /* 시작 전 인자 받기 */     

    int memoryFlag = 0;
    string addr1, addr2;
    int dynamicFlag = 0;
    int num_instruction = -1;
    string inputFileName;
    
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (arg == "-m") {
            arg = argv[++i];

            memoryFlag = 1;
            addr1 = arg.substr(0, arg.find(":"));
            addr2 = arg.substr(arg.find(":") + 1);
        }
        else if (arg == "-d") {
            dynamicFlag = 1;
        }
        else if (arg == "-n") {
            num_instruction = stoul(argv[++i]);
        }
        else {
            inputFileName = arg;
        }
    }

    inputFile.open(inputFileName);

    while (!inputFile.is_open()) {
        cout << "The file name is incorrect. Please re-enter it.\n";
        return 0;
    }
    
    /* 시작 전 읽기 */

    string fileLine;

    unsigned int textSecNum, dataSecNum;
    vector<string> textList, dataList; // address, value(hex, 1 byte)

    inputFile >> fileLine; // text section num
    textSecNum = stoul(fileLine, nullptr, 16) / 4;

    inputFile >> fileLine; // data section num
    dataSecNum = stoul(fileLine, nullptr, 16) / 4;

    for (int i = 0; i < textSecNum; ++i) {
        inputFile >> fileLine;

        string textHex = fileLine.substr(2);
        int textLength = textHex.length();

        if (textLength < 8)
            for (int j = 0; j < 8 - textLength; ++j)
                textHex = "0" + textHex;

        textList.push_back(textHex.substr(0, 2));
        textList.push_back(textHex.substr(2, 2));
        textList.push_back(textHex.substr(4, 2));
        textList.push_back(textHex.substr(6, 2));
    }

    textList.push_back("00"); // textSecNum * 4 + 1
    textList.push_back("00"); // textSecNum * 4 + 2
    textList.push_back("00"); // textSecNum * 4 + 3
    
    for (int i = 0; i < dataSecNum; ++i) {
        inputFile >> fileLine;

        string dataHex = fileLine.substr(2);
        int dataLength = dataHex.length();

        if (dataLength < 8)
            for (int j = 0; j < 8 - dataLength; ++j)
                dataHex = "0" + dataHex;

        dataList.push_back(dataHex.substr(0, 2));
        dataList.push_back(dataHex.substr(2, 2));
        dataList.push_back(dataHex.substr(4, 2));
        dataList.push_back(dataHex.substr(6, 2));
    }

    dataList.push_back("00"); // dataSecNum * 4 + 1
    dataList.push_back("00"); // dataSecNum * 4 + 2
    dataList.push_back("00"); // dataSecNum * 4 + 3
    
    inputFile.close();
    
    /* 진정한 시작 */
    
    int i = 0;
    unsigned int pc = 0x400000;
    unsigned int regList[32];
    fill_n(regList, 32, 0);

    while(pc < 0x400000 + textList.size() - 3) {
        if (i == num_instruction) {
            break;
        }

        pc += 4;

        unsigned int index = pc - 0x400004;
        string instValue = textList[index] + textList[index + 1] + textList[index + 2] + textList[index + 3];
        string instBin = bitset<32>(stoul(instValue, nullptr, 16)).to_string();

        unsigned int op = stoul(instBin.substr(0, 6), nullptr, 2);

        if (op == 0) { // R format
            unsigned int rs = stoul(instBin.substr(6, 5), nullptr, 2);
            unsigned int rt = stoul(instBin.substr(11, 5), nullptr, 2);
            unsigned int rd = stoul(instBin.substr(16, 5), nullptr, 2);
            unsigned int shamt = stoul(instBin.substr(21, 5), nullptr, 2);
            unsigned int funct = stoul(instBin.substr(26), nullptr, 2);

            if (funct == 33) { // addu
                regList[rd] = regList[rs] + regList[rt];
            }
            else if (funct == 36) { // and
                regList[rd] = regList[rs] & regList[rt];
            }
            else if (funct == 39) { // nor
                regList[rd] = ~(regList[rs] | regList[rt]);
            }
            else if (funct == 37) { // or
                regList[rd] = regList[rs] | regList[rt];
            }
            else if (funct == 43) { // sltu
                regList[rd] = regList[rs] < regList[rt];
            }
            else if (funct == 0) { // sll
                regList[rd] = regList[rt] << shamt;
            }
            else if (funct == 2) { // srl
                regList[rd] = regList[rt] >> shamt;
            }
            else if (funct == 35) { // subu
                regList[rd] = regList[rs] - regList[rt];
            }
            else if (funct == 8) { // jr
                pc = regList[rs];
            }
        }
        else if (op == 2 || op == 3) { // J format
            unsigned int target = stoul(instBin.substr(6), nullptr, 2);

            if (op == 2) { // j
                pc = target << 2;
            }
            else if (op == 3) { // jal
                regList[31] = pc;
                pc = target << 2;
            }
        }
        else { // I format
            unsigned int rs = stoul(instBin.substr(6, 5), nullptr, 2);
            unsigned int rt = stoul(instBin.substr(11, 5), nullptr, 2);
            short int immediate = stoul(instBin.substr(16), nullptr, 2);

            unsigned int target = regList[rs] + immediate;

            if (op == 15) { // lui
                regList[rt] = immediate << 16;
            }
            else if (op == 35) { // lw
                int isFound = 0;

                for (int j = 0; j < textList.size() - 3; ++j) {
                    if (0x400000 + j == target) {
                        isFound = 1;

                        string textValue = textList[j] + textList[j + 1] + textList[j + 2] + textList[j + 3];
                        unsigned int textHex = stoul(textValue, nullptr, 16);

                        regList[rt] = textHex;

                        break;
                    }
                }

                for (int j = 0; j < dataList.size() - 3; ++j) {
                    if (0x10000000 + j == target) {
                        isFound = 1;

                        string dataValue = dataList[j] + dataList[j + 1] + dataList[j + 2] + dataList[j + 3];
                        unsigned int dataHex = stoul(dataValue, nullptr, 16);

                        regList[rt] = dataHex;

                        break;
                    }
                }

                if (isFound != 1) {
                    regList[rt] = 0;
                }
            }
            else if (op == 32) { // lb
                int isFound = 0;

                for (int j = 0; j < textList.size() - 3; ++j) {
                    if (0x400000 + j == target) {
                        isFound = 1;

                        string textValue = textList[j];
                        string textBin = bitset<8>(stoul(textValue, nullptr, 16)).to_string();

                        if (textBin[0] == 0) {
                            textValue = "000000" + textValue;
                        }
                        else {
                            textValue = "ffffff" + textValue;
                        }

                        int textHex = stoul(textValue, nullptr, 16);

                        regList[rt] = textHex;

                        break;
                    }
                }

                for (int j = 0; j < dataList.size() - 3; ++j) {
                    if (0x10000000 + j == target) {
                        isFound = 1;

                        string dataValue = dataList[j];
                        string dataBin = bitset<8>(stoul(dataValue, nullptr, 16)).to_string();

                        if (dataBin[0] == 0) {
                            dataValue = "000000" + dataValue;
                        }
                        else {
                            dataValue = "ffffff" + dataValue;
                        }

                        int dataHex = stoul(dataValue, nullptr, 16);

                        regList[rt] = dataHex;

                        break;
                    }
                }

                if (isFound != 1) {
                    regList[rt] = 0;
                }
            }
            else if (op == 43) { // sw
                int isFound = 0;

                string rtBin = bitset<32>(regList[rt]).to_string();

                string rtHex = "";
                string hexList[16] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f" };

                for (int j = 0; j < 8; ++j) {
                    rtHex += hexList[stoul(rtBin.substr(j * 4, 4), nullptr, 2)];
                }

                for (int j = 0; j < textList.size() - 3; ++j) {
                    if (0x400000 + j == target) {
                        isFound = 1;

                        textList[j + 0] = rtHex.substr(0, 2);
                        textList[j + 1] = rtHex.substr(2, 2);
                        textList[j + 2] = rtHex.substr(4, 2);
                        textList[j + 3] = rtHex.substr(6, 2);

                        break;
                    }
                }

                for (int j = 0; j < dataList.size() - 3; ++j) {
                    if (0x10000000 + j == target) {
                        isFound = 1;

                        dataList[j + 0] = rtHex.substr(0, 2);
                        dataList[j + 1] = rtHex.substr(2, 2);
                        dataList[j + 2] = rtHex.substr(4, 2);
                        dataList[j + 3] = rtHex.substr(6, 2);

                        break;
                    }
                }

                if (isFound != 1) {
                    if (target < 0x10000000) {
                        textList.pop_back();
                        textList.pop_back();
                        textList.pop_back();

                        int offset = target - 0x400000;
                        int textLength = textList.size();

                        for (int j = 0; j < offset - textLength; ++j) {
                            textList.push_back("00");
                        }

                        textList.push_back(rtHex.substr(0, 2));
                        textList.push_back(rtHex.substr(2, 2));
                        textList.push_back(rtHex.substr(4, 2));
                        textList.push_back(rtHex.substr(6, 2));

                        textList.push_back("00");
                        textList.push_back("00");
                        textList.push_back("00");
                    }
                    else {
                        dataList.pop_back();
                        dataList.pop_back();
                        dataList.pop_back();

                        int offset = target - 0x10000000;
                        int dataLength = dataList.size();

                        for (int j = 0; j < offset - dataLength; ++j) {
                            dataList.push_back("00");
                        }

                        dataList.push_back(rtHex.substr(0, 2));
                        dataList.push_back(rtHex.substr(2, 2));
                        dataList.push_back(rtHex.substr(4, 2));
                        dataList.push_back(rtHex.substr(6, 2));

                        dataList.push_back("00");
                        dataList.push_back("00");
                        dataList.push_back("00");
                    }
                }
            }
            else if (op == 40) { // sb
                int isFound = 0;

                string rtBin = bitset<32>(regList[rt]).to_string();

                string rtHex = "";
                string hexList[16] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f" };

                for (int j = 0; j < 8; ++j) {
                    rtHex += hexList[stoul(rtBin.substr(j * 4, 4), nullptr, 2)];
                }

                for (int j = 0; j < textList.size() - 3; ++j) {
                    if (0x400000 + j == target) {
                        isFound = 1;

                        textList[j + 0] = rtHex.substr(6, 2);

                        break;
                    }
                }

                for (int j = 0; j < dataList.size() - 3; ++j) {
                    if (0x10000000 + j == target) {
                        isFound = 1;

                        dataList[j + 0] = rtHex.substr(6, 2);

                        break;
                    }
                }

                if (isFound != 1) {
                    if (target < 0x10000000) {
                        textList.pop_back();
                        textList.pop_back();
                        textList.pop_back();

                        int offset = target - 0x400000;
                        int textLength = textList.size();

                        for (int j = 0; j < offset - textLength; ++j) {
                            textList.push_back("00");
                        }

                        textList.push_back(rtHex.substr(6, 2));

                        textList.push_back("00");
                        textList.push_back("00");
                        textList.push_back("00");
                    }
                    else {
                        dataList.pop_back();
                        dataList.pop_back();
                        dataList.pop_back();

                        int offset = target - 0x10000000;
                        int dataLength = dataList.size();

                        for (int j = 0; j < offset - dataLength; ++j) {
                            dataList.push_back("00");
                        }

                        dataList.push_back(rtHex.substr(6, 2));

                        dataList.push_back("00");
                        dataList.push_back("00");
                        dataList.push_back("00");
                    }
                }
            }
            else if (op == 9) { // addiu
                regList[rt] = regList[rs] + immediate;
            }
            else if (op == 12) { // andi
                regList[rt] = regList[rs] & immediate;
            }
            else if (op == 4) { // beq
                if (regList[rs] == regList[rt]) {
                    pc += immediate * 4;
                }
            }
            else if (op == 5) { // bne
                if (regList[rs] != regList[rt]) {
                    pc += immediate * 4;
                }
            }
            else if (op == 13) { // ori
                regList[rt] = regList[rs] | immediate;
            }
            else if (op == 11) { // sltiu
                regList[rt] = regList[rs] < immediate;
            }
        }

        if (dynamicFlag == 1) {
            cons.printPCReg(pc, regList);

            if (memoryFlag == 1) {
                cons.printMemory(addr1, addr2, textList, dataList);
            }
        }

        ++i;
    }

    cons.printPCReg(pc, regList);

    if (memoryFlag == 1) {
        cons.printMemory(addr1, addr2, textList, dataList);
    }

    return 0;
}
