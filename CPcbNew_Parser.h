/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef CPCBNEW_PARSER_H_
#define CPCBNEW_PARSER_H_

#include <string>
#include <vector>
#include <list>
using std::string;
using std::list;

#include <CPoint.hpp>

class CPartical;
class CElement;
class CPcbNew_Parser {
 public:
    CPcbNew_Parser(int argc, char** argv);
    virtual ~CPcbNew_Parser();

    bool Parse();
    bool Normalize();
    bool Sort();
    bool Invert();
    bool GenerateGCode(std::string sFileName);

    std::string m_sFileName;
    CPartical* m_pRootPartical;

    CPoint  m_Min;
    CPoint  m_Max;

    static list<CElement*> m_Elements;
};

#endif  // CPCBNEW_PARSER_H_
