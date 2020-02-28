/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include <main.h>
#include <iostream>
#include <string>
#include "CParser_PcbNew.h"
#include "CParser_Svg.h"

#include "tclap/CmdLine.h"
using std::vector;

void usage() {
  printf("Aborting: no input file provided\n");
  printf("Usage:\n");
  printf("\t./KiCadGCode <filename>\n");
}

int main(int argc, char** argv) {

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

    TCLAP::ValueArg<int> ncycles("c", "cycles", "cutting cycles", false, CParser::ms_nCutCycles, "int");
    cmd.add(ncycles);

    TCLAP::ValueArg<int> matrixX("", "matrixX", "Number of Matrix cuts X", false, CParser::ms_nMatrixX, "int");
    cmd.add(matrixX);
    TCLAP::ValueArg<int> matrixY("", "matrixY", "Number of Matrix cuts Y", false, CParser::ms_nMatrixY, "int");
    cmd.add(matrixY);
    TCLAP::ValueArg<double> matrixOffset("", "matrixOffset", "Offset between Matrix cuts [mm]", false, CParser::ms_dMatrixOffset, "double");
    cmd.add(matrixOffset);

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

    TCLAP::ValueArg<int> spindlespeed("", "spindlespeed", "Spindle rotation speed [u/min]", false, CParser::ms_nSpindleSpeed, "int");
    cmd.add(spindlespeed);

    vector<string> allowed_shapes;
    allowed_shapes.push_back("line");
    allowed_shapes.push_back("circle");
    allowed_shapes.push_back("arc");
    TCLAP::ValuesConstraint<string> allowedShapes(allowed_shapes);
    TCLAP::MultiArg<string> shapes("", "shapes", "Shapes to include in gcode", false, &allowedShapes);
    cmd.add(shapes);

    TCLAP::ValueArg<string> prefix("", "prefix", "File name prefix to export", false, "", "string");
    cmd.add(prefix);

    TCLAP::ValueArg<bool> LaserMode("L", "LaserMode", "Enable LaserMode. Uses M4 for dynamic laser power", false, false, "boolean");
    cmd.add(LaserMode);

    TCLAP::ValueArg<bool> AppendFile("a", "append", "Append to gcode file if exist", false, false, "boolean");
    cmd.add(AppendFile);

    TCLAP::ValueArg<bool> TestMode("t", "testmode", "Generate test matrix", false, false, "boolean");
    cmd.add(TestMode);

    cmd.parse(argc, argv);

    CParser::ms_sFileName = filename.getValue();
    CParser::ms_nVerbose = verbose.getValue();

    CElement::ms_dZSafe = zsafe.getValue();
    CElement::ms_dZProcess = zprocess.getValue();

    vector<string> vlayers = layers.getValue();
    if ( vlayers.size() > 0 ) {
      CParser::ms_Layers = vlayers;
    } else {
      CParser::ms_Layers = allowed_layers;
    }

    vector<string> vshapes = shapes.getValue();
    if ( vshapes.size() == 0 ) {
      CElementCircle::ms_bToExport = true;
      CElementLine::ms_bToExport = true;
      CElementArc::ms_bToExport = true;
    } else {
      for ( unsigned int j=0; j < vshapes.size(); j++ ) {
        string sshape = vshapes[j];
        if ( sshape == "circle" )
          CElementCircle::ms_bToExport = true;
        else if ( sshape == "line" )
          CElementLine::ms_bToExport = true;
        else if ( sshape == "arc" )
          CElementArc::ms_bToExport = true;
      }
    }

    string sSide = side.getValue();
    if ( sSide == "front" ) CParser::ms_bCreateBack = false;
    if ( sSide == "back" )  CParser::ms_bCreateFront =  false;

    CParser::ms_Offset.m_dX = x0.getValue();
    CParser::ms_Offset.m_dY = y0.getValue();

    CElement::ms_nFeedRate = feedrate.getValue();
    CElement::ms_nFeedRatePlunge = plungefeedrate.getValue();
    CElement::ms_nFeedRateProcess = processfeedrate.getValue();
    CParser::ms_nSpindleSpeed = spindlespeed.getValue();

    CParser::ms_sExportPrefix = prefix.getValue();

    CParser::ms_nCutCycles = ncycles.getValue();

    CParser::ms_nMatrixX = matrixX.getValue();
    CParser::ms_nMatrixY = matrixY.getValue();
    CParser::ms_dMatrixOffset = matrixOffset.getValue();

    CParser::ms_bLaserMode = LaserMode.getValue();
    CParser::ms_bAppendFile = AppendFile.getValue();
    CParser::ms_bTestMode = TestMode.getValue();

  } catch (TCLAP::ArgException &e) {
    // catch any exceptions
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(-1);
  }

  if ( CParser::ms_nVerbose > 0 ) {
    printf("using arguments: f=%s, v=%d, s=%.3f, p=%.3f, Layers:{", CParser::ms_sFileName.c_str(), CParser::ms_nVerbose, CElement::ms_dZSafe, CElement::ms_dZProcess);
    for ( unsigned int n=0; n < CParser::ms_Layers.size(); n++ )
      printf("[%s]", CParser::ms_Layers[n].c_str());
    printf("}");
    printf(", f=%s, b=%s", std::to_string(CParser::ms_bCreateFront).c_str(), std::to_string(CParser::ms_bCreateBack).c_str());
    printf(", L=%s\n", CParser::ms_bLaserMode?"true":"false");
  }


    CParser* pParser = NULL;

    string sFile = CParser::ms_sFileName;
    std::transform(sFile.begin(), sFile.end(), sFile.begin(),
    [](unsigned char c){ return std::tolower(c); });
    if ( sFile.find(".kicad_pcb") != std::string::npos )
    {
      printf("CParser_PcbNew\n");
      pParser = new CParser_PcbNew(argc, argv);
    } else if ( sFile.find(".svg") != std::string::npos) {
        printf("CParser_Svg\n");
        pParser = new CParser_Svg(argc, argv);
    } else {
      printf(" filename should end \".kicad_pcb\" or \".svg\"\n");
      exit(-1);
    }

    pParser->Parse();

    printf("\nCreating Front side\n");
    pParser->Normalize();
    pParser->Sort();
    if ( CParser::ms_bCreateFront ) {
      pParser->GenerateGCode(CParser::ms_sExportPrefix + CParser::ms_sFileName + "_f.ngc");
    }
    if ( CParser::ms_bCreateBack ) {
      printf("\nCreating Back side\n");
      pParser->Invert(CElement::x);
      pParser->Normalize();
      pParser->Sort();
      pParser->GenerateGCode(CParser::ms_sExportPrefix + CParser::ms_sFileName + "_b.ngc");
    }
    printf("\n");
    return 0;
}
