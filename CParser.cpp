/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include <CParser.h>

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

// #include "getopt/getopt.h"
#include "tclap/CmdLine.h"
using std::vector;

/*static*/ std::string CParser::ms_sFileName = "";
/*static*/ CPoint  CParser::ms_Offset;
/*static*/ list<CElement*> CParser::m_Elements;
/*static*/ int  CParser::ms_nVerbose = 0;
/*static*/ std::vector<string> CParser::ms_Layers;
/*static*/ int CParser::ms_nSpindleSpeed = 1000;
/*static*/ uint8_t CParser::ms_nCutCycles = 1;
/*static*/ uint8_t CParser::ms_nMatrixX = 1;
/*static*/ uint8_t CParser::ms_nMatrixY = 1;
/*static*/ double CParser::ms_dMatrixOffset = 5.0;
/*static*/ bool CParser::ms_bLaserMode = false;
/*static*/ bool CParser::ms_bAppendFile = false;
/*static*/ bool CParser::ms_bTestMode = false;
/*static*/ bool CParser::ms_bCreateFront = true;
/*static*/ bool CParser::ms_bCreateBack = true;
/*static*/ string CParser::ms_sExportPrefix = "";

int nLast = -1;
void printProgress(uint64_t n, uint64_t k, std::string sLog) {
  double progress = static_cast<float>(n) / static_cast<float>(k);
  int nCurrent = static_cast<int>(progress * 100.0);
  if ( (nLast == nCurrent) && (n != k) )
    return;
  nLast = progress;

  int barWidth = 70;
  std::cout << "[";
  int npos = barWidth * progress;
  for (int i = 0; i < barWidth; ++i) {
    if (i < npos)
      std::cout << "=";
    else if (i == npos)
      std::cout << ">";
    else
      std::cout << " ";
  }
  // std::cout << "] " << int(progress * 100.0) << "%  (" << sLog << ": " << n << " of " << k << ") \r";
  std::cout << "] " << nCurrent << "%  (" << n << " of " << k << ") \r";
  std::cout.flush();
}
void printProgress(uint64_t n, uint64_t k) {
  printProgress(n, k, "");
}

CParser::CParser(int argc, char** argv) {

}

CParser::~CParser() {
    // dtor
    while ( m_Elements.size() > 0 ) {
        CElement* pElement = *m_Elements.begin();
        m_Elements.erase(m_Elements.begin());
        delete pElement;
    }
}

bool CParser::Parse() {

  if ( m_Elements.size() == 0 ) {
    printf("0 Elements after parsing - exit\n");
    exit(-2);
  }
  printf("Found %lu gcode elements\n", m_Elements.size());

  if ( CParser_PcbNew::ms_nVerbose > 1 ) {
    printf("%s", _sELEM_);
    for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
      CElement* pElement = *it;
      printf("%s\n", pElement->print().c_str());
    }
    printf("%s\n\n", _sELEM_);
  }

  return true;
}

#define _sFILE_   "############################# FILE #############################\n"
#define _sELEM_   "########################### ELEMENTS ###########################\n"

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

bool CParser::Normalize() {
    printf("Normalize elements\n");
    CPoint min;
    CPoint max;
    (*m_Elements.begin())->GetEndPoint(&min);
    (*m_Elements.begin())->GetEndPoint(&max);
    int n = 0;
    for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
        CElement* pElement = *it;
        pElement->minmax(&min, &max);
        printProgress(++n, m_Elements.size());
    }
    printf("\n");

    min.normalize(ms_Offset);
    n = 0;
    if ( CParser::ms_nVerbose > 0 ) printf("Before: Min: %s, Max:%s\n", min.print().c_str(), max.print().c_str());
    for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
        CElement* pElement = *it;
        pElement->normalize(min);
        printProgress(++n, m_Elements.size());
    }
    printf("\n");

    if ( CParser::ms_nVerbose > 1 ) {
      printf("%s", _sELEM_);
      for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
        CElement* pElement = *it;
        printf("%s\n", pElement->print().c_str());
      }
      printf("%s\n\n", _sELEM_);
    }

    m_Min = CPoint();
    m_Max = max;
    m_Max.normalize(min);
    if ( CParser::ms_nVerbose > 0 ) printf("After: Min: %s, Max: %s\n", m_Min.print().c_str(), m_Max.print().c_str());

    return  true;
}

bool CParser::Sort() {
    printf("Sorting elements\n");
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
        printProgress(_out.size(), m_Elements.size());
    }
    printf("\n");
    // printf("Ending at %s\n", _current.print().c_str());
    m_Elements = _out;

    if ( CParser::ms_nVerbose > 1 ) {
      printf("%s", _sELEM_);
      for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
        CElement* pElement = *it;
        printf("%s\n", pElement->print().c_str());
      }
      printf("%s\n\n", _sELEM_);
    }

    return true;
}

bool CParser::Invert(CElement::_E_InvMode eInvMode) {
  printf("Invert elements, mode=%d\n", eInvMode);
  int n = 0;
  for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
      CElement* pElement = *it;
      pElement->invert(m_Max, eInvMode);
      printProgress(++n, m_Elements.size());
  }
  printf("\n");
  return true;
}

bool CParser::GenerateGCode(std::string sFileName) {
    printf("Generate %s - %u cycles\n", sFileName.c_str(), CParser::ms_nCutCycles);
    string sGCode;

    int nSpindle = 1000;

    sGCode += "(Block-name: Header)\n";
    sGCode += "(Block-expand: 0)\n";
    sGCode += "(Block-enable: 1)\n";
    sGCode += "; generated by CParser from " + ms_sFileName +"\n";
    sGCode += "G40\t; disable tool radius compensation\n";
    sGCode += "G49\t; disable tool length compensation\n";
    sGCode += "G80\t; cancel modal motion\n";
    sGCode += "G54\t; select coordinate system 1\n";
    sGCode += "G90\t; disable incremental moves\n";
    sGCode += "G21\t; metric\n";
    sGCode += CElement::GetGCodeFeedrate();
    sGCode += "G61\t; exact path mode\n";
    sGCode += "S" + std::to_string(nSpindle) + "\t; set spindle speed\n";
    if ( ms_bLaserMode )
    {
      sGCode += "M4\t; start laser\n";
    }
    else
    {
      sGCode += "M3\t; start spindle\n";
    }
    sGCode += "G04 P3.0\t; wait for 3.0 seconds\n";


    if ( ms_bTestMode ) {
      std::vector<int> vCycles;
      for ( uint8_t n=0; n < CParser::ms_nCutCycles; n++ )
        vCycles.push_back(n+1);
      std::vector<int> vFeedRate;
      CElement::ms_nFeedRateProcess = 500;
      for ( uint8_t n=0;n<3; n++ )
        vFeedRate.push_back((1+n)*CElement::ms_nFeedRateProcess/2);

      std::vector<int> vSpindleSpeed;
      for ( uint8_t n=0;n<2; n++ )
        vSpindleSpeed.push_back(750+250*(n));

      CElement::ms_dZProcess_n = CElement::ms_dZProcess;
      int k = 0;
      int nCutCycles = CParser::ms_nCutCycles;
      CPoint  _current;
      for ( uint8_t c=0; c<nCutCycles; c++ ) {
        for ( uint8_t s=0; s<2; s++ ) {
          for (uint8_t f=0; f<3; f++ ) {

            CPoint _offset((f+3*s)*(CParser::ms_dMatrixOffset+m_Max.m_dX), c*(CParser::ms_dMatrixOffset+m_Max.m_dY));

            CParser::ms_nCutCycles = vCycles[c];
            CParser::ms_nSpindleSpeed = vSpindleSpeed[s];
            CElement::ms_nFeedRateProcess = vFeedRate[f];

            printf("Cycle %d: Cuts=%d, SpindleSpeed=%d, FeedRate=%d, dx=%.1f, dy=%.1f\n", (k+1), CParser::ms_nCutCycles, CParser::ms_nSpindleSpeed, CElement::ms_nFeedRateProcess, _offset.m_dX, _offset.m_dY);

            // sGCode += "(Block-name: Cyc=" + std::to_string(CParser::ms_nCutCycles) + " S=" + std::to_string(CParser::ms_nSpindleSpeed) + " F=" + std::to_string(CElement::ms_nFeedRateProcess) + ")\n";

            for (uint8_t o=0; o<CParser::ms_nCutCycles; o++ ) {
              for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
                CElement* pElement = *it;
                if ( pElement->m_bToExport ) {
                  // printf("Start: %s\n", _current.print().c_str());
                  pElement->m_sBlockName = "Cuts_" + std::to_string(CParser::ms_nCutCycles) + "_SS_" + std::to_string(CParser::ms_nSpindleSpeed) + "_FR_" + std::to_string(CElement::ms_nFeedRateProcess);
                  sGCode += pElement->GetGCode(&_current, &_offset, it==m_Elements.begin() && o==0);
                  // printf("End: %s\n", _current.print().c_str());
                }
                printProgress(++k, m_Elements.size()*CParser::ms_nCutCycles*5*5);
              }
            }
          }
        }
      }
    } else {
      for ( uint8_t n=0; n < CParser::ms_nCutCycles; n++ )  {

        if ( CParser::ms_nCutCycles > 1 ) {
          printf("Cycle %u\n", n+1);
          sGCode += "(Block-name: Cycle " + std::to_string(n+1) + ")\n";
          sGCode += "(Block-expand: 0)\n";
          sGCode += "(Block-enable: 1)\n";
          sGCode += ";\n";
        }

        CElement::ms_dZProcess_n = -1.0 * fabs(CElement::ms_dZProcess * (n+1));
        int k = 0;
        for ( uint8_t x=0; x<CParser::ms_nMatrixX; x++ ) {
          for ( uint8_t y=0; y<CParser::ms_nMatrixY; y++ ) {
            CPoint  _current;
            CPoint _offset(x*(CParser::ms_dMatrixOffset+m_Max.m_dX), y*(CParser::ms_dMatrixOffset+m_Max.m_dY));
            printf("x=%d, y=%d, dx=%.1f, dy=%.1f", x, y, _offset.m_dX, _offset.m_dY);
            for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
              CElement* pElement = *it;
              if ( pElement->m_bToExport )
              sGCode += pElement->GetGCode(&_current, &_offset);
              printProgress(++k, m_Elements.size()*CParser::ms_nMatrixX*CParser::ms_nMatrixY);
            }
          }

        }
        printf("\n");
      }
    }


    sGCode += "(Block-name: End)\n";
    sGCode += "(Block-expand: 0)\n";
    sGCode += "(Block-enable: 1)\n";
    sGCode += "G0 M5\n";
    sGCode += "M2\t; end program\n";

    if ( CParser::ms_nVerbose > 2 ) {
      printf("EXPORT : %s \n", sFileName.c_str());
      printf("%s\n", sGCode.c_str());
      printf("EXPORT : %s \n", sFileName.c_str());
    }

    std::ios_base::openmode mode = std::ios_base::out;
    if ( CParser::ms_bAppendFile )
      mode |= std::ios_base::app;

    std::ofstream out(sFileName, mode);
    out << sGCode;
    out.close();

    printf("\n");

    return true;
}
