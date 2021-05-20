#include "console.h"

int main(int argc, char* argv[]) {
    Console cons;

    ifstream inputFile;



    /* 시작 전 인자 받기 */     

    int takenFlag = 0;
    int memoryFlag = 0;
    string addr1, addr2;
    int debugingFlag = 0;
    int pipelineFlag = 0;
    int num_instruction = -1;
    string inputFileName;

    string arg = argv[1];
    
    if (arg == "-atp") {
        takenFlag = 1;
    }
    else if (arg == "-antp") {
        takenFlag = 0;
    }
    else {
        cout << "The option is wrong. Please re-enter it.\n";
        return 0;
    }

    for (int i = 2; i < argc; i++) {
        arg = argv[i];

        if (arg == "-m") {
            arg = argv[++i];

            memoryFlag = 1;
            addr1 = arg.substr(0, arg.find(":"));
            addr2 = arg.substr(arg.find(":") + 1);
        }
        else if (arg == "-d") {
            debugingFlag = 1;
        }
        else if (arg == "-p") {
            pipelineFlag = 1;
        }
        else if (arg == "-n") {
            num_instruction = stoul(argv[++i]);
        }
        else {
            inputFileName = arg;
        }
    }

    inputFile.open(inputFileName);

    if (!inputFile.is_open()) {
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
    int flushNum = 0;
    unsigned int jumptar = 0;
    unsigned int pc = 0x400000;
    unsigned int pcList[5];
    fill_n(pcList, 5, 0);

    unsigned int regList[32];
    fill_n(regList, 32, 0);

    unsigned int controlUnit[9]; // RegDst, ALUOp, ALUSrc, Branch, MemRead, MemWrite, RegWrite, MemtoReg, Jump
    fill_n(controlUnit, 9, 0);

    unsigned int ifidRegList[5]; // NPC, Instr, pc, rs, rt
    unsigned int idexRegList[19]; // NPC, op, rsd, rtd, IMM, rt, rd, target, RegDst, ALUOp, ALUSrc, Branch, MemRead, MemWrite, RegWrite, MemtoReg, pc, rs, rt
    unsigned int exmemRegList[10]; // op, ALU_OUT, rtd, rd, Branch, MemRead, MemWrite, RegWrite, MemtoReg, pc
    unsigned int memwbRegList[7]; // op, MEM_OUT, ALU_OUT, rd, RegWrite, MemtoReg, pc
    fill_n(ifidRegList, 5, 0);
    fill_n(idexRegList, 19, 0);
    fill_n(exmemRegList, 10, 0);
    fill_n(memwbRegList, 7, 0);
    
    while(pc < 0x400000 + textList.size() - 3) {
        if (i == num_instruction) {
            break;
        }

        cout << dec << "===== Cycle " << i + 1 << " =====" << endl;

        int stall = 0;

        if (i >= 4) {
            pcList[4] = cons.wbStage(controlUnit, memwbRegList, regList);
        }
        if (i >= 3) {
            pcList[3] = cons.memStage(exmemRegList, textList, dataList, memwbRegList, idexRegList, regList);

            /*if (flushNum != 0) {
                fill_n(exmemRegList, 10, 0);
            }*/
        }
        if (i >= 2) {
            pcList[2] = cons.exStage(idexRegList, exmemRegList, regList);

            /*if (flushNum != 0) {
                fill_n(idexRegList, 19, 0);
            }*/
        }
        if (i >= 1) {
            pcList[1] = cons.idStage(ifidRegList, controlUnit, regList, idexRegList);

            // Control Hazard
            if (flushNum == 0) {

                stall = cons.controlHarzard(pc, ifidRegList, controlUnit, regList, takenFlag, idexRegList)[2];
                flushNum = cons.controlHarzard(pc, ifidRegList, controlUnit, regList, takenFlag, idexRegList)[1];
                jumptar = cons.controlHarzard(pc, ifidRegList, controlUnit, regList, takenFlag, idexRegList)[0];
            }
            
            /*if (flushNum != 0) {
                pc = jumptar;
                fill_n(ifidRegList, 5, 0);
            }*/

        }
        if (i >= 0) {
            pc += 4;
            pcList[0] = cons.ifStage(pc, textList, ifidRegList);
        }



        if (stall != 0) {
            pc -= 4;
            fill_n(ifidRegList, 5, 0);
        }

        
        if (flushNum != 0) {
            if (flushNum >= 3) {
                fill_n(exmemRegList, 10, 0);
            }
            if (flushNum >= 2) {
                fill_n(idexRegList, 19, 0);
            }
            if (flushNum >= 1) {
                fill_n(ifidRegList, 5, 0);
            }
            flushNum--;
        }

        if (jumptar != 0) {
            pc = jumptar;
        }


        if (pipelineFlag == 1) {
            cons.printStatePC(pcList);
        }
        
        if (debugingFlag == 1) {
            cons.printPCReg(pc, regList);
            
            if (memoryFlag == 1) {
                cons.printMemory(addr1, addr2, textList, dataList);
            }
        }

        ++i;
    }

    cout << dec << "===== Completion cycle: " << i + 1 << " =====\n" << endl;
    
    cons.printStatePC(pcList);
    cons.printPCReg(pc, regList);

    if (memoryFlag == 1) {
        cons.printMemory(addr1, addr2, textList, dataList);
    }

    return 0;
}

// ex Branch Jump
/*
        else if (idexRegList[9] == 1) { // ALUOp == 1 --> Branch Jump
            if (idexRegList[11] == 1) { // Branch == 1 --> Branch
                exmemRegList[1] = idexRegList[7]; // BR_TARGET

                if (idexRegList[1] == 4) { // beq
                    if (idexRegList[2] == idexRegList[3]) {
                        exmemRegList[2] = 1; // ALUZero
                    }
                    else {
                        exmemRegList[2] = 0;
                    }
                }
                else if (idexRegList[1] == 5) { // bne
                    if (idexRegList[2] != idexRegList[3]) {
                        exmemRegList[2] = 1;
                    }
                    else {
                        exmemRegList[2] = 0;
                    }
                }
            }
            else if (idexRegList[16] == 1) { // Jump == 1 --> Jump
                pc =
            }
        }
*/