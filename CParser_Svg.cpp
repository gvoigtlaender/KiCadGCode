/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include <CParser_Svg.h>

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

#include <tinyxml2.h>

using std::vector;
using namespace tinyxml2;

CParser_Svg::CParser_Svg(int argc, char** argv) : CParser(argc, argv)
{
}

CParser_Svg::~CParser_Svg() {
    // dtor
    while ( m_Elements.size() > 0 ) {
        CElement* pElement = *m_Elements.begin();
        m_Elements.erase(m_Elements.begin());
        delete pElement;
    }
}

vector<string> split(string str, string token){
    vector<string>result;
    while(str.size()){
        long unsigned int index = str.find(token);
        if(index!=string::npos){
            result.push_back(str.substr(0,index));
            str = str.substr(index+token.size());
            if(str.size()==0)result.push_back(str);
        }else{
            result.push_back(str);
            str = "";
        }
    }
    return result;
}

bool CParser_Svg::Parse() {
    string sFileName = ms_sFileName;

    printf("Parsing %s\n", ms_sFileName.c_str());

    XMLDocument doc;
	  doc.LoadFile( ms_sFileName.c_str() );

    XMLElement* root = doc.FirstChildElement("svg");

    list<CElementLine*> Elements;
	  for ( XMLNode* pGroup = root->FirstChildElement("g"); pGroup; pGroup = pGroup->NextSiblingElement("g") ) {
      printf("pG = %s : %s\n", pGroup->ToElement()->Attribute("id"), pGroup->FirstChildElement("title")->ToElement()->GetText());

      ParsePaths(pGroup, Elements);
      ParseCircles(pGroup);
    }

    //! remove redundant paths
    while ( Elements.size() > 0 )
    {
      CElementLine* pLine = Elements.front();
      Elements.pop_front();

      for ( list<CElementLine*>::iterator it = Elements.begin(); it!=Elements.end(); it++ )
      {
        if ( pLine->m_Start == (*it)->m_Start && pLine->m_End == (*it)->m_End )
        {
          printf("%s == %s -> remove\n", pLine->print().c_str(), (*it)->print().c_str());
          delete pLine;
          pLine = NULL;
          break;
        }
        if ( pLine->m_Start == (*it)->m_End && pLine->m_End == (*it)->m_Start )
        {
          printf("%s == %s -> remove\n", pLine->print().c_str(), (*it)->print().c_str());
          delete pLine;
          pLine = NULL;
          break;
        }
      }
      if ( pLine )
        m_Elements.push_back(pLine);
    }

    return CParser::Parse();
}

bool CParser_Svg::Normalize() {
  return CParser::Normalize();
}

bool CParser_Svg::Sort() {
  return CParser::Sort();
}

bool CParser_Svg::Invert(CElement::_E_InvMode eInvMode) {
  printf("CParser_Svg::Invert\n");
  if ( eInvMode == CElement::y )
    return true;
  return CParser::Invert(eInvMode);
}

bool CParser_Svg::ParsePaths(void* pGroup, list<CElementLine*>& Elements) {

  string sGroup = static_cast<XMLNode*>(pGroup)->FirstChildElement("title")->ToElement()->GetText();
  for ( XMLNode* pPath = static_cast<XMLNode*>(pGroup)->FirstChildElement("path"); pPath; pPath = pPath->NextSiblingElement("path") ) {
    //printf("\tpP = %s\n", pPath->ToElement()->Attribute("id"));
    string sD = pPath->ToElement()->Attribute("d");
    printf("\tpP = %s : %s\n", pPath->ToElement()->Attribute("id"), sD.c_str());

    std::string s = sD;
    std::vector<std::string> vstrings = split(s, " ");

    printf("\t %lu partials found\n", vstrings.size());

    CPoint* p0 = NULL;
    for ( unsigned int n=0;n<vstrings.size(); n++ )
    {
      if ( vstrings[n] == "" )
        break;
      double dX = std::stod(vstrings[n+1]);
      double dY = std::stod(vstrings[n+2]);

      CPoint p{dX, dY};
      printf("\t%u %.1f %.1f\n", n, dX, dY);


      if ( p0 == NULL )
        p0 = new CPoint(dX, dY);
      else
      {
        CElementLine* pLine = new CElementLine(sGroup + std::to_string(n));
        pLine->m_Start = *p0;
        pLine->m_End = p;
        Elements.push_back(pLine);
        *p0 = p;
      }
      n += 2;
    }
    if ( p0 )
      delete p0;

  }

  return true;
}

bool CParser_Svg::ParseCircles(void* pGroup) {

  string sGroup = static_cast<XMLNode*>(pGroup)->FirstChildElement("title")->ToElement()->GetText();
  for ( XMLNode* pCircle = static_cast<XMLNode*>(pGroup)->FirstChildElement("circle"); pCircle; pCircle = pCircle->NextSiblingElement("circle") ) {

    printf("\tpC = %s : %s : %s\n", pCircle->ToElement()->Attribute("cx"), pCircle->ToElement()->Attribute("cy"), pCircle->ToElement()->Attribute("r"));

    double dX = std::stod(pCircle->ToElement()->Attribute("cx"));
    double dY = std::stod(pCircle->ToElement()->Attribute("cy"));
    double dR = std::stod(pCircle->ToElement()->Attribute("r"));

    CElementCircle* pCirc = new CElementCircle(sGroup);
    pCirc->m_Center = CPoint(dX, dY);
    pCirc->m_dRadius = dR;
    m_Elements.push_back(pCirc);
  }
  return true;
}
