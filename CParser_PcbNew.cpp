/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include <CParser_PcbNew.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

#include <cctype>
#include <string>
#include <algorithm>
#include <cstdio>
#include <vector>

#include <CElement.h>
#include <CPartical.h>

using std::vector;


CParser_PcbNew::CParser_PcbNew(int argc, char** argv) : CParser(argc, argv)
, m_pRootPartical(nullptr) {
}

CParser_PcbNew::~CParser_PcbNew() {
    // dtor
    if ( m_pRootPartical != nullptr )
        delete m_pRootPartical;

    while ( m_Elements.size() > 0 ) {
        CElement* pElement = *m_Elements.begin();
        m_Elements.erase(m_Elements.begin());
        delete pElement;
    }
}

#define _sFILE_   "############################# FILE #############################\n"
#define _sPART_   "########################### PARTICALS ##########################\n"

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

bool EntryToMask(const std::string& sName) {

  string sMaskedNames[] = {
    "general",
    "page",
    "layers",
    "setup",
    "net",
    "net_class",
    "module",
    "segment",
    "via",
//     "gr_text",
    "gr_poly",
    "zone",
    "tstamp",
    "effects",
    "font",
    "size",
    "thickness"
  };

  for (const string &text : sMaskedNames) {
    if ( sName.find(text) == 0 ) {
      // printf("%s masked by %s\n", sName.c_str(), sMaskedNames[n].c_str());
      return true;
    }
  }

  return false;
}

bool CParser_PcbNew::Parse() {
    string sFileName = ms_sFileName;

    printf("Parsing %s\n", ms_sFileName.c_str());

    std::ifstream inFile;
    inFile.open(sFileName);  // open the input file
    if  ( inFile.fail() ) {
      printf("inFile.fail()\n");
      exit(-1);
    }

    std::stringstream strStream;
    strStream << inFile.rdbuf();  // read the file
    inFile.close();

    std::string content = strStream.str();  // str holds the content of the file
    strStream.clear();

    content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
    content.erase(std::remove(content.begin(), content.end(), '\a'), content.end());

    if ( CParser_PcbNew::ms_nVerbose > 2 ) {
      printf("%s", _sFILE_);
      printf("%s\n", content.c_str());
      printf("%s\n\n", _sFILE_);
    }

    CPartical* pCurrent = nullptr;

    if ( CParser_PcbNew::ms_nVerbose > 0 ) printf("evaluating kicad elements .");
    uint nSkippedEntry = 0;
    for ( uint64_t pos = 0; pos < content.length(); pos++ ) {
        char c = content[pos];
        //! enter partical
        if (c == '(') {
            if ( pCurrent && EntryToMask(pCurrent->m_sName) ) {
              // printf("Skip sub entries for name %s\n", pCurrent->m_sName.c_str() );
              pCurrent->m_bToSkip = true;
              nSkippedEntry++;
            } else if ( pCurrent ) {
              // printf("Allow sub entries for name %s\n", pCurrent->m_sName.c_str() );
            }
            if ( nSkippedEntry == 0 ) {
              CPartical* p = new CPartical(pCurrent, false);
              if ( !m_pRootPartical )
                  m_pRootPartical = p;
              pCurrent = p;
              // std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        } else if (c == ')') {
            //! leave partialcal
            if ( pCurrent ) {
              if ( nSkippedEntry == 0 ) {
                CPartical* pParent = pCurrent->m_pParent;
                if ( EntryToMask(pCurrent->m_sName) )
                  pCurrent->m_bToSkip = true;
                if ( pCurrent->m_bToSkip ) {
                  // printf("delete skipped entry %s\n", pCurrent->m_sName.c_str());
                  pParent->m_Childs.pop_back();
                  delete pCurrent;
                }
                pCurrent = pParent;
              } else {

              }
            }
            if ( nSkippedEntry > 0 )
              nSkippedEntry--;
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        } else if ( c == '\n' ) {
        } else if ( c == '\r' ) {
        } else if ( c == '\t' ) {
        } else {
            if ( nSkippedEntry == 0 && pCurrent && pCurrent->m_Childs.size() == 0 )
                pCurrent->m_sName += c;
        }
        printProgress(pos+1, content.length());
    }
    printf("\n");

    if ( CParser_PcbNew::ms_nVerbose > 0 ) printf(" done \n\n");

    printf("Found %lu kicad elements\n", CPartical::ms_ulNoOfObjects);

    CPartical::reset_progress();
    if ( CParser_PcbNew::ms_nVerbose > 1 ) printf("%s", _sPART_);
    if ( m_pRootPartical ) {
        m_pRootPartical->evaluate();
        if ( CParser_PcbNew::ms_nVerbose > 1 ) m_pRootPartical->print(0);
    }
    printf("\n");

    if ( CParser_PcbNew::ms_nVerbose > 1 ) printf("%s\n\n", _sPART_);

    Invert(CElement::y);

    return CParser::Parse();
}
