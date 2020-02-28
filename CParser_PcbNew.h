/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef CPARSER_PCBNEW_H_
#define CPARSER_PCBNEW_H_


#include <CParser.h>
class CPartical;
class CParser_PcbNew : public CParser {
 public:
    CParser_PcbNew(int argc, char** argv);
    virtual ~CParser_PcbNew();

    bool Parse();

    CPartical* m_pRootPartical;
};

#endif  // CPARSER_PCBNEW_H_
