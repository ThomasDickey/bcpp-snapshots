#include "foobar.h"
#define FOO this\
		& that
#define BAR "this\
		& that"

static	char	what[] = "@(#)filename	";

main()
{
    float x = 0.0;
    float x0 = 0e0;
    float x1 = .0;
    float x2 = .0e1;
    float x3 = .0e+1;
    float x4 = .0e-1;
    float x5 = 1.23456789e+123L;
    float x6 = 1.23456789e-123;
    int y = 1;
    int y1 = 1L;
    int y2 = 1ul;
    int y3 = 12345678u;
    int z = 0x1;
    int z1 = 0X1L;
    int z2 = 0X1ul;
    int z3 = 0X12345678u;
    char *a = "aaa""bbb";

printf("test with a tab:	:embedded (not considered legal by C_COUNT)\n");
printf("test with embedded pound\
        # like this\n");
}
