#include "test.h"

int c = 4;

int main()
{
    assert(c == 4);
    {
        int c = 9;
        assert(c == 9);
        {
            int c = 20;
            assert(c == 20);
        }
    }

    int a = 40;
    {
        int c = 50;
        a -= 20;
        assert(c == 50);
    }

    SUCCESS;
}
