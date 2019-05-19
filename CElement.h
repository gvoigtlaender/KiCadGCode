/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef CELEMENT_H_
#define CELEMENT_H_

#include <string>
using std::string;

#include <CPoint.hpp>

class CPartical;
class CElement {
 public:
    enum _E_InvMode { x, y };
    explicit CElement(string sName)
        : m_sName(sName)
        , m_nId(++ms_nId)
        , m_sLayer("") {
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
    virtual std::string GetGCode(CPoint* /*start*/) const;
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
    static int    ms_nFeedRate;
    static int    ms_nFeedRateProcess;
    static int    ms_nFeedRatePlunge;

    string m_sLayer;
};

class CElementCircle : public CElement {
 public:
    CElementCircle(string sName, CPartical* pPartial)
        : CElement(sName) {
        evaluate(pPartial);
    }
    std::string print() const override;
    void evaluate(CPartical* pPartial) override;
    double GetDistance(const CPoint& current, CPoint* start) const override;
    void GetEndPoint(CPoint* start) const override;
    std::string GetGCode(CPoint* start) const override;
    void  minmax(CPoint* min, CPoint* max) const override;
    void normalize(const CPoint& start) override;
    void invert(const CPoint& inv, _E_InvMode eIM) override;
    CPoint  m_Center;
};
class CElementLine : public CElement {
 public:
    CElementLine(string sName, CPartical* pPartial)
        : CElement(sName) {
        evaluate(pPartial);
    }
    std::string print() const override;
    void evaluate(CPartical* pPartial) override;
    double GetDistance(const CPoint& current, CPoint* start) const override;
    void GetEndPoint(CPoint* start) const override;
    std::string GetGCode(CPoint* start) const override;
    void  minmax(CPoint* min, CPoint* max) const override;
    void normalize(const CPoint& start) override;
    void invert(const CPoint& inv, _E_InvMode eIM) override;

    CPoint  m_Start;
    CPoint  m_End;
};

#endif  //  CELEMENT_H_
