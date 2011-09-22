#include <stdio.h>

int main()
{
    rt_init();
    InitArcomage();
    rt_term();
    printf("We did not crash!");
    getchar();
    return 0;
}
