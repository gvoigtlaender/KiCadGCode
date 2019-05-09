/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
// #include <main.h>
#include <iostream>
#include <string>
#include "CPcbNew_Parser.h"


int main(int argc, char** argv) {
    CPcbNew_Parser* pParser = new CPcbNew_Parser(argc, argv);

    if (pParser->m_sFileName != "" )
      pParser->Parse();
    else
      pParser->Parse("2.kicad_pcb");
    pParser->Normalize();
    pParser->Sort();
    pParser->GenerateGCode("2_f.ngc");
    pParser->Invert();
    pParser->Sort();
    pParser->GenerateGCode("2_b.ngc");
    return 0;
}
