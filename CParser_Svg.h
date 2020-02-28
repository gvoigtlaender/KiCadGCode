/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef CPARSER_SVG_H_
#define CPARSER_SVG_H_


#include <CParser.h>

class CParser_Svg : public CParser {
 public:
    CParser_Svg(int argc, char** argv);
    virtual ~CParser_Svg();

    bool Parse() override;
    bool Normalize() override;
    bool Sort() override;
    bool Invert(CElement::_E_InvMode eInvMode) override;

    bool ParsePaths(void* pGroup, list<CElementLine*>& Elements);
    bool ParseCircles(void* pGroup);

};

#endif  // CPARSER_SVG_H_
