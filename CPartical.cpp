/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include <CPartical.h>
#include <CElement.h>
#include <CPcbNew_Parser.h>

#include <fstream>
#include <iostream>
#include <sstream>
using std::string;

#include <cctype>
#include <string>
#include <algorithm>
#include <cstdio>
#include <list>

/*static*/ uint32_t CPartical::ms_ulNoOfObjects = 0;

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
  return (std::find(CPcbNew_Parser::ms_Layers.begin(), CPcbNew_Parser::ms_Layers.end(), sLayer) != CPcbNew_Parser::ms_Layers.end());
}

void CPartical::evaluate() {
    m_sName = trim(m_sName);
    if ( m_sName == "gr_line" )
        m_pElement = new CElementLine("line", this);
    else if ( m_sName == "gr_circle" )
        m_pElement = new CElementCircle("circle", this);

    // if ( m_pElement != nullptr && m_pElement->m_sLayer != "Edge.Cuts" ) {
    if ( m_pElement != nullptr && !LayerContains(m_pElement->m_sLayer) ) {
      if ( CPcbNew_Parser::ms_nVerbose > 0 ) printf("delete %s -> layer %s\n", m_pElement->m_sName.c_str(), m_pElement->m_sLayer.c_str());
      delete m_pElement;
      m_pElement = nullptr;
    }
    if ( m_pElement )
        CPcbNew_Parser::m_Elements.push_back(m_pElement);
    for ( uint8_t n=0; n < m_Childs.size(); n++ )
        m_Childs[n]->evaluate();
}
