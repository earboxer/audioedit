#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *in = NULL;
char *out = NULL;
int bflag = 0;
int eflag = 0;
int c;

void parse(int argc, char **argv)
{
    while ((c = getopt (argc, argv, "i:o:b:e:")) != -1){
        switch (c) {
            case 'i':
                in = optarg;
                break;
            case 'o':
                out = optarg;
                break;
            case 'b':
                bflag = 1;
            case 'e':
                eflag = 1;
            case '?':
                puts("unkown");
            default:
                abort();
        }
    }
}