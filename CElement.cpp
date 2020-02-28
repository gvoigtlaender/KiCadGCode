/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include <CElement.h>
#include <CPartical.h>
#include <CParser.h>

namespace math
{
    const double PI(3.14159265);

    const double toDegree(const double radian)
    { return radian * 180.0 / PI; }

    const double toRadian(const double degree)
    { return degree * PI / 180.0; }
}; // math


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

std::string CElement::GetGCode(CPoint* start, CPoint* offset, bool bCreateBlock /*= true*/) const {
    string s;
    if ( bCreateBlock ) {
      s += "(Block-name: "+ m_sBlockName + ")\n";
      s += "(Block-expand: 0)\n";
      s += "(Block-enable: 1)\n";
      if ( CParser::ms_bLaserMode && CParser::ms_bTestMode ) {
        s += "S" + std::to_string(CParser::ms_nSpindleSpeed) + "\t; set spindle speed\n";
      }
    }

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
    s += "; {C" + std::to_string(m_Center.m_dX) + ":" + std::to_string(m_Center.m_dY) + ":" + std::to_string(m_dRadius) + "}";
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

    m_sBlockName = m_sName + "_ID_" + std::to_string(m_nId);
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

CPoint  CElementCircle::GetPointForAngle(double dAngleDeg) const
{
  CPoint pt;

  double dA = math::toRadian(dAngleDeg);
  pt.m_dX = m_Center.m_dX + std::cos(dA) * m_dRadius;
  pt.m_dY = m_Center.m_dY + std::sin(dA) * m_dRadius;

  // printf("deg=%.1f => x=%.1f, y=%.1f\n", dAngleDeg, pt.m_dX, pt.m_dY);

  return pt;
}

std::string CElementCircle::GetGCode(CPoint* start, CPoint* offset, bool bCreateBlock /*= true*/) const {
    string s;
    s += CElement::GetGCode(start, offset, bCreateBlock);

    CPoint p0 = m_Center+*offset;
    if ( start->GetDistance(p0) > 0.01 || bCreateBlock ) {
      printf("ID %d, %s -> %s move to start: %.3f\n", m_nId, start->print().c_str(), p0.print().c_str(), start->GetDistance(p0));
      s += GetGCodeFeedrate();
      s += " G0";
      if ( !CParser::ms_bLaserMode ) {
        s += " X" + std::to_string(m_Center.m_dX + offset->m_dX);
        s += " Y" + std::to_string(m_Center.m_dY + offset->m_dY);
        s += " Z" + std::to_string(CElement::ms_dZSafe);
      } else {
        CPoint p = GetPointForAngle(0.0);
        s += " X" + std::to_string(p.m_dX + offset->m_dX);
        s += " Y" + std::to_string(p.m_dY + offset->m_dY);
      }
      s += "\t; move to start pos\n";
    }

      if ( !CParser::ms_bLaserMode ) {
      s += GetGCodeFeedratePlunge();
      s += " G1 Z" + std::to_string(CElement::ms_dZProcess_n) + "\n";
    } else {
      double dInc = 0.1 * 360.0 / 2.0 / math::PI / m_dRadius;
      printf("rad=%.1f,  dInc=%.1f\n", m_dRadius, dInc);
      for ( double dAng = 0.0; dAng<360.0; dAng+=dInc )
      {
        s += " G1";
        CPoint p = GetPointForAngle(dAng);
        s += " X" + std::to_string(p.m_dX + offset->m_dX);
        s += " Y" + std::to_string(p.m_dY + offset->m_dY);
        s += "\t; angle:";
        s += std::to_string(dAng);
        s += "\n";
      }
      s += " G1";
      CPoint p = GetPointForAngle(0.0);
      s += " X" + std::to_string(p.m_dX + offset->m_dX);
      s += " Y" + std::to_string(p.m_dY + offset->m_dY);
      s += "\t; angle:";
      s += std::to_string(360.0);
      s += "\n";
    }


    if ( !CParser::ms_bLaserMode ) {
      s += GetGCodeFeedrate();
      s += " G0";
      s += " Z" + std::to_string(CElement::ms_dZSafe);
      s += "\n";
    }
    if ( start != NULL )
      *start = m_Center + *offset;

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
    s += "; {S" + std::to_string(m_Start.m_dX) + ":" + std::to_string(m_Start.m_dY) + "}";
    s += "; {E" + std::to_string(m_End.m_dX) + ":" + std::to_string(m_End.m_dY) + "}";
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

std::string CElementLine::GetGCode(CPoint* start, CPoint* offset, bool bCreateBlock /*= true*/) const {
    string s;
    s += CElement::GetGCode(start, offset, bCreateBlock);

    CPoint  p1, p2;

    if ( CPoint::GetDistance(m_Start+*offset, *start) < CPoint::GetDistance(m_End+*offset, *start) ) {
        p1 = m_Start+*offset;
        p2 = m_End+*offset;
    } else {
        p1 = m_End+*offset;
        p2 = m_Start+*offset;
    }

    CPoint p0 = p1;
    if ( start->GetDistance(p0) > 0.01 || bCreateBlock ) {
      printf("ID %d, %s -> %s move to start: %.3f\n", m_nId, start->print().c_str(), p0.print().c_str(), start->GetDistance(p0));
      s += GetGCodeFeedrate();
      s += " G0";
      s += " X" + std::to_string(p1.m_dX);
      s += " Y" + std::to_string(p1.m_dY);
      if ( !CParser::ms_bLaserMode )
      {
        s += " Z" + std::to_string(CElement::ms_dZSafe);
      }
      s += "\t; move to start pos\n";
    }

    if ( !CParser::ms_bLaserMode )
    {
      s += GetGCodeFeedratePlunge();
      s += " G1 Z" + std::to_string(CElement::ms_dZProcess_n) + "\n";
    }
    s += GetGCodeFeedrateProcess();
    if ( fabs(p1.m_dX - p2.m_dX) < 0.05 || fabs(p1.m_dY - p2.m_dY) < 0.05 )
    {
      s += " G1";
      s += " X" + std::to_string(p2.m_dX);
      s += " Y" + std::to_string(p2.m_dY);
      s += "\n";
    }
    else
    {
      double dM = (p2.m_dY - p1.m_dY) / (p2.m_dX - p1.m_dX);
      double dN = p1.m_dY - dM*p1.m_dX;

      printf("ID %d: diagonal move %s -> %s m=%.1f n=%.1f\n", m_nId, p1.print().c_str(), p1.print().c_str(), dM, dN);

      for ( double dX = p1.m_dX; dX<=p2.m_dX; dX += 0.05 )
      {
        double dY = dM * dX + dN;
        s += " G1";
        s += " X" + std::to_string(dX);
        s += " Y" + std::to_string(dY);
        s += "\n";
      }
    }

    if ( !CParser::ms_bLaserMode )
    {
      s += GetGCodeFeedrate();
      s += " G0";
      s += " Z" + std::to_string(CElement::ms_dZSafe);
      s += "\n";
    }

    if ( start != NULL )
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


/*static*/ bool CElementArc::ms_bToExport = false;

std::string CElementArc::print() const {
    string s = "; ID=" + std::to_string(m_nId) + "[ElementArc: " + m_sName;
    s += "; {M" + std::to_string(m_Mid.m_dX) + ":" + std::to_string(m_Mid.m_dY) + "}";
    s += "; {S" + std::to_string(m_Start.m_dX) + ":" + std::to_string(m_Start.m_dY) + "}";
    s += "; {E" + std::to_string(m_End.m_dX) + ":" + std::to_string(m_End.m_dY) + "}";
    s += "; {Radius: " + std::to_string(m_dRadius) + "}";
    s += "; {Angle: " + std::to_string(m_dAngle) + "}";
    s += "; {Ang0: " + std::to_string(m_dAng0) + "}";
    s += "]";
    return s;
}

void CElementArc::evaluate(CPartical* pPartial) {
  printf("CElementArc::evaluate .");
    string sLine = pPartial->m_Childs[0]->m_sName;
    sscanf(sLine.c_str(), "%*s %lf %lf", &m_Mid.m_dX, &m_Mid.m_dY);
    sLine = pPartial->m_Childs[1]->m_sName;
    sscanf(sLine.c_str(), "%*s %lf %lf", &m_Start.m_dX, &m_Start.m_dY);
    sLine = pPartial->m_Childs[2]->m_sName;
    double dAngle = 0.0;
    sscanf(sLine.c_str(), "%*s %lf", &dAngle);
    m_dAngle = dAngle;
    sLine = pPartial->m_Childs[3]->m_sName;
    char szLayer[255];
    sscanf(sLine.c_str(), "%*s %255s", szLayer);
    m_sLayer = szLayer;

    m_dRadius = std::sqrt((m_Mid.m_dX-m_Start.m_dX)*(m_Mid.m_dX-m_Start.m_dX) + (m_Mid.m_dY-m_Start.m_dY)*(m_Mid.m_dY-m_Start.m_dY));

    m_dAng0 = math::toDegree(std::atan((m_Mid.m_dY-m_Start.m_dY)/(m_Mid.m_dX-m_Start.m_dX)));

    printf(".. done \n");

    m_End = GetPointForAngle(m_dAng0 - m_dAngle);

    /*
    for ( double dDeg=0.0; dDeg<=360.0; dDeg+=45.0 )
      GetPointForAngle(dDeg);
    */
}

double CElementArc::GetDistance(const CPoint& current, CPoint* start) const {
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

void CElementArc::GetEndPoint(CPoint* start) const {
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

std::string CElementArc::GetGCode(CPoint* start, CPoint* offset, bool bCreateBlock /*= true*/) const {
    string s;
    s += CElement::GetGCode(start, offset, bCreateBlock);

    CPoint  p1, p2;

    double dAng0 = 180.0 - m_dAng0;
    if ( CPoint::GetDistance(m_Start, *start) < CPoint::GetDistance(m_End, *start) ) {
        p1 = m_Start;
        p2 = m_End;
    } else {
        p1 = m_End;
        p2 = m_Start;
    }

    CPoint p0 = p1+*offset;
    if ( start->GetDistance(p0) > 0.01 || bCreateBlock ) {
      printf("ID %d, %s -> %s move to start: %.3f\n", m_nId, start->print().c_str(), p0.print().c_str(), start->GetDistance(p0));
      s += GetGCodeFeedrate();
      s += " G0";
      s += " X" + std::to_string(p1.m_dX + offset->m_dX);
      s += " Y" + std::to_string(p1.m_dY + offset->m_dY);
      if ( !CParser::ms_bLaserMode )
      {
        s += " Z" + std::to_string(CElement::ms_dZSafe);
      }
      s += "\t; move to start pos\n";
    }

    if ( !CParser::ms_bLaserMode )
    {
      s += GetGCodeFeedratePlunge();
      s += " G1 Z" + std::to_string(CElement::ms_dZProcess_n) + "\n";
    }
    s += GetGCodeFeedrateProcess();
    // s += " G1";
    // s += " X" + std::to_string(p2.m_dX + offset->m_dX);
    // s += " Y" + std::to_string(p2.m_dY + offset->m_dY);
    double dInc = 0.05 * 360.0 / 2.0 / math::PI / m_dRadius;
    // for ( double dAng = 0.0; dAng<-m_dAngle; dAng+=1 )
    for ( double dAng = 0.0; dAng<-m_dAngle; dAng+=dInc )
    {
      string sA = " G1";
      CPoint p = GetPointForAngle(dAng0 + dAng);
      sA += " X" + std::to_string(p.m_dX + offset->m_dX);
      sA += " Y" + std::to_string(p.m_dY + offset->m_dY);
      s += sA;
      s += "\t; angle:";
      s += std::to_string(dAng0 + dAng);
      s += "\n";
    }

    if ( !CParser::ms_bLaserMode )
    {
      s += GetGCodeFeedrate();
      s += " G0";
      s += " Z" + std::to_string(CElement::ms_dZSafe);
      s += "\n";
    }

    if ( start != nullptr )
      *start = p2 + *offset;

    return s;
}

void  CElementArc::minmax(CPoint* min, CPoint* max) const {
    m_Start.minmax(min, max);
    m_End.minmax(min, max);
    m_Mid.minmax(min, max);
}

void CElementArc::normalize(const CPoint& start) {
    m_Start.normalize(start);
    m_End.normalize(start);
    m_Mid.normalize(start);
}

void CElementArc::invert(const CPoint& inv, _E_InvMode eIM) {
  if ( eIM == x ) {
    m_Start.m_dX = inv.m_dX - m_Start.m_dX;
    m_End.m_dX = inv.m_dX - m_End.m_dX;
    m_Mid.m_dX = inv.m_dX - m_Mid.m_dX;
  } else {
    m_Start.m_dY = inv.m_dY - m_Start.m_dY;
    m_End.m_dY = inv.m_dY - m_End.m_dY;
    m_Mid.m_dY = inv.m_dY - m_Mid.m_dY;
  }
}

CPoint  CElementArc::GetPointForAngle(double dAngleDeg) const
{
  CPoint pt;

  double dA = math::toRadian(dAngleDeg);
  pt.m_dX = m_Mid.m_dX + std::cos(dA) * m_dRadius;
  pt.m_dY = m_Mid.m_dY + std::sin(dA) * m_dRadius;

  // printf("deg=%.1f => x=%.1f, y=%.1f\n", dAngleDeg, pt.m_dX, pt.m_dY);

  return pt;
}

/*static*/ bool CElementText::ms_bToExport = false;

std::string CElementText::print() const {
    string s = "; ID=" + std::to_string(m_nId) + "[ElementText: " + m_sName;
    s += "; {C" + std::to_string(m_Center.m_dX) + ":" + std::to_string(m_Center.m_dY) + "}";
    s += "; {Text=" + m_sText + "}";
    s += "]";
    return s;
}

void CElementText::evaluate(CPartical* pPartial) {
    string sLine = pPartial->m_Childs[0]->m_sName;
    sscanf(sLine.c_str(), "%*s %lf %lf", &m_Center.m_dX, &m_Center.m_dY);
    sLine = pPartial->m_Childs[1]->m_sName;
    char szLayer[255];
    sscanf(sLine.c_str(), "%*s %255s", szLayer);
    m_sLayer = szLayer;
}

double CElementText::GetDistance(const CPoint& current, CPoint* start) const {
    if ( start != nullptr )
      *start = m_Center;
    return CPoint::GetDistance(current, m_Center);
}

void CElementText::GetEndPoint(CPoint* start) const {
  if ( start != nullptr )
    *start = m_Center;
}

std::string CElementText::GetGCode(CPoint* start, CPoint* offset, bool bCreateBlock /*= true*/) const {
    string s;
    s += CElement::GetGCode(start, offset, bCreateBlock);

    CPoint p0 = m_Center+*offset;
    if ( start->GetDistance(p0) > 0.01 || bCreateBlock ) {
      printf("ID %d, %s -> %s move to start: %.3f\n", m_nId, start->print().c_str(), p0.print().c_str(), start->GetDistance(p0));
      s += GetGCodeFeedrate();
      s += " G0";
      s += " X" + std::to_string(m_Center.m_dX + offset->m_dX);
      s += " Y" + std::to_string(m_Center.m_dY + offset->m_dY);
      s += " Z" + std::to_string(CElement::ms_dZSafe);
      s += "\t; move to start pos\n";
    }

    if ( !CParser::ms_bLaserMode ) {
      s += GetGCodeFeedratePlunge();
      s += " G1 Z" + std::to_string(CElement::ms_dZProcess_n) + "\n";
    }

    s += GetGCodeFeedrate();
    s += " G0";
    s += " Z" + std::to_string(CElement::ms_dZSafe);
    s += "\n";

    if ( start != NULL )
      *start = m_Center + *offset;

    return s;
}

void  CElementText::minmax(CPoint* min, CPoint* max) const {
    m_Center.minmax(min, max);
}

void CElementText::normalize(const CPoint& start) {
    m_Center.normalize(start);
}

void CElementText::invert(const CPoint& inv, _E_InvMode eIM) {
  if ( eIM == x ) {
    m_Center.m_dX = inv.m_dX - m_Center.m_dX;
  } else {
    m_Center.m_dY = inv.m_dY - m_Center.m_dY;
  }
}
