#ifndef _LABEL_H_
#define _LABEL_H_

#include "Identifier.h"
#include "visitors/Visitor.h"


class Label : public Identifier
{
public:
    Label(const std::string& n) : Identifier(n) {}

    void Accept(Visitor* v) override { v->VisitLabel(this); }
};

#endif // _LABEL_H_
