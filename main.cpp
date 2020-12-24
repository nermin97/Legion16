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

unsigned short int memory[65536]={0x0000}, registers[16], noOfInstructions; // regs[15] je PC
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
    // registers[15] is PC=Program counter register
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
    int i = 0;
    unsigned short int opcode, r, n, k;
    bool r1, n1;
    memory[0] = 366;
    opcode = memory[0];

    switch(opcode & 0xF000) {
        case 0x0000:
            cout << "jeste, tu smo stigli " << std::endl;
            break;
        default:
            cout << "nije to to, u default smo" << std::endl;
    }
//    opcode
//    cout << mem << std::endl;
}

void generateInstructionsRandomly () {
    // We use this to generate random instructions and number of instructions
    srand (time(NULL));
    unsigned short int numberOfInstructions = rand() % 1000 + 100;

    for(int i = 0; i < numberOfInstructions; ++i)
        memory[i] = rand() % 65536 + 0;
//    cout << "prvi " << memory[0] << " broj inst " << numberOfInstructions << std::endl;

//    mem
//    cout << instruction << " int verzija" << std::hex << instruction << " instrukcija" << std::endl;
}

void loadInstructions() {
    std::fstream memFile;
    memFile.open("memory.txt", std::fstream::in|std::fstream::out);
    if(memFile.is_open()) cout << "otvorilo se" << std::endl;
    unsigned short int instr;
    // mem[0] = {nesto} stavlja sve vrijednosti na to
//    mem[0] = 0x916e;
//    cout << "vrijednost " << mem[0] << std::hex << mem[0] << std::endl;
//    while(memFile >> std::hex >> instr) {
//        cout << std::hex << instr;
//        mem[noOfInstructions] = instr;
//        noOfInstructions++;
//    }

}

int main() {

//    populateOpenModes();
//    populateLseeekPos();
//    populateFileModes();
//    setRegsToZero(); //PC postavimo na nultu poziciju
//    loadInstructions(); //Očitavamo instrukcije iz mem.txt
//
//    interpret();
//    loadInstructions();
//    generateInstructionsRandomly();
    emulate();
    generateInstructionsRandomly();
//    std::cout << "Emulation finished!" << std::endl;
    return 0;
}
