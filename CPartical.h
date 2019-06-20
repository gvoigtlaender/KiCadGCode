/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef CPARTICAL_H_
#define CPARTICAL_H_

#include <string>
#include <vector>

class CElement;

class CPartical {
 public:
    CPartical(CPartical* pParent, bool bToSkip)
    : m_pParent(pParent)
    , m_sName("")
    , m_pElement(nullptr)
    , m_ulId(CPartical::ms_ulNoOfObjects)
    , m_bToSkip(bToSkip) {
        if ( pParent )
            pParent->m_Childs.push_back(this);
        CPartical::ms_ulNoOfObjects++;
    }
    virtual ~CPartical() {
        for ( uint8_t n=0; n < m_Childs.size(); n++ )
            delete m_Childs[n];
        m_Childs.clear();
        CPartical::ms_ulNoOfObjects--;
    }

    virtual void print(uint8_t nIdx = 0);
    virtual void evaluate();

    CPartical*  m_pParent;

    std::string m_sName;
    std::vector<std::string> m_sData;

    std::vector<CPartical*> m_Childs;

    CElement*   m_pElement;
    uint64_t  m_ulId;
    bool      m_bToSkip;
    static uint64_t ms_ulNoOfObjects;

    void show_progress();
    static void reset_progress();
    static uint64_t ms_ulProgressMax;
    static uint64_t ms_ulProgress;
};

#endif  // CPARTICAL_H_
