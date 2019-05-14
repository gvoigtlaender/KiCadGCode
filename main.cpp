/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include <main.h>
#include <iostream>
#include <string>
#include "CPcbNew_Parser.h"

void usage() {
  printf("Aborting: no input file provided\n");
  printf("Usage:\n");
  printf("\t./KiCadGCode <filename>\n");
}

int main(int argc, char** argv) {
    CPcbNew_Parser* pParser = new CPcbNew_Parser(argc, argv);

    pParser->Parse();
    pParser->Normalize();
    pParser->Sort();
    if ( pParser->m_bCreateFront ) {
      pParser->GenerateGCode(pParser->m_sFileName + "_f.ngc");
    }
    if ( pParser->m_bCreateBack ) {
      pParser->Invert();
      pParser->Normalize();
      pParser->Sort();
      pParser->GenerateGCode(pParser->m_sFileName + "_b.ngc");
    }
    return 0;
}
