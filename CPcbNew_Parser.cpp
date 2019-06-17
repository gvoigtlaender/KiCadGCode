/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include <CPcbNew_Parser.h>

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

// #include "getopt/getopt.h"
#include "tclap/CmdLine.h"
using std::vector;

/*static*/ list<CElement*> CPcbNew_Parser::m_Elements;
/*static*/ int  CPcbNew_Parser::ms_nVerbose = 0;
/*static*/ std::vector<string> CPcbNew_Parser::ms_Layers;
/*static*/ int CPcbNew_Parser::ms_nSpindleSpeed = 1000;
/*static*/ uint8_t CPcbNew_Parser::ms_nCutCycles = 1;

CPcbNew_Parser::CPcbNew_Parser(int argc, char** argv)
: m_sFileName("")
, m_pRootPartical(nullptr)
, m_bCreateFront(true)
, m_bCreateBack(true)
, m_sExportPrefix("") {

  try {
    TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
    TCLAP::UnlabeledValueArg<string> filename("filename", "filename2", "filename3", "string", "");
    cmd.add(filename);

    TCLAP::MultiSwitchArg verbose("v", "verbose", "Be verbosive", false);
    cmd.add(verbose);

    TCLAP::ValueArg<double> zsafe("s", "z_safe", "Z-Safety position to use for non-processing x/y-moves", false, CElement::ms_dZSafe, "double");
    cmd.add(zsafe);
    TCLAP::ValueArg<double> zprocess("p", "z_process", "z-Process position to use for processing x/y-moves (cutting depth)", false, CElement::ms_dZProcess, "double");
    cmd.add(zprocess);

    TCLAP::ValueArg<int> ncycles("c", "cycles", "cutting cycles", false, CPcbNew_Parser::ms_nCutCycles, "int");
    cmd.add(ncycles);

    vector<string> allowed_layers;
    allowed_layers.push_back("Edge.Cuts");
    allowed_layers.push_back("B.Fab");
    allowed_layers.push_back("Dwgs.User");
    TCLAP::ValuesConstraint<string> allowedLayers(allowed_layers);
    TCLAP::MultiArg<string> layers("l", "layer", "Layer to parse, only Edge.Cuts is included in gcode", false, &allowedLayers);
    cmd.add(layers);

    vector<string> allowed_side;
    allowed_side.push_back("front");
    allowed_side.push_back("back");
    allowed_side.push_back("both");
    TCLAP::ValuesConstraint<string> allowedSides(allowed_side);
    TCLAP::ValueArg<string> side("e", "export_sides", "PCB side gcode to export", false, "both", &allowedSides);
    cmd.add(side);

    TCLAP::ValueArg<double> x0("x", "x_offset", "X-Offset (lower left corner to zero)", false, 0.0, "double");
    cmd.add(x0);
    TCLAP::ValueArg<double> y0("y", "y_offset", "Y-Offset (lower left corner to zero)", false, 0.0, "double");
    cmd.add(y0);

    TCLAP::ValueArg<int> feedrate("", "feedrate", "Feed rate for non-process moves", false, CElement::ms_nFeedRate, "int");
    cmd.add(feedrate);

    TCLAP::ValueArg<int> processfeedrate("", "processfeedrate", "Feed rate for process moves", false, CElement::ms_nFeedRateProcess, "int");
    cmd.add(processfeedrate);

    TCLAP::ValueArg<int> plungefeedrate("", "plungefeedrate", "Feed rate for plunge moves (z-down)", false, CElement::ms_nFeedRatePlunge, "int");
    cmd.add(plungefeedrate);

    TCLAP::ValueArg<int> spindlespeed("", "spindlespeed", "Spindle rotation speed [u/min]", false, ms_nSpindleSpeed, "int");
    cmd.add(spindlespeed);

    vector<string> allowed_shapes;
    allowed_shapes.push_back("line");
    allowed_shapes.push_back("circle");
    TCLAP::ValuesConstraint<string> allowedShapes(allowed_shapes);
    TCLAP::MultiArg<string> shapes("", "shapes", "Shapes to include in gcode", false, &allowedShapes);
    cmd.add(shapes);

    TCLAP::ValueArg<string> prefix("", "prefix", "File name prefix to export", false, "", "string");
    cmd.add(prefix);

    cmd.parse(argc, argv);

    m_sFileName = filename.getValue();
    CPcbNew_Parser::ms_nVerbose = verbose.getValue();

    CElement::ms_dZSafe = zsafe.getValue();
    CElement::ms_dZProcess = zprocess.getValue();

    vector<string> vlayers = layers.getValue();
    if ( vlayers.size() > 0 ) {
      CPcbNew_Parser::ms_Layers = vlayers;
    } else {
      CPcbNew_Parser::ms_Layers = allowed_layers;
    }

    vector<string> vshapes = shapes.getValue();
    if ( vshapes.size() == 0 ) {
      CElementCircle::ms_bToExport = true;
      CElementLine::ms_bToExport = true;
    } else {
      for ( unsigned int j=0; j < vshapes.size(); j++ ) {
        string sshape = vshapes[j];
        if ( sshape == "circle" )
          CElementCircle::ms_bToExport = true;
        else if ( sshape == "line" )
          CElementLine::ms_bToExport = true;
      }
    }

    string sSide = side.getValue();
    if ( sSide == "front" ) m_bCreateBack = false;
    if ( sSide == "back" )  m_bCreateFront =  false;

    m_Offset.m_dX = x0.getValue();
    m_Offset.m_dY = y0.getValue();

    CElement::ms_nFeedRate = feedrate.getValue();
    CElement::ms_nFeedRatePlunge = plungefeedrate.getValue();
    CElement::ms_nFeedRateProcess = processfeedrate.getValue();
    ms_nSpindleSpeed = spindlespeed.getValue();

    m_sExportPrefix = prefix.getValue();

    CPcbNew_Parser::ms_nCutCycles = ncycles.getValue();

  } catch (TCLAP::ArgException &e) {
    // catch any exceptions
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(-1);
  }

  if ( CPcbNew_Parser::ms_nVerbose > 0 ) {
    printf("using arguments: f=%s, v=%d, s=%.3f, p=%.3f, Layers:{", m_sFileName.c_str(), CPcbNew_Parser::ms_nVerbose, CElement::ms_dZSafe, CElement::ms_dZProcess);
    for ( unsigned int n=0; n < CPcbNew_Parser::ms_Layers.size(); n++ )
      printf("[%s]", CPcbNew_Parser::ms_Layers[n].c_str());
    printf("}");
    printf(", f=%s, b=%s\n", std::to_string(m_bCreateFront).c_str(), std::to_string(m_bCreateBack).c_str());
  }
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

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

void printProgress(uint64_t n, uint64_t k) {
  double progress = static_cast<float>(n) / static_cast<float>(k);

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
  std::cout << "] " << int(progress * 100.0) << "%  (" << n << " of " << k << ") \r";
  std::cout.flush();}

bool CPcbNew_Parser::Parse() {
    string sFileName = m_sFileName;

    printf("Parsing %s\n", m_sFileName.c_str());

    std::ifstream inFile;
    inFile.open(sFileName);  // open the input file
    if  ( inFile.fail() ) {
      printf("inFile.fail()\n");
      exit(-1);
    }

    std::stringstream strStream;
    strStream << inFile.rdbuf();  // read the file
    std::string content = strStream.str();  // str holds the content of the file

    if ( CPcbNew_Parser::ms_nVerbose > 1 ) {
      printf("%s", _sFILE_);
      printf("%s\n", content.c_str());
      printf("%s\n\n", _sFILE_);
    }

    CPartical* pCurrent = nullptr;

    if ( CPcbNew_Parser::ms_nVerbose > 0 ) printf("evaluating kicad elements .");

    for ( uint64_t pos = 0; pos < content.length(); pos++ ) {
        char c = content[pos];
        //! enter partical
        if (c == '(') {
            CPartical* p = new CPartical(pCurrent);
            if ( !m_pRootPartical )
                m_pRootPartical = p;
            pCurrent = p;
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        } else if (c == ')') {
            //! leave partialcal
            if ( pCurrent )
                pCurrent = pCurrent->m_pParent;
            if ( CPcbNew_Parser::ms_nVerbose > 0 ) printf(".");
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        } else if ( c == '\n' ) {
        } else if ( c == '\r' ) {
        } else if ( c == '\t' ) {
        } else {
            if ( pCurrent && pCurrent->m_Childs.size() == 0 )
                pCurrent->m_sName += c;
        }
        printProgress(pos+1, content.length());
    }
    printf("\n");

    if ( CPcbNew_Parser::ms_nVerbose > 0 ) printf(" done \n\n");

    printf("Found %lu kicad elements\n", CPartical::ms_ulNoOfObjects);

    CPartical::reset_progress();
    if ( CPcbNew_Parser::ms_nVerbose > 1 ) printf("%s", _sPART_);
    if ( m_pRootPartical ) {
        m_pRootPartical->evaluate();
        if ( CPcbNew_Parser::ms_nVerbose > 1 ) m_pRootPartical->print(0);
    }
    printf("\n");
    if ( CPcbNew_Parser::ms_nVerbose > 1 ) printf("%s\n\n", _sPART_);

    if ( m_Elements.size() == 0 ) {
      printf("0 Elements after parsing - exit\n");
      exit(-2);
    }
    printf("Found %lu gcode elements\n", m_Elements.size());

    if ( CPcbNew_Parser::ms_nVerbose > 1 ) {
      printf("%s", _sELEM_);
      for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
        CElement* pElement = *it;
        printf("%s\n", pElement->print().c_str());
      }
      printf("%s\n\n", _sELEM_);
    }

    return true;
}

bool CPcbNew_Parser::Normalize() {
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

    min.normalize(m_Offset);
    n = 0;
    if ( CPcbNew_Parser::ms_nVerbose > 0 ) printf("Before: Min: %s, Max:%s\n", min.print().c_str(), max.print().c_str());
    for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
        CElement* pElement = *it;
        pElement->normalize(min);
        printProgress(++n, m_Elements.size());
    }
    printf("\n");

    if ( CPcbNew_Parser::ms_nVerbose > 1 ) {
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
    if ( CPcbNew_Parser::ms_nVerbose > 0 ) printf("After: Min: %s, Max: %s\n", m_Min.print().c_str(), m_Max.print().c_str());

    return  true;
}

bool CPcbNew_Parser::Sort() {
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

    if ( CPcbNew_Parser::ms_nVerbose > 1 ) {
      printf("%s", _sELEM_);
      for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
        CElement* pElement = *it;
        printf("%s\n", pElement->print().c_str());
      }
      printf("%s\n\n", _sELEM_);
    }

    return true;
}

bool CPcbNew_Parser::Invert() {
  printf("Invert elements\n");
  int n = 0;
  for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
      CElement* pElement = *it;
      pElement->invert(m_Max, CElement::x);
      printProgress(++n, m_Elements.size());
  }
  printf("\n");
  return true;
}
bool CPcbNew_Parser::GenerateGCode(std::string sFileName) {
    printf("Generate %s - %u cycles\n", sFileName.c_str(), CPcbNew_Parser::ms_nCutCycles);
    string sGCode;

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
    sGCode += CElement::GetGCodeFeedrate();
    sGCode += "G61\t; exact path mode\n";
    sGCode += "S" + std::to_string(nSpindle) + "\t; set spindle speed\n";
    sGCode += "M3\t; start spindle\n";
    sGCode += "G04 P3.0\t; wait for 3.0 seconds\n";

    for ( uint8_t n=0; n < CPcbNew_Parser::ms_nCutCycles; n++ )  {

      if ( CPcbNew_Parser::ms_nCutCycles > 1 ) {
        printf("Cycle %u\n", n+1);
        sGCode += "(Block-name: Cycle " + std::to_string(n+1) + ")\n";
        sGCode += "(Block-expand: 0)\n";
        sGCode += "(Block-enable: 1)\n";
        sGCode += ";\n";
      }

      CPoint  _current;
      CElement::ms_dZProcess_n = -1.0 * fabs(CElement::ms_dZProcess * (n+1));
      int k = 0;
      for ( list<CElement*>::iterator it = m_Elements.begin(); it != m_Elements.end(); it++ ) {
          CElement* pElement = *it;
          if ( pElement->m_bToExport )
            sGCode += pElement->GetGCode(&_current);
          printProgress(++k, m_Elements.size());
      }
      printf("\n");
    }

    sGCode += "(Block-name: End)\n";
    sGCode += "(Block-expand: 0)\n";
    sGCode += "(Block-enable: 1)\n";
    sGCode += "G0 M5\n";
    sGCode += "M2\t; end program\n";

    if ( CPcbNew_Parser::ms_nVerbose > 1 ) {
      printf("EXPORT : %s \n", sFileName.c_str());
      printf("%s\n", sGCode.c_str());
      printf("EXPORT : %s \n", sFileName.c_str());
    }

    std::ofstream out(sFileName);
    out << sGCode;
    out.close();

    printf("\n");

    return true;
}
