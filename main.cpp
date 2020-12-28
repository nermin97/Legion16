#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <sys/stat.h>
#include<string>
#include <bitset>
#include <dirent.h>
#include <vector>
#define _POSIX_SOURCE
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#undef _POSIX_SOURCE
#include <stdio.h>
#include <sstream>
#include <cmath>
#include <time.h>
#include <map>

using namespace std;

struct instruction {
    void* opcode;
    unsigned short int src2 = 0x0000;
    unsigned char dest, src1;
} code[65536];

unsigned short int TPC = 0, SPC = 0, memory[65536]={0x0000}, registers[16], numberOfInstructions; // regs[15] je PC
int fileModes[16], openModes[16], lseekpos[16], fd;

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

void populateFileModes(){
    fileModes[0] = 64;//S_IXUSR
    fileModes[1] = 128;//S_IWUSR
    fileModes[2] = 256;//S_IRUSR
    fileModes[3] = 448;//S_IRWXU
    fileModes[4] = 256;//Defaultna vrijednost u slicaju da je rijec registra R2 veca od 4;
    fileModes[5] = 256;
    fileModes[6] = 256;
    fileModes[7] = 256;
    fileModes[8] = 256;
    fileModes[9] = 256;
    fileModes[10] = 256;
    fileModes[11] = 256;
    fileModes[12] = 256;
    fileModes[13] = 256;
    fileModes[14] = 256;
    fileModes[15] = 256;


    //Napomena: Neki mode/privilegije koji su podržani u unix sistemima, nisu podržani u windowsu.
    //Također, neki mode/privilegije su podržane u nekim kompajlerima
    //https://www.ibm.com/support/knowledgecenter/SSLTBW_2.2.0/com.ibm.zos.v2r2.bpxbd00/rtcre.htm


}

void settingRegistersToZero() {
    // Setting registers to zero
    // registers[15] is PC(Program counter) register
    for(int i = 0; i < 16; i++)
        registers[i] = 0;
}

bool increaseProgramCountersAndCheckIfEXIT() {
    SPC += 4;
    TPC++;
    if(TPC >= numberOfInstructions)
        return true;
    return false;
}

std::string readFromMemory(int address, int numberOfBytes = 0) {
    std::string name = "";
    //Provjeriti uslove
    char firstCharacter = (0x00);
    char secondCharacter = (0x00);
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
    // Program counters
    unsigned short int r, n, k, temporarySRC2;
    bool rBOOL, nBOOL;
    std::string stringForWriting = "";
    void* routinesArray[16] = {&&LOD, &&ADD, &&SUB, &&AND, &&ORA, &&XOR, &&SHR, &&MUL, &&STO, &&LDC, &&GTU, &&GTS, &&LTU, &&LTS, &&EQU, &&MAJ};
    // We need here code for specific opcode, dest, src1 and src2
    // Used to generate random instructions and a random number of those
    srand (time(NULL));

    numberOfInstructions = rand() % 1000 + 100;
    for(int i = 0; i < numberOfInstructions; ++i) {
        memory[i] = rand() % 65536;

        code[i].opcode = routinesArray[(memory[i] >> 12) & 0x000F];
        code[i].dest = (memory[i] >> 8) & 0x000F;
        code[i].src1 = (memory[i] >> 4) & 0x000F;
        code[i].src2 = memory[i] & 0x000F;
    }

    // Check if there is no data available
    if(numberOfInstructions == 0 || sizeof(memory) == 0 || sizeof(code) == 0) goto EXIT;

    // Initial jump to first routine
    goto *code[0].opcode;

    // Here are our routines defined
    LOD:
        // Ovdje treba ja mislim memory[code[TPC].src2] (ja mislim da je greska u proslom kodu)
//        registers[code[TPC].dest] = registers[code[TPC].src2];
        registers[code[TPC].dest] = memory[code[TPC].src2];
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    ADD:
        registers[code[TPC].dest] = registers[code[TPC].src1] + registers[code[TPC].src2];
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    SUB:
        registers[code[TPC].dest] = registers[code[TPC].src1] - registers[code[TPC].src2];
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    AND:
        registers[code[TPC].dest] = registers[code[TPC].src1] & registers[code[TPC].src2];
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    ORA:
        registers[code[TPC].dest] = registers[code[TPC].src1] | registers[code[TPC].src2];
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    XOR:
        registers[code[TPC].dest] = registers[code[TPC].src1] ^ registers[code[TPC].src2];
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    SHR:
        r = registers[code[TPC].src2];
        n = registers[code[TPC].src2] & 0x000F;
        if (((r >> 4) & 0x0003) == 0) {
            k = (registers[code[TPC].src1] | 0x7FFF);
            // odredjivanje je li 1 najveci bit, ako je 1 mora se dodati 1 na kraj, a ako je 0, obicni shift ce svakako dodati nulu
            registers[code[TPC].dest] = k == 0xFFFF ?  (registers[code[TPC].src1] >> n) & ~(((0x1 << 16) >> n) << 1) : registers[code[TPC].src1] >> n;
        }
        else if (((r >> 4) & 0x0003) == 1)
            registers[code[TPC].dest] = registers[code[TPC].src1] >> n;
        else if (((r >> 4) & 0x0003) == 2)
            registers[code[TPC].dest] = (registers[code[TPC].src1] << n) | (registers[code[TPC].src1] >> (16 - n));
        else
            registers[code[TPC].dest] = registers[code[TPC].src1] << n;

        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    MUL:
        registers[code[TPC].dest] = registers[code[TPC].src1] * registers[code[TPC].src2];
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    STO:
        memory[registers[code[TPC].src2]] = registers[code[TPC].src1];
        registers[code[TPC].dest] = registers[code[TPC].src1];
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    LDC:
        registers[code[TPC].dest] = (code[TPC].src1 | code[TPC].src2);
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    GTU:
        registers[code[TPC].dest] = registers[code[TPC].src1] > registers[code[TPC].src2] ? 0x0001 : 0x0000;
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    GTS:
        rBOOL = (registers[code[TPC].src1] & 0x8000) == 0x0000 ? false : true;
        nBOOL = (registers[code[TPC].src2] & 0x8000) == 0x0000 ? false : true;
        if(rBOOL && !nBOOL)
            registers[code[TPC].dest] = 0x0001;
        else if(!rBOOL && nBOOL)
            registers[code[TPC].dest] = 0x0000;
        else
            registers[code[TPC].dest] = registers[code[TPC].src1] > registers[code[TPC].src2] ? 0x0001 : 0x0000;

        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    LTU:
        registers[code[TPC].dest] = registers[code[TPC].src1] < registers[code[TPC].dest] ? 0x0001 : 0x0000;
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    LTS:
        rBOOL = (registers[code[TPC].src1] & 0x8000) == 0x0000 ? false : true;
        nBOOL = (registers[code[TPC].src2] & 0x8000) == 0x0000 ? false : true;
        if(rBOOL && !nBOOL)
            registers[code[TPC].dest] = 0x0000;
        else if(!rBOOL && nBOOL)
            registers[code[TPC].dest] = 0x0001;
        else
            registers[code[TPC].dest] = registers[code[TPC].src1] < registers[code[TPC].src2] ? 0x0001 : 0x0000;

        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    EQU:
        registers[code[TPC].dest] = registers[code[TPC].src1] == registers[code[TPC].src2] ? 0x0001 : 0x0000;
        if(increaseProgramCountersAndCheckIfEXIT()) goto EXIT;
        goto *code[TPC].opcode;
    MAJ:
//        registers[code[TPC].dest] = registers[code[TPC].src1];
        // What if DEST and SRC2 are the same
        temporarySRC2 = registers[code[TPC].src2];
        // We need to remember our PC location in this register
        registers[15] = TPC;
        registers[code[TPC].dest] = registers[15];

        // We set the PC's here
        registers[15] = code[TPC].src2;
        SPC += 4;

        int numberOfBytes;
        // Maybe we need this line here
//        registers[15] == 0x000F &&
        if(code[TPC].dest == 0x000C && code[TPC].src1 == 0x000F && code[TPC].src2 == 0x000B && temporarySRC2 == 0xFFF0) {
            // System calls
            std::string fn, fileName;
            int exists;

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
                        // CHECK WHY THERE ARE ONLY 3 CHARACTERS HERE
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

        // Check this part out a bit more if this works nicely
        registers[15] = registers[code[TPC].dest];
        TPC = registers[code[TPC].dest];

        if(TPC >= numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    EXIT:
        // We deallocate, free up manually all the variables and memory
        memory[65536] = {0x0000};
        n = 0;
        k = 0;
        r = 0;
        TPC = 0;
        SPC = 0;
        cout << "End of emulation!" << std::endl;
}

int main() {
    settingRegistersToZero();
    populateOpenModes();
    populateFileModes();
    emulate();
    return 0;
}
