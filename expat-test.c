#include <stdio.h>
#include <stdint.h>
#include <expat.h>


#define XML_FMT_INT_MOD "l"

#define BUFFSIZE        8192

char Buff[BUFFSIZE];

int Depth;

static void XMLCALL
start(void *data, const char *el, const char **attr)
{
    if (strcmp("node", el) != 0 && strcmp("way", el) != 0)
    {
        return;
    }

    XML_Parser p = (XML_Parser)data;

    int i, j;
    XML_Size lineno = XML_GetCurrentLineNumber(p);
    XML_Index idx = XML_GetCurrentByteIndex(p);

    uint64_t id = 0;

    for (i = 0; attr[i]; i += 2)
    {
        if (strcmp(attr[i], "id") == 0)
            id = strtoul(attr[i+1], NULL, 10);
    }

    printf("%s %llu %lu\n", el, id, idx);
    Depth++;
}

static void XMLCALL
end(void *data, const char *el)
{
    int i;
    Depth--;
  
    XML_Parser p = (XML_Parser)data;
    XML_Size lineno = XML_GetCurrentLineNumber(p);
}

int
main(int argc, char *argv[])
{
  XML_Parser p = XML_ParserCreate(NULL);
  if (! p) {
    fprintf(stderr, "Couldn't allocate memory for parser\n");
    exit(-1);
  }

  FILE* f = fopen("1.xml", "rb");
  
  XML_SetElementHandler(p, start, end);
  XML_SetUserData(p, p);

    for (;;)
    {
        int done;
        int len;

        len = (int)fread(Buff, 1, BUFFSIZE, f);
        if (ferror(f)) {
            fprintf(stderr, "Read error\n");
            exit(-1);
        }
        done = feof(f);

        if (XML_Parse(p, Buff, len, done) == XML_STATUS_ERROR) {
            fprintf(stderr, "Parse error at line %" XML_FMT_INT_MOD "u:\n%s\n",
            XML_GetCurrentLineNumber(p),
            XML_ErrorString(XML_GetErrorCode(p)));
            exit(-1);
        }

        if (done)
            break;
    }
    XML_ParserFree(p);
    return 0;
}
