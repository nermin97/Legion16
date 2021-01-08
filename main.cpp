#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <sstream>
#include <ctime>

using namespace std;

// Struktura instrukcije
struct instruction {
    void* opcode = nullptr;
    unsigned short int src2 = 0x0000;
    unsigned char dest = 0x0, src1= 0x0;
} code[65536];

unsigned short int TPC = 0, SPC = 0, memory[65536]={0x0000}, registers[16] = {0}, numberOfInstructions;
int openModes[16], fd;

bool checkTPCLimit() {
    if(TPC >= numberOfInstructions)
        return true;
    return false;
}

void increasePCs() {
    SPC += 4;
    TPC++;
}

void populateOpenModes(){
    openModes[0] = 0;//O_RDONLY
    openModes[1] = 1;//O_WRONLY
    openModes[2] = 2;//O_RDWR
    openModes[3] = 8;//O_APPEND
    openModes[4] = 256;//O_CREAT
    openModes[5] = 512;//O_TRUNC
    openModes[6] = 1024;//O_EXCL
    openModes[7] = 0;//Defaultni mode
    openModes[8] = 0;
    openModes[9] = 0;
    openModes[10] = 0;
    openModes[11] = 0;
    openModes[12] = 0;
    openModes[13] = 0;
    openModes[14] = 0;
    openModes[15] = 0;
}

bool increaseProgramCountersAndCheckIfEXIT() {
    increasePCs();
    if(checkTPCLimit())
        return true;
    return false;
}

std::string readFromMemory(int address, int numberOfBytes = 0) {
    std::string name;
    char firstCharacter;
    char secondCharacter;
    int charactersRead = 0;

    do {
        auto value = memory[address];
        firstCharacter = char((value >> 8) & 0x00FF);
        secondCharacter = char(value & 0x00FF);

        if(firstCharacter != (0x00)) {
            charactersRead++;
            if(numberOfBytes != 0 && charactersRead > numberOfBytes) break;
            name += firstCharacter;

            if(secondCharacter != (0x00)) {
                charactersRead++;
                if(numberOfBytes != 0 && charactersRead > numberOfBytes) break;
            }
            name += secondCharacter;
        }
        address++;
    } while(firstCharacter != (0x00));

    return name;
}

void writeToMemory(int whereToWrite, std::string stringToWrite, int numberOfBytes = 0) {

    if(numberOfBytes > stringToWrite.size())
        return;

    unsigned int size = stringToWrite.size();
    bool controlVariable = false;
    if(numberOfBytes != 0) {
        size = numberOfBytes;
        if(numberOfBytes % 2 != 0) {
            controlVariable = true;
            size++;
        }
    }

    for(unsigned int i = 0; i < size; ++i) {
        short int twoChars = (char(stringToWrite[i]) << 8);

        if(i == size - 1 && controlVariable) {
            memory[whereToWrite] = twoChars;
            break;
        }

        if((i + 1) < size) {
            twoChars =  (twoChars | char(stringToWrite[i+1]));
            i++;
        }

        memory[whereToWrite] = twoChars;
        if((i + 1) < size)
            whereToWrite++;
    }
    if(size % 2 != 0 || controlVariable)
        memory[whereToWrite] = memory[whereToWrite] & 0xFF00;
    else
        memory[++whereToWrite] = 0x0000;
}

void emulate() {
    // Output Constants:
    const string DESTINATION = "Destination: ";
    const string SOURCE_1 = ", Source_1: ";
    const string SOURCE_2 = ", Source_2: ";

    // Program counters
    unsigned short int r, n, k, r_temp, temporarySRC2;
    bool rBOOL, nBOOL;
    std::string stringForWriting;
    void* routinesArray[16] = {&&LOD, &&ADD, &&SUB, &&AND, &&ORA, &&XOR, &&SHR, &&MUL, &&STO, &&LDC, &&GTU, &&GTS, &&LTU, &&LTS, &&EQU, &&MAJ};
    // We need here code for specific opcode, dest, src1 and src2
    // Used to generate random instructions and a random number of those
    srand (time(nullptr));

    numberOfInstructions = rand() % 1000 + 100;
    cout << "Number of instructions: " << numberOfInstructions << endl;

    for(int i = 0; i < numberOfInstructions; ++i) {
        memory[i] = rand() % 65536;

        code[i].opcode = routinesArray[(memory[i] >> 12) & 0x000F];
        code[i].dest = (memory[i] >> 8) & 0x000F;
        code[i].src1 = (memory[i] >> 4) & 0x000F;
        code[i].src2 = memory[i] & 0x000F;
    }

    // Check if there is no data available
    if(numberOfInstructions == 0) goto EXIT;

    // Initial jump to first routine
    goto *code[0].opcode;

    // Addition to STO and LOD instruction unpacking

    // Here are our routines defined
    LOD:
        cout << "LOD" << endl;
        // special case if src2 == PC add 2
        registers[code[TPC].dest] = memory[code[TPC].src2];
        cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "]" << SOURCE_2 << "memory[" << to_string(code[TPC].src2) << "]" << endl << endl;
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    ADD:
        cout << "ADD" << endl;
        registers[code[TPC].dest] = registers[code[TPC].src1] + registers[code[TPC].src2];
        cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "]" << SOURCE_1 << "registers[" << to_string(code[TPC].src1) << "]" << SOURCE_2 << "registers[" << to_string(code[TPC].src2) << "]" << endl << endl;
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    SUB:
        cout << "SUB" << endl;
        registers[code[TPC].dest] = registers[code[TPC].src1] - registers[code[TPC].src2];
        cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "]" << SOURCE_1 << "registers[" << to_string(code[TPC].src1) << "]" << SOURCE_2 << "registers[" << to_string(code[TPC].src2) << "]" << endl << endl;
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    AND:
        cout << "AND" << endl;
        registers[code[TPC].dest] = registers[code[TPC].src1] & registers[code[TPC].src2];
        cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "]" << SOURCE_1 << "registers[" << to_string(code[TPC].src1) << "]" << SOURCE_2 << "registers[" << to_string(code[TPC].src2) << "]" << endl << endl;
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    ORA:
        cout << "ORA" << endl;
        registers[code[TPC].dest] = registers[code[TPC].src1] | registers[code[TPC].src2];
        cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "]" << SOURCE_1 << "registers[" << to_string(code[TPC].src1) << "]" << SOURCE_2 << "registers[" << to_string(code[TPC].src2) << "]" << endl << endl;
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    XOR:
        cout << "XOR:" << endl;
        registers[code[TPC].dest] = registers[code[TPC].src1] ^ registers[code[TPC].src2];
        cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "]" << SOURCE_1 << "registers[" << to_string(code[TPC].src1) << "]" << SOURCE_2 << "registers[" << to_string(code[TPC].src2) << "]" << endl << endl;
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    SHR:
        cout << "Executing SHR" << endl << endl;
        r = registers[code[TPC].src2];
        n = registers[code[TPC].src2] & 0x000F;
        r_temp = ((r >> 4) & 0x0003);
        if (r_temp == 0) {
            k = (registers[code[TPC].src1] | 0x7FFF);
            registers[code[TPC].dest] = (k == 0xFFFF) ?  (registers[code[TPC].src1] >> n) & ~(((0x1 << 16) >> n) << 1) : registers[code[TPC].src1] >> n;
        }
        else if (r_temp == 1)
            registers[code[TPC].dest] = registers[code[TPC].src1] >> n;
        else if (r_temp == 2)
            registers[code[TPC].dest] = (registers[code[TPC].src1] << n) | (registers[code[TPC].src1] >> (16 - n));
        else
            registers[code[TPC].dest] = registers[code[TPC].src1] << n;

        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    MUL:
        cout << "MUL" << endl;
        registers[code[TPC].dest] = registers[code[TPC].src1] * registers[code[TPC].src2];
        cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "]" << SOURCE_1 << "registers[" << to_string(code[TPC].src1) << "]" << SOURCE_2 << "registers[" << to_string(code[TPC].src2) << "]" << endl << endl;
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    STO:
        cout << "STO" << endl;
        memory[registers[code[TPC].src2]] = registers[code[TPC].src1];
        cout <<  "memory[" << SOURCE_2 << "registers[" << to_string(code[TPC].src2) << "]] =" << SOURCE_1 << "registers[" << to_string(code[TPC].src1) << "]" << endl;
        registers[code[TPC].dest] = registers[code[TPC].src1];
        cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "]" << SOURCE_1 << "registers[" << to_string(code[TPC].src1) << "]" << endl << endl;
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    LDC:
        cout << "LDC" << endl;
        registers[code[TPC].dest] = (code[TPC].src1 | code[TPC].src2);
        cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "]" << SOURCE_1 << "registers[" << to_string(code[TPC].src1) << "]" << SOURCE_2 << "registers[" << to_string(code[TPC].src2) << "]" << endl << endl;
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    GTU:
        cout << "GTU" << endl;
        registers[code[TPC].dest] = registers[code[TPC].src1] > registers[code[TPC].src2] ? 0x0001 : 0x0000;
        cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "]" << SOURCE_1 << "registers[" << to_string(code[TPC].src1) << "]" << SOURCE_2 << "registers[" << to_string(code[TPC].src2) << "]" << endl << endl;
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    GTS:
        cout << "GTS" << endl;
        rBOOL = (registers[code[TPC].src1] & 0x8000) != 0x0000;
        nBOOL = (registers[code[TPC].src2] & 0x8000) != 0x0000;
        if(rBOOL && !nBOOL) {
            registers[code[TPC].dest] = 0x0001;
            cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "] = 0x0001" << endl << endl;
        }
        else if(!rBOOL && nBOOL) {
            registers[code[TPC].dest] = 0x0000;
            cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "] = 0x0000" << endl << endl;
        }
        else {
            registers[code[TPC].dest] = registers[code[TPC].src1] > registers[code[TPC].src2] ? 0x0001 : 0x0000;
            cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "]" << SOURCE_1 << "registers[" << to_string(code[TPC].src1) << "]" << SOURCE_2 << "registers[" << to_string(code[TPC].src2) << "]" << endl << endl;
        }
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    LTU:
        cout << "LTU" << endl;
        registers[code[TPC].dest] = registers[code[TPC].src1] < registers[code[TPC].dest] ? 0x0001 : 0x0000;
        cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "]" << SOURCE_1 << "registers[" << to_string(code[TPC].src1) << "]" << SOURCE_2 << "registers[" << to_string(code[TPC].src2) << "]" << endl << endl;
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    LTS:
        cout << "LTS" << endl;
        rBOOL = (registers[code[TPC].src1] & 0x8000) != 0x0000;
        nBOOL = (registers[code[TPC].src2] & 0x8000) != 0x0000;
        if(rBOOL && !nBOOL) {
            registers[code[TPC].dest] = 0x0000;
            cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "] = 0x0000" << endl << endl;
        }
        else if(!rBOOL && nBOOL) {
            registers[code[TPC].dest] = 0x0001;
            cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "] = 0x0001" << endl << endl;
        }
        else {
            registers[code[TPC].dest] = registers[code[TPC].src1] < registers[code[TPC].src2] ? 0x0001 : 0x0000;
            cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "]" << SOURCE_1 << "registers[" << to_string(code[TPC].src1) << "]" << SOURCE_2 << "registers[" << to_string(code[TPC].src2) << "]" << endl << endl;
        }

        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    EQU:
        cout << "EQU" << endl;
        registers[code[TPC].dest] = registers[code[TPC].src1] == registers[code[TPC].src2] ? 0x0001 : 0x0000;
        cout << DESTINATION << "registers[" << to_string(code[TPC].dest) << "]" << SOURCE_1 << "registers[" << to_string(code[TPC].src1) << "]" << SOURCE_2 << "registers[" << to_string(code[TPC].src2) << "]" << endl << endl;
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    MAJ:
        cout << "MAJ" << endl;
        temporarySRC2 = TPC;
        registers[code[TPC].dest] = registers[code[TPC].src1];

        TPC = registers[code[TPC].src2];
        SPC += 4;

        int numberOfBytes;

        if(code[temporarySRC2].dest == 0x000C && code[temporarySRC2].src1 == 0x000F && code[temporarySRC2].src2 == 0x000B && registers[code[temporarySRC2].src2] == 0xFFF0) {
            // System calls
            std::string fn, fileName;

            // We can use here (registers[0] & 0x000F) or (registers[0] & 0x0003), it will have the same effect for our case
            switch(registers[0] & 0x000F) {
                case 0x0000:
                    fn = "./storage/" + readFromMemory(registers[1]);
                    if ((fd = open(fn.c_str(), openModes[registers[2] & 0x000F] | openModes[(registers[2] >> 4) & 0x000F] | openModes[(registers[2] >> 8) & 0x000F] | openModes[(registers[2] >> 12) & 0x000F])) < 0)
                        perror("open() error");
                    else {
                        registers[1] = fd;
                        std::cout << "File with the name: " << fn << " opened!" << std::endl;
                    }
                    break;
                case 0x0001:
                    if(registers[1] < 0)
                        std::cout << "No file opened!" << std::endl;
                    else {
                        char buff[1024];
                        if((numberOfBytes = read(registers[1], buff, registers[3])) > 0) {
                            cout << "Read block: " << std::endl << buff << std::endl;
                            stringForWriting = buff;
                            writeToMemory(registers[2], stringForWriting, registers[3]);
                        }
                        else
                            std::cout << "Mistake at reading a block!" << std::endl;
                    }
                    break;
                case 0x0002:
                    if(registers[1] < 0)
                        std::cout << "No file opened!" << std::endl;
                    else {
                        stringForWriting = readFromMemory(registers[2],registers[3]);
                        char stringForWritingChar[3];
                        for (int i = 0; i < stringForWriting.length(); i++)
                            stringForWritingChar[i] = stringForWriting[i];
                        if((numberOfBytes = write(registers[1], stringForWritingChar, registers[3])) == -1)
                            std::cout << "Writing to file failed!";
                        else
                            std::cout << "Number of written bytes: " << numberOfBytes << std::endl;
                    }
                    break;
                case 0x0003:
                    if(registers[1] != 0) {
                        close(registers[1]);
                        std::cout << "File closed!" << std::endl;
                    } else
                        std::cout << "Mistake at closing file!" << std::endl;
                    break;
            }
        }

        if(TPC >= numberOfInstructions) goto EXIT;

        if(registers[code[temporarySRC2].src1] == 0x000F) goto *code[TPC].opcode;

        TPC = temporarySRC2;
        TPC++;

        if(TPC >= numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    EXIT:
        cout << "End of emulation!" << std::endl;
}

int main() {
    populateOpenModes();
    emulate();
    return 0;
}
