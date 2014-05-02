#include <stdio.h>
#include <libxml/xmlreader.h>

int process_node(xmlTextReaderPtr r)
{
    int rc = xmlTextReaderMoveToFirstAttribute(r);
    while (rc == 1)
    {
        const xmlChar* name = xmlTextReaderConstName(r);
        const xmlChar* value = xmlTextReaderConstValue(r);
        if (xmlStrcmp("id", name) == 0)
        {
            
        }
        printf("        %s: %s\n", name, value);
        rc = xmlTextReaderMoveToNextAttribute(r);
    }
    return 0;
}

int main()
{
    xmlTextReaderPtr r = xmlReaderForFile("new-york-latest.osm", NULL, 0);
    if (r == NULL)
    {
        printf("cannot open file\n");
        return 1;
    }

    int rc;
    int cnt = 0;
    int ret = xmlTextReaderRead(r);
    const xmlChar* name = xmlTextReaderConstName(r);
    printf("%s\n", name);
    if (xmlStrcmp("osm", name) != 0)
    {
        perror("invalid top element");
        goto out;
    }

    while (ret == 1)
    {
        ret = xmlTextReaderRead(r);
        int node_type = xmlTextReaderNodeType(r);
        if (node_type != XML_READER_TYPE_ELEMENT && node_type != XML_READER_TYPE_END_ELEMENT)
            continue;
        cnt++;
        int lineno = xmlTextReaderGetParserLineNumber(r);
        name = xmlTextReaderConstName(r);
        printf("%s %d %7d %7d\n", name, node_type, lineno, cnt);
    }

out:
    xmlFreeTextReader(r);
    xmlCleanupCharEncodingHandlers();
    xmlDictCleanup();
    xmlCleanupParser();
    xmlMemoryDump();
}
