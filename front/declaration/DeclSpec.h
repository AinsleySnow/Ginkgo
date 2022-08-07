#ifndef _DECLSPEC_H_
#define _DECLSPEC_H_

#include "../Node.h"
#include "../../utilities/EnumsforEntry.h"

class DeclSpec : public Node
{
private:
    /*
     * rawSpecifiers is used for identifying type of a
     * variable when user combines several type specifiers
     * together. From the lowest bit to the highest bit, the
     * 1st bit, the 2nd bit, the 3rd bit, the 4th and 5th
     * bits, the 6th bit and the 7th bit represent char, int,
     * short, long and long, float and double, respectively.
     * When the parser encounters type specifiers, it'll mask
     * corresponding bits as it retriving them, and will check
     * if any conflict has raisen.
     *
    */
    unsigned int rawSpecifiers{};

    /*
     * In rawQualifiers, we set the first, the second, the third,
     * the forth bit to indicate that the variable has been qualified
     * using const, restrict, volatile or atomic, respectively.
    */
    unsigned int rawQualifiers{};

    static constexpr unsigned int _char = 1, _int = 2, _short = 4,
        _long = 8, _longlong = 24, _float = 32, _double = 64, 
        _signed = 128, _unsigned = 256, _bool = 512, _void = 1024;

    static constexpr unsigned int _const = 1, _restrict = 2, 
        _volatile = 4, _atomic = 8;

    inline void markType(int);
    inline void markTypeLong();
    inline void markQual(int);

public:
    void MarkSpec(Tag);
    void MarkQual(Tag);
    void Join(const DeclSpec*);
    TypeSpec GetSpec();
    unsigned int GetQual();
};

#endif // _DECLSPEC_H_
