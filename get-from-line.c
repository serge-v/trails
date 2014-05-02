#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    uint64_t offs = 0, lines = 0;
    int rc = 1;
    FILE* fidx = NULL;
    FILE* fin = NULL;
    char s[1024];

    if (argc < 3)
    {
        fprintf(stderr, "usage: get-from-line FILENAME LINE_START LINE_END\n");
        goto out;
    }

    fidx = fopen("out.idx", "rb");
    if (fidx == NULL)
    {
        perror("cannot open index file");
        goto out;
    }
    
    fin = fopen(argv[1], "rt");
    if (fin == NULL)
    {
        perror("cannot open text file");
        goto out;
    }

    uint64_t lines_start = strtoul(argv[2], NULL, 10);
    uint64_t lines_end = strtoul(argv[3], NULL, 10);
    
    if (lines_end < lines_start)
    {
        fprintf(stderr, "LINE_END is less than LINE_START");
        goto out;
    }
    
    lines = lines_end - lines_start + 1;

    uint64_t pos1 = sizeof(offs) * lines_start;
    if (fseek(fidx, pos1, SEEK_SET) != 0)
    {
        perror("fseek index");
        goto out;
    }
    
    fread(&offs, sizeof(offs), 1, fidx);
    
    if (fseek(fin, offs, SEEK_SET) != 0)
    {
        perror("fseek input");
        goto out;
    }

    while (lines--)
    {
        char* p = fgets(s, 1024, fin);
        if (feof(fin))
            break;
        printf("%s", p);
    }

out:

    if (fin != NULL)
        fclose(fin);

    if (fidx != NULL)
        fclose(fidx);
    
    return 0;
}
