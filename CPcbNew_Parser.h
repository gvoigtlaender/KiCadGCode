/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef CPCBNEW_PARSER_H_
#define CPCBNEW_PARSER_H_

#include <string>
#include <vector>
#include <list>
using std::string;
using std::list;
using std::vector;

#include <CPoint.hpp>
#include <CElement.h>
class CPartical;
class CPcbNew_Parser {
 public:
    CPcbNew_Parser(int argc, char** argv);
    virtual ~CPcbNew_Parser();

    bool Parse();
    bool Normalize();
    bool Sort();
    bool Invert(CElement::_E_InvMode eInvMode);
    bool GenerateGCode(std::string sFileName);

    std::string m_sFileName;
    CPartical* m_pRootPartical;

    CPoint  m_Offset;
    CPoint  m_Min;
    CPoint  m_Max;

    static list<CElement*> m_Elements;
    static int  ms_nVerbose;
    static vector<string> ms_Layers;
    static int ms_nSpindleSpeed;
    static uint8_t ms_nCutCycles;
    bool m_bCreateFront;
    bool m_bCreateBack;
    string m_sExportPrefix;
};

#endif  // CPCBNEW_PARSER_H_
