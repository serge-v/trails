#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    char s[1024];
    uint64_t offs = 0;
    int rc = 1;
    FILE* fin = NULL;
    FILE* fout = NULL;

    if (argc < 2)
    {
        fprintf(stderr, "input file is not specified\n");
        goto out;
    }

    fin = fopen(argv[1], "rt");
    if (fin == NULL)
    {
        perror("cannot open input file");
        goto out;
    }

    fout = fopen("out.idx", "wb");
    if (fout == NULL)
    {
        perror("cannot open output file");
        goto out;
    }

    while (1)
    {
        char* p = fgets(s, 1024, fin);
        if (feof(fin))
            break;

        fwrite(&offs, sizeof(offs), 1, fout);
        offs = ftell(fin);
    }

    rc = 0;

out:

    if (fin != NULL)
        fclose(fin);

    if (fout != NULL)
        fclose(fout);
    
    return 0;
}
