/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef CELEMENT_H_
#define CELEMENT_H_

#include <string>
using std::string;

#include <CPoint.hpp>

class CPartical;
class CElement {
 public:
    enum _E_InvMode { x=0, y };
    CElement(string sName, bool bToExport)
        : m_sName(sName)
        , m_nId(++ms_nId)
        , m_sLayer("")
        , m_bToExport(bToExport)
        , m_BlendingPrev(NULL)
        , m_BlendingNext(NULL)
        , m_sBlockName(sName) {
    }
    virtual ~CElement() {
    }
    string m_sName;
    virtual std::string print() const;
    virtual void evaluate(CPartical* pPartial) = 0;
    virtual double GetDistance(const CPoint& current, CPoint* start) const = 0;
    double GetDistance(const CPoint& current) const {
        CPoint tmp;
        return this->GetDistance(current, &tmp);
    }
    virtual void GetEndPoint(CPoint* start) const = 0;
    virtual std::string GetGCode(CPoint* start, CPoint* offset, bool bCreateBlock = true) const;
    static std::string GetGCodeFeedratePlunge();
    static std::string GetGCodeFeedrateProcess();
    static std::string GetGCodeFeedrate();
    virtual void  minmax(CPoint* min, CPoint* max) const = 0;
    virtual void normalize(const CPoint& start) = 0;
    virtual void invert(const CPoint& inv, _E_InvMode eIM) = 0;
    unsigned int m_nId;
    static unsigned int ms_nId;

    static double ms_dZSafe;
    static double ms_dZProcess;
    static double ms_dZProcess_n;
    static int    ms_nFeedRate;
    static int    ms_nFeedRateProcess;
    static int    ms_nFeedRatePlunge;

    string m_sLayer;
    bool m_bToExport;

    CElement* m_BlendingPrev;
    CElement* m_BlendingNext;

    string m_sBlockName;
};

class CElementCircle : public CElement {
 public:
   CElementCircle(string sName, CPartical* pPartial)
       : CElement(sName, CElementCircle::ms_bToExport)
       , m_Center()
       , m_dRadius(1.0) {
       evaluate(pPartial);
   }
   CElementCircle(string sName)
       : CElement(sName, CElementCircle::ms_bToExport)
       , m_Center()
       , m_dRadius(1.0) {
   }
    std::string print() const override;
    void evaluate(CPartical* pPartial) override;
    double GetDistance(const CPoint& current, CPoint* start) const override;
    void GetEndPoint(CPoint* start) const override;
    CPoint GetPointForAngle(double dAngleDeg) const;
    std::string GetGCode(CPoint* start, CPoint* offset, bool bCreateBlock = true) const override;
    void  minmax(CPoint* min, CPoint* max) const override;
    void normalize(const CPoint& start) override;
    void invert(const CPoint& inv, _E_InvMode eIM) override;
    CPoint  m_Center;
    double m_dRadius;
    static bool ms_bToExport;
};

class CElementLine : public CElement {
 public:
   CElementLine(string sName, CPartical* pPartial)
       : CElement(sName, CElementLine::ms_bToExport)
       , m_Start()
       , m_End()
       , m_dM(1.0)
       , m_dN(0.0) {
         evaluate(pPartial);
   }
   CElementLine(string sName)
       : CElement(sName, CElementLine::ms_bToExport)
       , m_Start()
       , m_End()
       , m_dM(1.0)
       , m_dN(0.0) {
   }
    std::string print() const override;
    void evaluate(CPartical* pPartial) override;
    double GetDistance(const CPoint& current, CPoint* start) const override;
    void GetEndPoint(CPoint* start) const override;
    std::string GetGCode(CPoint* start, CPoint* offset, bool bCreateBlock = true) const override;
    void  minmax(CPoint* min, CPoint* max) const override;
    void normalize(const CPoint& start) override;
    void invert(const CPoint& inv, _E_InvMode eIM) override;

    CPoint  m_Start;
    CPoint  m_End;
    double  m_dM;
    double  m_dN;
    static bool ms_bToExport;
};

class CElementArc : public CElement {
 public:
   CElementArc(string sName, CPartical* pPartial)
    : CElement(sName, CElementArc::ms_bToExport)
    , m_Start()
    , m_End()
    , m_Mid()
    , m_dRadius(1.0)
    , m_dAngle(90.0)
    , m_dAng0(0.0) {
      evaluate(pPartial);
    }
    std::string print() const override;
    void evaluate(CPartical* pPartial) override;
    double GetDistance(const CPoint& current, CPoint* start) const override;
    void GetEndPoint(CPoint* start) const override;
    std::string GetGCode(CPoint* start, CPoint* offset, bool bCreateBlock = true) const override;
    void  minmax(CPoint* min, CPoint* max) const override;
    void normalize(const CPoint& start) override;
    void invert(const CPoint& inv, _E_InvMode eIM) override;

    CPoint  GetPointForAngle(double dAngleDeg) const;

    CPoint  m_Start;
    CPoint  m_End;
    CPoint  m_Mid;
    double  m_dRadius;
    double  m_dAngle;
    double  m_dAng0;

    static bool ms_bToExport;
};

class CElementText : public CElement {
 public:
    CElementText(string sName, CPartical* pPartial)
        : CElement(sName, CElementText::ms_bToExport)
        , m_Center()
        , m_sText("") {
        evaluate(pPartial);
    }
    std::string print() const override;
    void evaluate(CPartical* pPartial) override;
    double GetDistance(const CPoint& current, CPoint* start) const override;
    void GetEndPoint(CPoint* start) const override;
    std::string GetGCode(CPoint* start, CPoint* offset, bool bCreateBlock = true) const override;
    void  minmax(CPoint* min, CPoint* max) const override;
    void normalize(const CPoint& start) override;
    void invert(const CPoint& inv, _E_InvMode eIM) override;
    CPoint  m_Center;
    string m_sText;
    static bool ms_bToExport;
};

#endif  //  CELEMENT_H_
