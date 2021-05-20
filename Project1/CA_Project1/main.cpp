#include "format.h"

int main(int argc, char* argv[]) {
    Format form;

    ifstream sampleFile;
    ofstream outputFile;

    string sampleFileName = argv[1];
    sampleFile.open(sampleFileName);

    while (!sampleFile.is_open()) {
        cout << "The file name is incorrect. Please re-enter it.\n";
        return 0;
    }

    string outputFileName;
    outputFileName = sampleFileName.substr(0, sampleFileName.find(".")) + ".o";
    outputFile.open(outputFileName);

    /*  시작 전 읽기  */

    string fileLineWord;

    sampleFile >> fileLineWord;

    vector<pair<string, int>> fileDataLabelList;
    int dataCount = 0;

    if (fileLineWord == ".data") {
        sampleFile >> fileLineWord;

        while (fileLineWord != ".text") {
            if (fileLineWord == ".word") {
                dataCount++;
                sampleFile >> fileLineWord;
            }
            else {
                fileDataLabelList.push_back(make_pair(fileLineWord.substr(0, fileLineWord.find(":")), dataCount));
            }

            sampleFile >> fileLineWord;
        }
    }

    sampleFile >> fileLineWord;

    vector<pair<string, int>> fileTextLabelList;
    int textCount = 0;

    while (!sampleFile.eof()) {
        if (fileLineWord == "j" || fileLineWord == "jal" || fileLineWord == "jr") {
            fileTextLabelList.push_back(make_pair(fileLineWord, textCount));
            textCount++;
            sampleFile >> fileLineWord;
        }
        else if (fileLineWord == "la" || fileLineWord == "lui" || fileLineWord == "lw" || fileLineWord == "lb" || fileLineWord == "sw" || fileLineWord == "sb") {
            fileTextLabelList.push_back(make_pair(fileLineWord, textCount));
            textCount++;
            sampleFile >> fileLineWord;
            sampleFile >> fileLineWord;
        }
        else if (fileLineWord == "addu" || fileLineWord == "and" || fileLineWord == "nor" || fileLineWord == "or" || fileLineWord == "sltu" || fileLineWord == "sll" || fileLineWord == "srl" || fileLineWord == "subu") {
            fileTextLabelList.push_back(make_pair(fileLineWord, textCount));
            textCount++;
            sampleFile >> fileLineWord;
            sampleFile >> fileLineWord;
            sampleFile >> fileLineWord;
        }
        else if (fileLineWord == "addiu" || fileLineWord == "andi" || fileLineWord == "beq" || fileLineWord == "bne" || fileLineWord == "ori" || fileLineWord == "sltiu") {
            fileTextLabelList.push_back(make_pair(fileLineWord, textCount));
            textCount++;
            sampleFile >> fileLineWord;
            sampleFile >> fileLineWord;
            sampleFile >> fileLineWord;
        }
        else {
            fileTextLabelList.push_back(make_pair(fileLineWord.substr(0, fileLineWord.find(":")), textCount));
        }

        sampleFile >> fileLineWord;
    }

    sampleFile.close();

    /*  진정한 시작  */

    sampleFile.open(sampleFileName);

    sampleFile >> fileLineWord;

    vector<unsigned int> fileDataList;
    unsigned int fileData;

    if (fileLineWord == ".data") {
        sampleFile >> fileLineWord;

        while (fileLineWord != ".text") {
            if (fileLineWord == ".word") {
                sampleFile >> fileLineWord;

                if (fileLineWord.find("x") == 1) fileData = stoul(fileLineWord, nullptr, 16); // 32 bit
                else fileData = stoul(fileLineWord);

                fileDataList.push_back(fileData);
            }

            sampleFile >> fileLineWord;
        }
    }

    sampleFile >> fileLineWord;

    vector<string> fileTextList;
    string fileText; // binary
    textCount = 0;

    while (!sampleFile.eof()) {
        vector<string> fileLine;

        if (fileLineWord == "j" || fileLineWord == "jal" || fileLineWord == "jr") {
            // J JAL JR
            // 2칸짜리 
            fileLine.push_back(fileLineWord);

            for (int i = 0; i < 1; i++) {
                sampleFile >> fileLineWord;
                fileLine.push_back(fileLineWord);
            }

            if (fileLine[0] == "j") {
                unsigned int targetAdress = 0;

                for (int i = 0; i < fileTextLabelList.size(); i++)
                    if (fileLine[1] == fileTextLabelList[i].first)
                        targetAdress = 0x400000 + 4 * fileTextLabelList[i].second;

                char hexAdress[16];
                sprintf(hexAdress, "%x", targetAdress / 4);
                string strHexAdress = hexAdress;

                fileText = form.jform2bin(fileLine[0], "2", "0x" + strHexAdress);
            }
            else if (fileLine[0] == "jal") {
                unsigned int targetAdress = 0;

                for (int i = 0; i < fileTextLabelList.size(); i++)
                    if (fileLine[1] == fileTextLabelList[i].first)
                        targetAdress = 0x400000 + 4 * fileTextLabelList[i].second;

                char hexAdress[16];
                sprintf(hexAdress, "%x", targetAdress / 4);
                string strHexAdress = hexAdress;

                fileText = form.jform2bin(fileLine[0], "3", "0x" + strHexAdress);
            }
            else if (fileLine[0] == "jr") fileText = form.rform2bin(fileLine[0], "0", fileLine[1].substr(1), "0", "0", "0", "8");

            fileTextList.push_back(fileText);

            textCount++;
        }
        else if (fileLineWord == "la" || fileLineWord == "lui" || fileLineWord == "lw" || fileLineWord == "lb" || fileLineWord == "sw" || fileLineWord == "sb") {
            // LA LUI LW LB SW SB
            // 3칸짜리
            fileLine.push_back(fileLineWord);

            for (int i = 0; i < 2; i++) {
                sampleFile >> fileLineWord;
                fileLine.push_back(fileLineWord);
            }

            if (fileLine[0] == "la") {
                unsigned int targetAdress = 0;

                for (int i = 0; i < fileDataLabelList.size(); i++)
                    if (fileLine[2] == fileDataLabelList[i].first)
                        targetAdress = 0x10000000 + 4 * fileDataLabelList[i].second;

                char hexAdress[16];
                sprintf(hexAdress, "%x", targetAdress);
                string strHexAdress = hexAdress;

                string upperAdress = "0x" + strHexAdress.substr(0, strHexAdress.size() - 4);
                string lowerAdress = "0x" + strHexAdress.substr(strHexAdress.size() - 4);

                fileText = form.iform2bin(fileLine[0], "15", "0", fileLine[1].substr(1, fileLine[1].length() - 1), upperAdress); // lui

                if (lowerAdress != "0x0000") {
                    fileTextList.push_back(fileText);
                    fileText = form.iform2bin(fileLine[0], "13", fileLine[1].substr(1, fileLine[1].length() - 1), fileLine[1].substr(1, fileLine[1].length() - 1), lowerAdress); // ori
                }
            }
            else if (fileLine[0] == "lui") fileText = form.iform2bin(fileLine[0], "15", "0", fileLine[1].substr(1, fileLine[1].length() - 1), fileLine[2]);
            else {
                string sa, sb, sc;

                sa = fileLine[1].substr(1, fileLine[1].length() - 2);
                sb = fileLine[2].substr(0, fileLine[2].find("("));
                fileLine[2].erase(0, fileLine[2].find("(") + 1);
                sc = fileLine[2].substr(1, fileLine[2].length() - 2);

                if (fileLine[0] == "lw") fileText = form.iform2bin(fileLine[0], "35", sc, sa, sb);
                else if (fileLine[0] == "lb") fileText = form.iform2bin(fileLine[0], "32", sc, sa, sb);
                else if (fileLine[0] == "sw") fileText = form.iform2bin(fileLine[0], "43", sc, sa, sb);
                else if (fileLine[0] == "sb") fileText = form.iform2bin(fileLine[0], "40", sc, sa, sb);
            }

            fileTextList.push_back(fileText);

            textCount++;
        }
        else if (fileLineWord == "addu" || fileLineWord == "and" || fileLineWord == "nor" || fileLineWord == "or" || fileLineWord == "sltu" || fileLineWord == "sll" || fileLineWord == "srl" || fileLineWord == "subu") {
            // ADDU AND NOR OR SLTU SLL SRL SUBU
            // 4칸짜리 R format
            fileLine.push_back(fileLineWord);

            for (int i = 0; i < 3; i++) {
                sampleFile >> fileLineWord;
                fileLine.push_back(fileLineWord);
            }

            string sa, sb, sc;

            sa = fileLine[1].substr(1, fileLine[1].length() - 2);
            sb = fileLine[2].substr(1, fileLine[2].length() - 2);
            sc = fileLine[3].substr(1);

            if (fileLine[0] == "addu") fileText = form.rform2bin(fileLine[0], "0", sb, sc, sa, "0", "33");
            else if (fileLine[0] == "and") fileText = form.rform2bin(fileLine[0], "0", sb, sc, sa, "0", "36");
            else if (fileLine[0] == "nor") fileText = form.rform2bin(fileLine[0], "0", sb, sc, sa, "0", "39");
            else if (fileLine[0] == "or") fileText = form.rform2bin(fileLine[0], "0", sb, sc, sa, "0", "37");
            else if (fileLine[0] == "sltu") fileText = form.rform2bin(fileLine[0], "0", sb, sc, sa, "0", "43");
            else if (fileLine[0] == "sll") fileText = form.rform2bin(fileLine[0], "0", "0", sb, sa, fileLine[3], "0");
            else if (fileLine[0] == "srl") fileText = form.rform2bin(fileLine[0], "0", "0", sb, sa, fileLine[3], "2");
            else if (fileLine[0] == "subu") fileText = form.rform2bin(fileLine[0], "0", sb, sc, sa, "0", "35");

            fileTextList.push_back(fileText);

            textCount++;
        }
        else if (fileLineWord == "addiu" || fileLineWord == "andi" || fileLineWord == "beq" || fileLineWord == "bne" || fileLineWord == "ori" || fileLineWord == "sltiu") {
            // ADDIU ANDI BEQ BNE ORI SLTIU
            // 4칸짜리 I format
            fileLine.push_back(fileLineWord);

            for (int i = 0; i < 3; i++) {
                sampleFile >> fileLineWord;
                fileLine.push_back(fileLineWord);
            }

            string sa, sb;

            sa = fileLine[1].substr(1, fileLine[1].length() - 2);
            sb = fileLine[2].substr(1, fileLine[2].length() - 2);

            if (fileLine[0] == "addiu") fileText = form.iform2bin(fileLine[0], "9", sb, sa, fileLine[3]);
            else if (fileLine[0] == "andi") fileText = form.iform2bin(fileLine[0], "12", sb, sa, fileLine[3]);
            else if (fileLine[0] == "beq") {
                unsigned int currentAdress = 0x400000 + 4 * textCount;
                unsigned int targetAdress = 0;

                for (int i = 0; i < fileTextLabelList.size(); i++)
                    if (fileLine[3] == fileTextLabelList[i].first)
                        targetAdress = 0x400000 + 4 * fileTextLabelList[i].second;

                char hexAdress[16];
                sprintf(hexAdress, "%x", (targetAdress - currentAdress - 4) / 4);
                string strHexAdress = hexAdress;

                fileText = form.iform2bin(fileLine[0], "4", sa, sb, "0x" + strHexAdress);
            }
            else if (fileLine[0] == "bne") {
                unsigned int currentAdress = 0x400000 + 4 * textCount;
                unsigned int targetAdress = 0;

                for (int i = 0; i < fileTextLabelList.size(); i++)
                    if (fileLine[3] == fileTextLabelList[i].first)
                        targetAdress = 0x400000 + 4 * fileTextLabelList[i].second;

                char hexAdress[16];
                sprintf(hexAdress, "%x", (targetAdress - currentAdress - 4) / 4);
                string strHexAdress = hexAdress;

                fileText = form.iform2bin(fileLine[0], "5", sa, sb, "0x" + strHexAdress);
            }
            else if (fileLine[0] == "ori") fileText = form.iform2bin(fileLine[0], "13", sb, sa, fileLine[3]);
            else if (fileLine[0] == "sltiu") fileText = form.iform2bin(fileLine[0], "11", sb, sa, fileLine[3]);

            fileTextList.push_back(fileText);

            textCount++;
        }

        sampleFile >> fileLineWord;
    }

    outputFile << "0x" << hex << textCount * 4 << endl;
    outputFile << "0x" << hex << dataCount * 4 << endl;

    for (int i = 0; i < fileTextList.size(); i++) {
        unsigned int intText = stoul(fileTextList[i], nullptr, 2); // 32 bit
        outputFile << "0x" << hex << intText << endl;
    }
    for (int i = 0; i < fileDataList.size(); i++) {
        outputFile << "0x" << hex << fileDataList[i] << endl;
    }

    sampleFile.close();
    outputFile.close();

    return 0;
}

/*

      outputFile << hex << fileDataList.size() * 4 << endl;
불리안으로 32자리 변수 만들기
불리안변수에 값 집어넣기
각 포맷마다 필드 지정
각 필드에 맞춰서 변수 대입

immediate, shamt에 16진수가 들어오는가?

파일명을 입력받기
파일 입출력으로 샘플 파일 불러오기
각 줄을 스플릿하기
각 줄의 단어들 나누기
이프 엘스로 펑션 값 비교
비교 후 함수 처리
*/