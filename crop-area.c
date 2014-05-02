#include <stdio.h>

char* fname = "o41074b2.osm";

int main()
{
	FILE* f = fopen(fname, "rt");
	
	int nodes = 0;
	int lines = 0;
	char s[1024];
	
	while (!feof(f))
	{
		fgets(s, 1024, f);
		lines++;

		if (strncmp(" <node ", s, 7) == 0)
			nodes++;
	}
	
	fclose(f);
	
	printf("lines: %d, nodes: %d\n", lines, nodes);
}
