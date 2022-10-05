#include <iostream>
#include "FilePipes.h"

int main(int argc, char *argv[]) {
    std::string buf;
    OutputFilePipe opi("test.pipe");
    InputFilePipe ipi("test.pipe");

    opi << "content";
    ipi >> buf;
    std::cout << buf << std::endl;;

    opi << "content2";
    ipi >> buf;
    std::cout << buf << std::endl;

    opi << "content3";
    ipi >> buf;
    std::cout << buf << std::endl;;
    return 0;
}