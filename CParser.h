/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef CPARSER_H_
#define CPARSER_H_

#include <string>
#include <vector>
#include <list>
using std::string;
using std::list;
using std::vector;

void printProgress(uint64_t n, uint64_t k, std::string sLog);
void printProgress(uint64_t n, uint64_t k);

#define _sELEM_   "########################### ELEMENTS ###########################\n"

#include <CPoint.hpp>
#include <CElement.h>
class CParser {
 public:
    CParser(int argc, char** argv);
    virtual ~CParser();

    virtual bool Parse();
    virtual bool Normalize();
    virtual bool Sort();
    virtual bool Invert(CElement::_E_InvMode eInvMode);
    bool GenerateGCode(std::string sFileName);

    static std::string ms_sFileName;

    static CPoint  ms_Offset;
    CPoint  m_Min;
    CPoint  m_Max;

    static list<CElement*> m_Elements;
    static int  ms_nVerbose;
    static vector<string> ms_Layers;
    static int ms_nSpindleSpeed;
    static uint8_t ms_nCutCycles;
    static uint8_t ms_nMatrixX;
    static uint8_t ms_nMatrixY;
    static double ms_dMatrixOffset;
    static bool ms_bLaserMode;
    static bool ms_bAppendFile;
    static bool ms_bTestMode;
    static bool ms_bCreateFront;
    static bool ms_bCreateBack;
    static string ms_sExportPrefix;
};

#endif  // CPARSER_H_
