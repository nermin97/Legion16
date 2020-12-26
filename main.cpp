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

using namespace std;

struct instruction {
    void* opcode;
    unsigned short int src2 = 0x0000;
    unsigned char dest, src1;
} code[65536];

unsigned short int memory[65536]={0x0000}, registers[16], numberOfInstructions; // regs[15] je PC
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

void populateLseeekPos(){
    lseekpos[0] = 0; //SEEK_SET
    lseekpos[1] = 1;//SEEK_CUR
    lseekpos[2] = 2;//SEEK_END
    lseekpos[3] = 0;
    lseekpos[4] = 0;
    lseekpos[5] = 0;
    lseekpos[6] = 0;
    lseekpos[7] = 0;
    lseekpos[8] = 0;
    lseekpos[9] = 0;
    lseekpos[10] = 0;
    lseekpos[11] = 0;
    lseekpos[12] = 0;
    lseekpos[13] = 0;
    lseekpos[14] = 0;
    lseekpos[15] = 0;
}

void emulate() {
    // Program counters
    unsigned int TPC = 0, SPC = 0;
    unsigned char RT, RA;
    unsigned short int r, n, k;
    bool rBOOL, nBOOL;
    // We need here code for specific opcode, dest, src1 and src2
    // Used to generate random instructions and a random number of those

    // This won't work exactly because we need to watch out for instruction forms that don't match any appropriate specific instruction format, maybe we will have to manually prepare it
    // instr format OP|DEST|SRC1|SRC2
//    cout << " da probam jos nesto " << ((65535) & 0x000F) <<" primjer hex instr " << std::hex << 65535 << " pomjereno do prvog " << ((0xFABC) & 0x000F) << std::endl;
//    (65535 & 0xF000) dobijanje prvog opcode
//    ((0xFABC) & 0x000F) src2
//    ((0xFABC >> 8) & 0x000F) dest
//    ((0xFABC >> 4) & 0x000F) src1
    srand (time(NULL));

    numberOfInstructions = rand() % 1000 + 100;
    for(int i = 0; i < numberOfInstructions; ++i) {
        memory[i] = rand() % 65536;

        // decide what routine it is to which to jump to
        switch(memory[i] & 0xF000) {
            case 0x0000:
                code[i].opcode = &&LOD;
                break;
            case 0x0001:
                code[i].opcode = &&ADD;
                break;
            case 0x0010:
                code[i].opcode = &&SUB;
                break;
            case 0x0011:
                code[i].opcode = &&AND;
                break;
            case 0x0100:
                code[i].opcode = &&ORA;
                break;
            case 0x0101:
                code[i].opcode = &&XOR;
                break;
            case 0x0110:
                code[i].opcode = &&SHR;
                break;
            case 0x0111:
                code[i].opcode = &&MUL;
                break;
            case 0x1000:
                code[i].opcode = &&STO;
                break;
            case 0x1001:
                code[i].opcode = &&LDC;
                break;
            case 0x1010:
                code[i].opcode = &&GTU;
                break;
            case 0x1011:
                code[i].opcode = &&GTS;
                break;
            case 0x1100:
                code[i].opcode = &&LTU;
                break;
            case 0x1101:
                code[i].opcode = &&LTS;
                break;
            case 0x1110:
                code[i].opcode = &&EQU;
                break;
            case 0x1111:
                code[i].opcode = &&MAJ;
                break;
        }

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
        registers[code[TPC].dest] = registers[code[TPC].src2];
        SPC += 4;
        TPC++;
        if(TPC == numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    ADD:
        registers[code[TPC].dest] = registers[code[TPC].src1] + registers[code[TPC].src2];
        SPC += 4;
        TPC++;
        if(TPC == numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    SUB:
        registers[code[TPC].dest] = registers[code[TPC].src1] - registers[code[TPC].src2];
        SPC += 4;
        TPC++;
        if(TPC == numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    AND:
        registers[code[TPC].dest] = registers[code[TPC].src1] & registers[code[TPC].src2];
        SPC += 4;
        TPC++;
        if(TPC == numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    ORA:
        registers[code[TPC].dest] = registers[code[TPC].src1] | registers[code[TPC].src2];
        SPC += 4;
        TPC++;
        if(TPC == numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    XOR:
        registers[code[TPC].dest] = registers[code[TPC].src1] ^ registers[code[TPC].src2];
        SPC += 4;
        TPC++;
        if(TPC == numberOfInstructions) goto EXIT;
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

        SPC += 4;
        TPC++;
        if(TPC == numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    MUL:
        registers[code[TPC].dest] = registers[code[TPC].src1] * registers[code[TPC].src2];
        SPC += 4;
        TPC++;
        if(TPC == numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    STO:
        memory[registers[code[TPC].src2]] = registers[code[TPC].src1];
        registers[code[TPC].dest] = registers[code[TPC].src1];
        SPC += 4;
        TPC++;
        if(TPC == numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    LDC:
        registers[code[TPC].dest] = (code[TPC].src1 | code[TPC].src2);
        SPC += 4;
        TPC++;
        if(TPC == numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    GTU:
        registers[code[TPC].dest] = registers[code[TPC].src1] > registers[code[TPC].src2] ? 0x0001 : 0x0000;
        SPC += 4;
        TPC++;
        if(TPC == numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    GTS:
        r = registers[code[TPC].src1];
        n = registers[code[TPC].src2];
        rBOOL = (r & 0x8000) == 0x0000 ? false : true;
        nBOOL = (n & 0x8000) == 0x0000 ? false : true;
        if(rBOOL && !nBOOL)
            registers[code[TPC].dest] = 0x0001;
        else if(!rBOOL && nBOOL)
            registers[code[TPC].dest] = 0x0000;
        else
            registers[code[TPC].dest] = registers[code[TPC].src1] > registers[code[TPC].src2] ? 0x0001 : 0x0000;

        SPC += 4;
        TPC++;
        if(TPC == numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    LTU:
        registers[code[TPC].dest] = registers[code[TPC].src1] < registers[code[TPC].dest] ? 0x0001 : 0x0000;
        SPC += 4;
        TPC++;
        if(TPC == numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    LTS:
        r = registers[code[TPC].src1];
        n = registers[code[TPC].src2];
        rBOOL = (r & 0x8000) == 0x0000 ? false : true;
        nBOOL = (n & 0x8000) == 0x0000 ? false : true;
        if(rBOOL && !nBOOL)
            registers[code[TPC].dest] = 0x0000;
        else if(!rBOOL && nBOOL)
            registers[code[TPC].dest] = 0x0001;
        else
            registers[code[TPC].dest] = registers[code[TPC].src1] < registers[code[TPC].src2] ? 0x0001 : 0x0000;

        SPC += 4;
        TPC++;
        if(TPC == numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    EQU:
        registers[code[TPC].dest] = registers[code[TPC].src1] == registers[code[TPC].src2] ? 0x0001 : 0x0000;
        SPC += 4;
        TPC++;
        if(TPC == numberOfInstructions) goto EXIT;
        goto *code[TPC].opcode;
    MAJ:

    EXIT:
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
    populateLseeekPos();
    populateFileModes();
    emulate();
    return 0;
}
