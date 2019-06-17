/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include <CElement.h>
#include <CPartical.h>
#include <CPcbNew_Parser.h>

/*static*/ unsigned int CElement::ms_nId = 0;
/*static*/ double CElement::ms_dZSafe = 1.0;
/*static*/ double CElement::ms_dZProcess = -0.1;
/*static*/ double CElement::ms_dZProcess_n = 0;
/*static*/ int    CElement::ms_nFeedRate = 300;
/*static*/ int    CElement::ms_nFeedRateProcess = 200;
/*static*/ int    CElement::ms_nFeedRatePlunge = 100;

std::string CElement::print() const {
    return "[Element: " + m_sName + "]";
}

std::string CElement::GetGCode(CPoint* /*start*/) const {
    string s;
    s += "(Block-name: "+ this->m_sName + "_ID_" + std::to_string(m_nId) + ")\n";
    s += "(Block-expand: 0)\n";
    s += "(Block-enable: 1)\n";

    return s;
}

/*static*/ std::string CElement::GetGCodeFeedratePlunge() {
  string s;
  s += "F" + std::to_string(ms_nFeedRatePlunge) + ";\t set plunge feed rate \n";
  return s;
}

/*static*/ std::string CElement::GetGCodeFeedrateProcess() {
  string s;
  s += "F" + std::to_string(ms_nFeedRateProcess) + ";\t set process feed rate \n";
  return s;
}

/*static*/ std::string CElement::GetGCodeFeedrate() {
  string s;
  s += "F" + std::to_string(ms_nFeedRate) + ";\t set feed rate \n";
  return s;
}

/*static*/ bool CElementCircle::ms_bToExport = false;

std::string CElementCircle::print() const {
    string s = "; ID=" + std::to_string(m_nId) + "[ElementCircle: " + m_sName;
    s += "; {" + std::to_string(m_Center.m_dX) + ":" + std::to_string(m_Center.m_dY) + "}";
    s += "]";
    return s;
}

void CElementCircle::evaluate(CPartical* pPartial) {
    string sLine = pPartial->m_Childs[0]->m_sName;
    sscanf(sLine.c_str(), "%*s %lf %lf", &m_Center.m_dX, &m_Center.m_dY);
    sLine = pPartial->m_Childs[2]->m_sName;
    char szLayer[255];
    sscanf(sLine.c_str(), "%*s %255s", szLayer);
    m_sLayer = szLayer;
}

double CElementCircle::GetDistance(const CPoint& current, CPoint* start) const {
    if ( start != nullptr )
      *start = m_Center;
    return CPoint::GetDistance(current, m_Center);
}

void CElementCircle::GetEndPoint(CPoint* start) const {
  if ( start != nullptr )
    *start = m_Center;
}

std::string CElementCircle::GetGCode(CPoint* start) const {
    string s;
    s += CElement::GetGCode(start);

    s += GetGCodeFeedrate();
    s += " G0";
    s += " X" + std::to_string(m_Center.m_dX);
    s += " Y" + std::to_string(m_Center.m_dY);
    s += " Z" + std::to_string(CElement::ms_dZSafe);
    s += "\t; move to start pos\n";

    s += GetGCodeFeedratePlunge();
    s += " G1 Z" + std::to_string(CElement::ms_dZProcess_n) + "\n";

    s += GetGCodeFeedrate();
    s += " G0";
    s += " Z" + std::to_string(CElement::ms_dZSafe);
    s += "\n";

    return s;
}

void  CElementCircle::minmax(CPoint* min, CPoint* max) const {
    m_Center.minmax(min, max);
}

void CElementCircle::normalize(const CPoint& start) {
    m_Center.normalize(start);
}

void CElementCircle::invert(const CPoint& inv, _E_InvMode eIM) {
  if ( eIM == x ) {
    m_Center.m_dX = inv.m_dX - m_Center.m_dX;
  } else {
    m_Center.m_dY = inv.m_dY - m_Center.m_dY;
  }
}

/*static*/ bool CElementLine::ms_bToExport = false;

std::string CElementLine::print() const {
    string s = "; ID=" + std::to_string(m_nId) + "[ElementLine: " + m_sName;
    s += "; {" + std::to_string(m_Start.m_dX) + ":" + std::to_string(m_Start.m_dY) + "}";
    s += "; {" + std::to_string(m_End.m_dX) + ":" + std::to_string(m_End.m_dY) + "}";
    s += "]";
    return s;
}

void CElementLine::evaluate(CPartical* pPartial) {
    string sLine = pPartial->m_Childs[0]->m_sName;
    sscanf(sLine.c_str(), "%*s %lf %lf", &m_Start.m_dX, &m_Start.m_dY);
    sLine = pPartial->m_Childs[1]->m_sName;
    sscanf(sLine.c_str(), "%*s %lf %lf", &m_End.m_dX, &m_End.m_dY);
    sLine = pPartial->m_Childs[2]->m_sName;
    char szLayer[255];
    sscanf(sLine.c_str(), "%*s %255s", szLayer);
    m_sLayer = szLayer;
}

double CElementLine::GetDistance(const CPoint& current, CPoint* start) const {
    double dDiffStart = CPoint::GetDistance(current, m_Start);
    double dDiffEnd = CPoint::GetDistance(current, m_End);

    if ( dDiffEnd < dDiffStart ) {
        if ( start != nullptr )
          *start = m_End;
        return dDiffEnd;
    } else {
        if ( start != nullptr )
          *start = m_Start;
        return dDiffStart;
    }
}

void CElementLine::GetEndPoint(CPoint* start) const {
    CPoint  p1, p2;

    if ( CPoint::GetDistance(m_Start, *start) < CPoint::GetDistance(m_End, *start) ) {
        p1 = m_Start;
        p2 = m_End;
    } else {
        p1 = m_End;
        p2 = m_Start;
    }
    if ( start != nullptr )
      *start = p2;
}

std::string CElementLine::GetGCode(CPoint* start) const {
    string s;
    s += CElement::GetGCode(start);

    CPoint  p1, p2;

    if ( CPoint::GetDistance(m_Start, *start) < CPoint::GetDistance(m_End, *start) ) {
        p1 = m_Start;
        p2 = m_End;
    } else {
        p1 = m_End;
        p2 = m_Start;
    }

    s += GetGCodeFeedrate();
    s += " G0";
    s += " X" + std::to_string(p1.m_dX);
    s += " Y" + std::to_string(p1.m_dY);
    s += " Z" + std::to_string(CElement::ms_dZSafe);
    s += "\t; move to start pos\n";

    s += GetGCodeFeedratePlunge();
    s += " G1 Z" + std::to_string(CElement::ms_dZProcess_n) + "\n";
    s += GetGCodeFeedrateProcess();
    s += " G1";
    s += " X" + std::to_string(p2.m_dX);
    s += " Y" + std::to_string(p2.m_dY);
    s += "\n";

    s += GetGCodeFeedrate();
    s += " G0";
    s += " Z" + std::to_string(CElement::ms_dZSafe);
    s += "\n";

    if ( start != nullptr )
      *start = p2;

    return s;
}

void  CElementLine::minmax(CPoint* min, CPoint* max) const {
    m_Start.minmax(min, max);
    m_End.minmax(min, max);
}

void CElementLine::normalize(const CPoint& start) {
    m_Start.normalize(start);
    m_End.normalize(start);
}

void CElementLine::invert(const CPoint& inv, _E_InvMode eIM) {
  if ( eIM == x ) {
    m_Start.m_dX = inv.m_dX - m_Start.m_dX;
    m_End.m_dX = inv.m_dX - m_End.m_dX;
  } else {
    m_Start.m_dY = inv.m_dY - m_Start.m_dY;
    m_End.m_dY = inv.m_dY - m_End.m_dY;
  }
}
