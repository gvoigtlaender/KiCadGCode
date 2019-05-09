/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include "CPcbNew_Parser.h"

#include <fstream>
#include <iostream>
#include <sstream>


#include <cctype>
#include <string>
#include <algorithm>
#include <cstdio>
#include <vector>

#include <CElement.h>
#include <CPartical.h>

using std::vector;

/*static*/ list<CElement*> CPcbNew_Parser::m_Elements;

vector<string> split(const string& str, const string& delim) {
    vector<string> tokens;
    size_t prev = 0, pos = 0;
    do {
        pos = str.find(delim, prev);
        if (pos == string::npos) pos = str.length();
        string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    } while (pos < str.length() && prev < str.length());
    return tokens;
}

CPcbNew_Parser::CPcbNew_Parser(int argc, char** argv)
: m_sFileName("")
, m_pRootPartical(nullptr) {
  for (int i = 1; i < argc; ++i) {
      vector<string> vArg = split(argv[i], "=");

      std::cout << "You have entered " << argc
           << " arguments:" << "\n";

      for (int i = 0; i < argc; ++i)
          std::cout << argv[i] << "\n";


      string sParam = vArg[0];

      if ( sParam == "" ) {
      } else if ( sParam == "pcb" ) {
        if ( vArg.size() == 2 ) {
          m_sFileName = vArg[1];
          printf("pcb: %s\n", m_sFileName.c_str());
        }
      }
    }
    // ctor
}

CPcbNew_Parser::~CPcbNew_Parser() {
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
#define _sELEM_   "########################### ELEMENTS ###########################\n"




bool CPcbNew_Parser::Parse(string sFileName) {
    m_sFileName = sFileName;

    std::ifstream inFile;
    inFile.open(sFileName);  // open the input file
    if  ( inFile.fail() ) {
      printf("inFile.fail()\n");
      exit(-1);
    }

    std::stringstream strStream;
    strStream << inFile.rdbuf();  // read the file
    std::string content = strStream.str();  // str holds the content of the file

    /*
    printf("%s", _sFILE_);
    printf("%s\n", content.c_str());
    printf("%s\n\n", _sFILE_);
    */

    CPartical* pCurrent = nullptr;

    for ( uint64_t pos = 0; pos < content.length(); pos++ ) {
        char c = content[pos];
        //! enter partical
        if (c == '(') {
            CPartical* p = new CPartical(pCurrent);
            if ( !m_pRootPartical )
                m_pRootPartical = p;
            pCurrent = p;
        } else if (c == ')') {
            //! leave partialcal
            if ( pCurrent )
                pCurrent = pCurrent->m_pParent;
        } else if ( c == '\n' ) {
        } else if ( c == '\r' ) {
        } else if ( c == '\t' ) {
        } else {
            if ( pCurrent && pCurrent->m_Childs.size() == 0 )
                pCurrent->m_sName += c;
        }
    }

    // printf("%s", _sPART_);
    if ( m_pRootPartical ) {
        m_pRootPartical->evaluate();
        // m_pRootPartical->print(0);
    }
    // printf("%s\n\n", _sPART_);

    /*
    printf("%s", _sELEM_);
    for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
        CElement* pElement = *it;
        printf("%s\n", pElement->print().c_str());
    }
    printf("%s\n\n", _sELEM_);
    */

    return true;
}

bool CPcbNew_Parser::Normalize() {
    printf("Normalize()\n");
    CPoint min;
    CPoint max;
    (*m_Elements.begin())->GetEndPoint(&min);
    (*m_Elements.begin())->GetEndPoint(&max);
    for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
        CElement* pElement = *it;
        pElement->minmax(&min, &max);
    }

    printf("\nMin: %s, Max:%s\n", min.print().c_str(), max.print().c_str());
    for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
        CElement* pElement = *it;
        pElement->normalize(min);
    }

    /*
    printf("%s", _sELEM_);
    for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
        CElement* pElement = *it;
        printf("%s\n", pElement->print().c_str());
    }
    printf("%s\n\n", _sELEM_);
    */

    m_Min = CPoint();
    m_Max = max;
    m_Max.normalize(min);
    printf("min: %s, max: %s\n", m_Min.print().c_str(), m_Max.print().c_str());

    return  true;
}

bool CPcbNew_Parser::Sort() {
    printf("Sorting\n");
    list<CElement*> _in = m_Elements;
    list<CElement*> _out;

    CPoint  _current;
    CPoint  _end;

    // printf("Starting at %s\n", _current.print().c_str());
    while ( _in.size() > 0 ) {
        _in.sort([_current](const CElement* a, const CElement* b) {
            double dDist_a = a->GetDistance(_current);
            double dDist_b = b->GetDistance(_current);
            return dDist_a <= dDist_b;
        });

        CElement* pFirst = *_in.begin();
        _in.pop_front();

        pFirst->GetEndPoint(&_current);
        _out.push_back(pFirst);
        // printf("%s\n", pFirst->print().c_str());
        // printf("Waypoint at at %s\n", _current.print().c_str());
    }
    // printf("Ending at %s\n", _current.print().c_str());
    m_Elements = _out;

    /*
    printf("%s", _sELEM_);
    for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
        CElement* pElement = *it;
        printf("%s\n", pElement->print().c_str());
    }
    printf("%s\n\n", _sELEM_);
    */

    return true;
}

bool CPcbNew_Parser::Invert() {
  for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
      CElement* pElement = *it;
      pElement->invert(m_Max, CElement::x);
  }
  return true;
}
bool CPcbNew_Parser::GenerateGCode(std::string sFileName) {
    string sGCode;

    int nFeedRate = 300;
    int nSpindle = 1000;

    sGCode += "(Block-name: Header)\n";
    sGCode += "(Block-expand: 0)\n";
    sGCode += "(Block-enable: 1)\n";
    sGCode += "; generated by CPcbNew_Parser from " + m_sFileName +"\n";
    sGCode += "G40\t; disable tool radius compensation\n";
    sGCode += "G49\t; disable tool length compensation\n";
    sGCode += "G80\t; cancel modal motion\n";
    sGCode += "G54\t; select coordinate system 1\n";
    sGCode += "G90\t; disable incremental moves\n";
    sGCode += "G21\t; metric\n";
    sGCode += "F" + std::to_string(nFeedRate) + "\t; set feedrate\n";
    sGCode += "G61\t; exact path mode\n";
    sGCode += "S" + std::to_string(nSpindle) + "\t; set spindle speed\n";
    sGCode += "M3\t; start spindle\n";
    sGCode += "G04 P3.0\t; wait for 3.0 seconds\n";

    CPoint  _current;
    for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
        CElement* pElement = *it;
        sGCode += pElement->GetGCode(&_current);
    }

    sGCode += "(Block-name: End)\n";
    sGCode += "(Block-expand: 0)\n";
    sGCode += "(Block-enable: 1)\n";
    sGCode += "G0 M5\n";
    sGCode += "M2\t; end program\n";

    /*
    printf("EXPORT : %s \n", sFileName.c_str());
    printf("%s\n", sGCode.c_str());
    printf("EXPORT : %s \n", sFileName.c_str());
    */

    std::ofstream out(sFileName);
    out << sGCode;
    out.close();

    return true;
}
