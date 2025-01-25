/* NAppGUI Console Application */

#include <core/coreall.h>

int main(int argc, char *argv[])
{
    unref(argc);
    unref(argv);
    core_start();
    bstd_printf("Hello world!\n");
    core_finish();
    return 0;
}

