/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include <CPartical.h>
#include <CElement.h>
#include <CParser.h>

#include <fstream>
#include <iostream>
#include <sstream>
using std::string;

#include <cctype>
#include <string>
#include <algorithm>
#include <cstdio>
#include <list>

/*static*/ uint64_t CPartical::ms_ulNoOfObjects = 0;

void printProgress(uint64_t n, uint64_t k, std::string sLog);
/*static*/ void CPartical::show_progress() {
  printProgress(++CPartical::ms_ulProgress, CPartical::ms_ulProgressMax, std::to_string(m_ulId));
  for (uint16_t a=0; a < 10000; a++ ) { }
}
/*static*/ void CPartical::reset_progress() {
  CPartical::ms_ulProgress = 0;
  CPartical::ms_ulProgressMax = CPartical::ms_ulNoOfObjects;
}
/*static*/ uint64_t CPartical::ms_ulProgressMax = 0;
/*static*/ uint64_t CPartical::ms_ulProgress = 0;


inline std::string trim(const std::string &s) {
  auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c){return std::isspace(c);});
  auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c){return std::isspace(c);}).base();
  return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
}

void CPartical::print(uint8_t nIdx /* =  0 */) {
    std::string s = "";
    for ( uint8_t t=0; t < nIdx; t++ )
        s += ' ';
    s += m_sName;
    if ( m_pElement != nullptr ) {
        s += m_pElement->print();
    }
    if ( m_pElement != nullptr || m_pParent == nullptr /*|| m_pParent->m_pElement != nullptr*/ )
        printf("%s\n", s.c_str());
    // cout << s << endl;
    for ( uint8_t n=0; n < m_Childs.size(); n++ )
        m_Childs[n]->print(nIdx+1);
}

bool LayerContains(const string& sLayer) {
  return (std::find(CParser::ms_Layers.begin(), CParser::ms_Layers.end(), sLayer) != CParser::ms_Layers.end());
}

void CPartical::evaluate() {
    m_sName = trim(m_sName);
    if ( CParser::ms_nVerbose >= 1 )
       printf("CPartical::evaluate(%s)\n", m_sName.c_str());
    if ( m_sName == "gr_line" )
        m_pElement = new CElementLine("line", this);
    else if ( m_sName == "gr_circle" )
        m_pElement = new CElementCircle("circle", this);
    else if ( m_sName == "gr_arc" )
        m_pElement = new CElementArc("arc", this);
    else if ( m_sName == "gr_text StartHere" )
    {
      CElementText* pText = new CElementText("text", this);
          m_pElement = pText;
          if ( m_pElement->m_sLayer == "Cmts.User" )
            m_pElement->m_sLayer = "Edge.Cuts";
          pText->m_sText = m_sName.substr(8);
    }

    // if ( m_pElement != nullptr && m_pElement->m_sLayer != "Edge.Cuts" ) {
    if ( m_pElement != nullptr && !LayerContains(m_pElement->m_sLayer) ) {
      if ( CParser::ms_nVerbose > 0 ) printf("delete %s -> layer %s\n", m_pElement->m_sName.c_str(), m_pElement->m_sLayer.c_str());
      delete m_pElement;
      m_pElement = nullptr;
    }
    if ( m_pElement ) {
      if ( m_pElement->m_sLayer != "Edge.Cuts" )
      m_pElement->m_bToExport = false;
      CParser::m_Elements.push_back(m_pElement);
    }
    show_progress();
    for ( uint8_t n=0; n < m_Childs.size(); n++ )
        m_Childs[n]->evaluate();
}
