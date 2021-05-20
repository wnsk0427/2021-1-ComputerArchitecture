#include "console.h"

unsigned int Console::signExtention(unsigned int number) {
    string num = bitset<16>(number).to_string();

    if (num[0] == '0') {
        num = "0000000000000000" + num;
    }
    else {
        num = "1111111111111111" + num;
    }

    return stoul(num, nullptr, 2);
}

unsigned int Console::ifStage(unsigned int pc, vector<string> textList, unsigned int ifidRegList[]) {
    unsigned int index = pc - 0x400004;
    string instValue = textList[index] + textList[index + 1] + textList[index + 2] + textList[index + 3];

    ifidRegList[0] = pc; // NPC
    ifidRegList[1] = stoul(instValue, nullptr, 16); // Instr
    ifidRegList[2] = pc - 4; // pc

    string instrBin = bitset<32>(ifidRegList[1]).to_string();

    ifidRegList[3] = stoul(instrBin.substr(6, 5), nullptr, 2); // rs
    ifidRegList[4] = stoul(instrBin.substr(11, 5), nullptr, 2); // rt

    return pc - 4;
}

unsigned int Console::idStage(unsigned int ifidRegList[], unsigned int controlUnit[], unsigned int regList[], unsigned int idexRegList[]) {
    string npcBin = bitset<32>(ifidRegList[0]).to_string();
    string instrBin = bitset<32>(ifidRegList[1]).to_string();

    unsigned int opcode = stoul(instrBin.substr(0, 6), nullptr, 2);

    if (opcode == 0) { // R-type - addu and nor or sltu sll srl subu jr
        controlUnit[0] = 1; // RegDst
        controlUnit[1] = 2; // ALUOp
        controlUnit[2] = 0; // ALUSrc
        controlUnit[3] = 0; // Branch
        controlUnit[4] = 0; // MemRead
        controlUnit[5] = 0; // MemWrite
        controlUnit[6] = 1; // RegWrite
        controlUnit[7] = 0; // MemtoReg
        controlUnit[8] = 0; // Jump

        if (stoul(instrBin.substr(26), nullptr, 2) == 8) { // jr
            controlUnit[6] = 0;
            controlUnit[8] = 1;
        }
    }
    else if (opcode == 35 || opcode == 32) { // Load - lw lb
        controlUnit[0] = 0; // RegDst
        controlUnit[1] = 0; // ALUOp
        controlUnit[2] = 1; // ALUSrc
        controlUnit[3] = 0; // Branch
        controlUnit[4] = 1; // MemRead
        controlUnit[5] = 0; // MemWrite
        controlUnit[6] = 1; // RegWrite
        controlUnit[7] = 1; // MemtoReg
        controlUnit[8] = 0; // Jump
    }
    else if (opcode == 43 || opcode == 40) { // Store - sw sb
        controlUnit[0] = 0; // RegDst
        controlUnit[1] = 0; // ALUOp
        controlUnit[2] = 1; // ALUSrc
        controlUnit[3] = 0; // Branch
        controlUnit[4] = 0; // MemRead
        controlUnit[5] = 1; // MemWrite
        controlUnit[6] = 0; // RegWrite
        controlUnit[7] = 0; // MemtoReg
        controlUnit[8] = 0; // Jump
    }
    else if (opcode == 4 || opcode == 5) { // Branch - beq bne
        controlUnit[0] = 0; // RegDst
        controlUnit[1] = 1; // ALUOp
        controlUnit[2] = 0; // ALUSrc
        controlUnit[3] = 1; // Branch
        controlUnit[4] = 0; // MemRead
        controlUnit[5] = 0; // MemWrite
        controlUnit[6] = 0; // RegWrite
        controlUnit[7] = 0; // MemtoReg
        controlUnit[8] = 0; // Jump
    }
    else if (opcode == 2 || opcode == 3) { // Jump - j jal
        controlUnit[0] = 0; // RegDst
        controlUnit[1] = 1; // ALUOp
        controlUnit[2] = 0; // ALUSrc
        controlUnit[3] = 0; // Branch
        controlUnit[4] = 0; // MemRead
        controlUnit[5] = 0; // MemWrite
        controlUnit[6] = 0; // RegWrite
        controlUnit[7] = 0; // MemtoReg
        controlUnit[8] = 1; // Jump

        if (opcode == 3) {
            controlUnit[6] = 1;
        }
    }
    else { // I-type - lui addiu andi ori sltiu
        controlUnit[0] = 0; // RegDst
        controlUnit[1] = 2; // ALUOp
        controlUnit[2] = 1; // ALUSrc
        controlUnit[3] = 0; // Branch
        controlUnit[4] = 0; // MemRead
        controlUnit[5] = 0; // MemWrite
        controlUnit[6] = 1; // RegWrite
        controlUnit[7] = 0; // MemtoReg
        controlUnit[8] = 0; // Jump
    }

    idexRegList[8] = controlUnit[0]; // RegDst
    idexRegList[9] = controlUnit[1]; // ALUOp
    idexRegList[10] = controlUnit[2]; // ALUSrc
    idexRegList[11] = controlUnit[3]; // Branch
    idexRegList[12] = controlUnit[4]; // MemRead
    idexRegList[13] = controlUnit[5]; // MemWrite
    idexRegList[14] = controlUnit[6]; // RegWrite
    idexRegList[15] = controlUnit[7]; // MemtoReg
    idexRegList[16] = ifidRegList[2]; // pc

    idexRegList[0] = ifidRegList[0]; // NPC
    idexRegList[1] = opcode; // op
    idexRegList[2] = regList[stoul(instrBin.substr(6, 5), nullptr, 2)]; // rsd
    idexRegList[3] = regList[stoul(instrBin.substr(11, 5), nullptr, 2)]; // rtd
    idexRegList[4] = signExtention(stoul(instrBin.substr(16), nullptr, 2)); // IMM
    idexRegList[5] = stoul(instrBin.substr(11, 5), nullptr, 2); // rt
    idexRegList[6] = stoul(instrBin.substr(16, 5), nullptr, 2); // rd

    idexRegList[17] = stoul(instrBin.substr(6, 5), nullptr, 2); // rs
    idexRegList[18] = stoul(instrBin.substr(11, 5), nullptr, 2); // rt

    if (controlUnit[3] == 1) { // Branch == 1 --> Branch target
        idexRegList[7] = ifidRegList[0] + (signExtention(stoul(instrBin.substr(16), nullptr, 2)) << 2); // target
    }

    return ifidRegList[2];
}

unsigned int Console::exStage(unsigned int idexRegList[], unsigned int exmemRegList[], unsigned int regList[]) {
    exmemRegList[4] = idexRegList[11]; // Branch
    exmemRegList[5] = idexRegList[12]; // MemRead
    exmemRegList[6] = idexRegList[13]; // MemWrite
    exmemRegList[7] = idexRegList[14]; // RegWrite
    exmemRegList[8] = idexRegList[15]; // MemtoReg
    exmemRegList[9] = idexRegList[16]; // pc

    exmemRegList[0] = idexRegList[1]; // op

    string immBin = bitset<32>(idexRegList[4]).to_string();
    unsigned int funct = stoul(immBin.substr(26), nullptr, 2);

    if (idexRegList[9] == 2) { // ALUOp == 2 --> R-type I-type
        if (idexRegList[10] == 0) { // ALUSrc == 0 --> R-type
            if (funct == 33) { // addu
                exmemRegList[1] = idexRegList[2] + idexRegList[3]; // ALU_OUT
            }
            else if (funct == 36) { // and
                exmemRegList[1] = idexRegList[2] & idexRegList[3];
            }
            else if (funct == 39) { // nor
                exmemRegList[1] = ~(idexRegList[2] | idexRegList[3]);
            }
            else if (funct == 37) { // or
                exmemRegList[1] = idexRegList[2] | idexRegList[3];
            }
            else if (funct == 43) { // sltu
                exmemRegList[1] = idexRegList[2] < idexRegList[3];
            }
            else if (funct == 0) { // sll
                exmemRegList[1] = idexRegList[3] << stoul(immBin.substr(21, 5), nullptr, 2);
            }
            else if (funct == 2) { // srl
                exmemRegList[1] = idexRegList[3] >> stoul(immBin.substr(21, 5), nullptr, 2);
            }
            else if (funct == 35) { // subu
                exmemRegList[1] = idexRegList[2] - idexRegList[3];
            }
        }
        else { // ALUSrc == 1 --> I-type
            if (idexRegList[1] == 15) { // lui
                exmemRegList[1] = idexRegList[4] << 16;
            }
            else if (idexRegList[1] == 9) { // addiu
                exmemRegList[1] = idexRegList[2] + idexRegList[4];
            }
            else if (idexRegList[1] == 12) { // andi
                exmemRegList[1] = idexRegList[2] & stoul(immBin.substr(16), nullptr, 2);
            }
            else if (idexRegList[1] == 13) { // ori
                exmemRegList[1] = idexRegList[2] | stoul(immBin.substr(16), nullptr, 2);
            }
            else if (idexRegList[1] == 11) { // sltiu
                exmemRegList[1] = idexRegList[2] < idexRegList[4];
            }
        }
    }
    else if (idexRegList[9] == 1) { // ALUOp == 1 --> Branch Jump
        if (idexRegList[1] == 3) { // jal
            exmemRegList[1] = idexRegList[0];
        }
    }
    else { // ALUOp == 0 --> Load Store
        exmemRegList[1] = idexRegList[2] + idexRegList[4]; // ALU_OUT

        if (idexRegList[12] == 0) { // MemRead == 0 --> Store
            exmemRegList[2] = idexRegList[3];
        }
    }

    if (idexRegList[8] == 0) { // RegDst == 0 --> rd = rt
        exmemRegList[3] = idexRegList[5];

        if (idexRegList[1] == 3) { // jal
            exmemRegList[3] = 31;
        }
    }
    else { // RegDst == 1 --> rd = rd
        exmemRegList[3] = idexRegList[6];
    }

    if (exmemRegList[7] == 1 && exmemRegList[3] != 0 && exmemRegList[3] == idexRegList[17]) {
        idexRegList[2] = regList[exmemRegList[3]];
    }
    if (exmemRegList[7] == 1 && exmemRegList[3] != 0 && exmemRegList[3] == idexRegList[18]) {
        idexRegList[3] = regList[exmemRegList[3]];
    }

    return idexRegList[16];
}

unsigned int Console::memStage(unsigned int exmemRegList[], vector<string> textList, vector<string> dataList, unsigned int memwbRegList[], unsigned int idexRegList[], unsigned int regList[]) {
    memwbRegList[4] = exmemRegList[7]; // RegWrite
    memwbRegList[5] = exmemRegList[8]; // MemtoReg
    memwbRegList[6] = exmemRegList[9]; // pc

    memwbRegList[0] = exmemRegList[0]; // op

    if (exmemRegList[0] == 35) { // lw
        int isFound = 0;

        for (int j = 0; j < textList.size() - 3; ++j) {
            if (0x400000 + j == exmemRegList[1]) { // ALU_OUT
                isFound = 1;

                string textValue = textList[j] + textList[j + 1] + textList[j + 2] + textList[j + 3];
                unsigned int textHex = stoul(textValue, nullptr, 16);

                memwbRegList[1] = textHex; // MEM_OUT

                break;
            }
        }

        for (int j = 0; j < dataList.size() - 3; ++j) {
            if (0x10000000 + j == exmemRegList[1]) { // ALU_OUT
                isFound = 1;

                string dataValue = dataList[j] + dataList[j + 1] + dataList[j + 2] + dataList[j + 3];
                unsigned int dataHex = stoul(dataValue, nullptr, 16);

                memwbRegList[1] = dataHex; // MEM_OUT

                break;
            }
        }

        if (isFound != 1) {
            memwbRegList[1] = 0; // MEM_OUT
        }
    }
    else if (exmemRegList[0] == 32) { // lb
        int isFound = 0;

        for (int j = 0; j < textList.size() - 3; ++j) {
            if (0x400000 + j == exmemRegList[1]) { // ALU_OUT
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

                memwbRegList[1] = textHex; // MEM_OUT

                break;
            }
        }

        for (int j = 0; j < dataList.size() - 3; ++j) {
            if (0x10000000 + j == exmemRegList[1]) { // ALU_OUT
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

                memwbRegList[1] = dataHex; // MEM_OUT

                break;
            }
        }

        if (isFound != 1) {
            memwbRegList[1] = 0; // MEM_OUT
        }
    }
    else if (exmemRegList[0] == 43) { // sw
        int isFound = 0;

        string rtBin = bitset<32>(exmemRegList[2]).to_string(); // rtd

        string rtHex = "";
        string hexList[16] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f" };

        for (int j = 0; j < 8; ++j) {
            rtHex += hexList[stoul(rtBin.substr(j * 4, 4), nullptr, 2)];
        }

        for (int j = 0; j < textList.size() - 3; ++j) {
            if (0x400000 + j == exmemRegList[1]) { // ALU_OUT
                isFound = 1;

                textList[j + 0] = rtHex.substr(0, 2);
                textList[j + 1] = rtHex.substr(2, 2);
                textList[j + 2] = rtHex.substr(4, 2);
                textList[j + 3] = rtHex.substr(6, 2);

                break;
            }
        }

        for (int j = 0; j < dataList.size() - 3; ++j) {
            if (0x10000000 + j == exmemRegList[1]) { // ALU_OUT
                isFound = 1;

                dataList[j + 0] = rtHex.substr(0, 2);
                dataList[j + 1] = rtHex.substr(2, 2);
                dataList[j + 2] = rtHex.substr(4, 2);
                dataList[j + 3] = rtHex.substr(6, 2);

                break;
            }
        }

        if (isFound != 1) {
            if (exmemRegList[1] < 0x10000000) { // ALU_OUT
                textList.pop_back();
                textList.pop_back();
                textList.pop_back();

                int offset = exmemRegList[1] - 0x400000; // ALU_OUT
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

                int offset = exmemRegList[1] - 0x10000000; // ALU_OUT
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
    else if (exmemRegList[0] == 40) { // sb
        int isFound = 0;

        string rtBin = bitset<32>(exmemRegList[2]).to_string(); // rtd

        string rtHex = "";
        string hexList[16] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f" };

        for (int j = 0; j < 8; ++j) {
            rtHex += hexList[stoul(rtBin.substr(j * 4, 4), nullptr, 2)];
        }

        for (int j = 0; j < textList.size() - 3; ++j) {
            if (0x400000 + j == exmemRegList[1]) { // ALU_OUT
                isFound = 1;

                textList[j + 0] = rtHex.substr(6, 2);

                break;
            }
        }

        for (int j = 0; j < dataList.size() - 3; ++j) {
            if (0x10000000 + j == exmemRegList[1]) { // ALU_OUT
                isFound = 1;

                dataList[j + 0] = rtHex.substr(6, 2);

                break;
            }
        }

        if (isFound != 1) {
            if (exmemRegList[1] < 0x10000000) { // ALU_OUT
                textList.pop_back();
                textList.pop_back();
                textList.pop_back();

                int offset = exmemRegList[1] - 0x400000; // ALU_OUT
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

                int offset = exmemRegList[1] - 0x10000000; // ALU_OUT
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

    memwbRegList[2] = exmemRegList[1]; // ALU_OUT
    memwbRegList[3] = exmemRegList[3]; // rd

    if (memwbRegList[4] == 1 && memwbRegList[3] != 0 && exmemRegList[3] != idexRegList[17] && memwbRegList[3] != idexRegList[17]) {
        idexRegList[2] = regList[memwbRegList[3]];
    }
    if (memwbRegList[4] == 1 && memwbRegList[3] != 0 && exmemRegList[3] != idexRegList[18] && memwbRegList[3] != idexRegList[18]) {
        idexRegList[3] = regList[memwbRegList[3]];
    }

    return exmemRegList[9];
}

unsigned int Console::wbStage(unsigned int controlUnit[], unsigned int memwbRegList[], unsigned int regList[]) {
    if (memwbRegList[0] != 4 && memwbRegList[0] != 5 && controlUnit[8] != 1) {
        if (controlUnit[6] == 1) { // RegWrite == 1
            if (controlUnit[7] == 1) { // MemtoReg == 1 --> RegData = MEM_OUT
                regList[memwbRegList[3]] = memwbRegList[1];
            }
            else { // MemtoReg == 0 --> RegData = ALU_OUT
                regList[memwbRegList[3]] = memwbRegList[2];
            }
        }
    }
    

    return memwbRegList[6];
}

unsigned int* Console::controlHarzard(unsigned int pc, unsigned int ifidRegList[], unsigned int controlUnit[], unsigned int regList[], int takenFlag, unsigned int idexRegList[]) {
    static unsigned int pcFlush[3];

    unsigned int jumptar = 0;
    unsigned int flushNum = 0;
    unsigned int stall = 0;

    string npcBin = bitset<32>(ifidRegList[0]).to_string();
    string instrBin = bitset<32>(ifidRegList[1]).to_string();

    unsigned int opcode = stoul(instrBin.substr(0, 6), nullptr, 2);



    if (idexRegList[12] == 1 && (idexRegList[5] != idexRegList[3] || idexRegList[5] != idexRegList[4])) {
        // stall
        stall = 1;
    }

    if (controlUnit[8] == 1) { // Jump == 1 --> Jump target
        if (stoul(instrBin.substr(26), nullptr, 2) == 8) { // jr
            jumptar = regList[stoul(instrBin.substr(6, 5), nullptr, 2)];
            // flush
            flushNum = 1;
        }
        else { // j jal
            jumptar = (stoul(npcBin.substr(0, 4), nullptr, 2) << 28) + (stoul(instrBin.substr(6), nullptr, 2) << 2); // target
            // flush
            flushNum = 1;
        }
    }

    if (takenFlag == 1) { // Always Taken
        if (opcode == 4) { // beq
            if (idexRegList[2] == idexRegList[3]) {
                jumptar = idexRegList[7];
                // flush
                flushNum = 1;
            }
            else {
                jumptar = pc;
                // flush
                // flush
                // flush
                flushNum = 3;
            }
        }
        else if (opcode == 5) { // bne
            if (idexRegList[2] != idexRegList[3]) {
                jumptar = idexRegList[7];
                // flush
                flushNum = 1;
            }
            else {
                jumptar = pc;
                // flush
                // flush
                // flush
                flushNum = 3;
            }
        }
    }
    else { // Always Not Taken
        if (opcode == 4) { // beq
            if (idexRegList[2] == idexRegList[3]) {
                jumptar = idexRegList[7];
                // flush
                // flush
                // flush
                flushNum = 3;
            }
            else {
                jumptar = pc;
                flushNum = 0;
            }
        }
        else if (opcode == 5) { // bne
            if (idexRegList[2] != idexRegList[3]) {
                jumptar = idexRegList[7];
                // flush
                // flush
                // flush
                flushNum = 3;
            }
            else {
                jumptar = pc;
                flushNum = 0;
            }
        }
    }

    pcFlush[0] = jumptar;
    pcFlush[1] = flushNum;
    pcFlush[2] = stall;

    return pcFlush;
}

void Console::printStatePC(unsigned int pcList[]) {
    cout << "Current pipeline PC state:" << endl;

    cout << "{";

    for (int i = 0; i < 5; ++i) {
        if (pcList[i] == 0) {
            cout << "";
        }
        else {
            cout << hex << "0x" << pcList[i];
        }

        if (i < 4) {
            cout << "|";
        }
    }

    cout << "}\n" << endl;
}

void Console::printPCReg(unsigned int pc, unsigned int regList[]) {
    cout << "Current register values:" << endl;
    cout << "-----------------------------------------" << endl;

    cout << "PC: 0x" << hex << pc << endl;

    cout << "Registers:" << endl;
    for (int i = 0; i < 32; ++i)
        cout << "R" << dec << i << ": 0x" << hex << regList[i] << endl;

    cout << endl;
}

void Console::printMemory(string addr1, string addr2, vector<string> textList, vector<string> dataList) {
    cout << "Memory content [" << addr1 << ".." << addr2 << "]:" << endl;
    cout << "-----------------------------------------" << endl;

    unsigned int intAddr1 = stoul(addr1, nullptr, 16);
    unsigned int intAddr2 = stoul(addr2, nullptr, 16);

    for (unsigned int i = intAddr1; i <= intAddr2; i += 4) {
        int isFound = 0;

        for (int j = 0; j < textList.size() - 3; j += 4) {
            if (i == 0x400000 + j) {
                isFound = 1;

                string textValue = textList[j] + textList[j + 1] + textList[j + 2] + textList[j + 3];
                unsigned int textHex = stoul(textValue, nullptr, 16);

                cout << "0x" << hex << i << ": 0x" << textHex << endl;

                break;
            }
        }

        for (int j = 0; j < dataList.size() - 3; j += 4) {
            if (i == 0x10000000 + j) {
                isFound = 1;

                string dataValue = dataList[j] + dataList[j + 1] + dataList[j + 2] + dataList[j + 3];
                unsigned int dataHex = stoul(dataValue, nullptr, 16);

                cout << "0x" << hex << i << ": 0x" << dataHex << endl;

                break;
            }
        }

        if (isFound != 1) {
            cout << "0x" << hex << i << ": 0x0" << endl;
        }
    }

    cout << endl;
}