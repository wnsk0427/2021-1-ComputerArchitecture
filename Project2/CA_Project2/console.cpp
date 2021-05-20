#include "console.h"

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