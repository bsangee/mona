#include "GCNode.h"

extern GCNode top;

void test()
{
    char* p = new char[32];
    char* q = new char;
}

void test2()
{
    short* r = new short;
}

int dummy(int i)
{
    if (i > 100) return 100;
    return dummy(i + 1);
}

int main(int argc, char *argv[])
{
    gc_init();

    int x = 0x12345678;
    int* q = new int;
    test();
    test2();
    int y = 0x99998888;
    dummy(0);
    gc();
    int size = gc_node_size(&top);

    if (size == 1 && top.next->size == 4)
    {
        printf("[OK] %s\n", argv[0]);
        return 0;
    }
    else
    {
        printf("%s [NG]\n", argv[0]);
        FOREACH_GC_NODE(&top, e)
        {
            printf("    not sweeped size = %d\n", e->size);
        }
        return 1;
    }

}
