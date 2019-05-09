/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef CPARTICAL_H_
#define CPARTICAL_H_

#include <string>
#include <vector>

class CElement;

class CPartical {
 public:
    explicit CPartical(CPartical* pParent)
    : m_pParent(pParent)
    , m_sName("")
    , m_pElement(nullptr) {
        if ( pParent )
            pParent->m_Childs.push_back(this);
    }
    virtual ~CPartical() {
        for ( uint8_t n=0; n < m_Childs.size(); n++ )
            delete m_Childs[n];
        m_Childs.clear();
    }

    virtual void print(uint8_t nIdx = 0);
    virtual void evaluate();

    CPartical*  m_pParent;

    std::string m_sName;
    std::vector<std::string> m_sData;

    std::vector<CPartical*> m_Childs;

    CElement*   m_pElement;
};

#endif  // CPARTICAL_H_
