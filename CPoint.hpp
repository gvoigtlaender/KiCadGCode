/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef CPOINT_HPP_
#define CPOINT_HPP_

#include <cmath>
#include <string>
#include <algorithm>

class CPoint {
 public:
    CPoint()
        : m_dX(0.0)
        , m_dY(0.0) {
    }
    CPoint(double dX, double dY)
        : m_dX(dX)
        , m_dY(dY) {
    }
    CPoint& operator=(const CPoint& other) {
        m_dX = other.m_dX;
        m_dY = other.m_dY;
        return *this;
    }
    bool operator==(const CPoint& other) {
        if ( fabs(m_dX - other.m_dX) > 0.01 )
            return  false;
        if ( fabs(m_dY - other.m_dY) > 0.01 )
            return  false;
        return  true;
    }

    const CPoint operator + (CPoint &obj) const {
      CPoint res;
      res.m_dX = m_dX + obj.m_dX;
      res.m_dY = m_dY + obj.m_dY;
      return res;
    }
    /*
    friend CPoint operator + (CPoint const &, CPoint const &);
    */


    std::string print() const {
        return std::to_string(m_dX) + ":" + std::to_string(m_dY);
    }

    void normalize(const CPoint& other) {
        m_dX -= other.m_dX;
        m_dY -= other.m_dY;
    }

    void minmax(CPoint* min, CPoint* max) const {
        min->m_dX = std::min<double>(m_dX, min->m_dX);
        min->m_dY = std::min<double>(m_dY, min->m_dY);
        max->m_dX = std::max<double>(m_dX, max->m_dX);
        max->m_dY = std::max<double>(m_dY, max->m_dY);
    }

    static double GetDistance(const CPoint& P1, const CPoint& P2) {
        return std::sqrt(std::pow(P1.m_dX - P2.m_dX, 2) + std::pow(P1.m_dY - P2.m_dY, 2));
    }

    double GetDistance(const CPoint& P1)
    {
      return CPoint::GetDistance(*this, P1);
    }
    double m_dX;
    double m_dY;
};

/*
CPoint operator + (CPoint const &c1, CPoint const &c2)
{
     return CPoint(c1.m_dX + c2.m_dX, c1.m_dY + c2.m_dY);
}
*/

#endif  // CPOINT_HPP_
