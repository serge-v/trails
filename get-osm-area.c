#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <expat.h>
#include "std_hash.h"
#include "coordconv.h"

struct rect
{
	float l,t,r,b;
};

int pt_in_rect(const struct rect* rc, double x, double y)
{
	return (rc->l <= x && rc->r >= x && rc->t >= y && rc->b <= y) ? 1 : 0;
}

enum parent_type
{
	NONE    = ' ',
	WAY     = 'w',
	NODE    = 'n'
};

const char* amenities[] =
{
	"amenity",
	"power",
	"railway",
	"barrier",
	"natural",
	"landuse",
	"leisure",
	"place",
	"shop",
	"tourism",
	"historic",
	"boundary",
	"waterway",
	"man_made",
	"source"
};

int namenities = sizeof(amenities) / sizeof(amenities[0]);

static struct option long_options[] =
{
	{"help",        no_argument,       0, 'h'},
	{"version",     no_argument,       0, 'v'},
	{"osm-file",    required_argument, 0, 'f'},
	{"bounds",      required_argument, 0, 'f'},
	{0, 0, 0, 0}
};

void usage()
{
	printf(
	    "get-osm-area. Version 0.1\n"
	    "Parse OSM XML file and dump list of ids and names.\n"
	    "Usage: get-osm-area [OPTIONS]\n"
	    "Options:\n"
	    "    --help,-h                  -- show help\n"
	    "    --version,-v               -- show version\n"
	    "    --bounds,-b L,T,R,B        -- bounds in degrees\n"
	    "    --osm-file,-f FILENAME     -- specify input osm file\n"
	    "\n"
	);
}

struct osm_parser
{
	const char*             osm_file_name;      // input file name
	FILE*                   osm_file;           // input file
	char*                   buff;               // read buffer
	int                     buff_size;          // read buffer size
	struct rect             bounds;             // area bounds
	XML_Parser              expat;              // XML parser
	struct std_hashtable*   nodes_table;        // all nodes in bounding rect
	uint64_t                id;                 // current object id
	enum parent_type        geo_type;           // currect object type
	int                     in_rect;            // in bounding rect
	int                     print;              // print object
	int                     depth;              // current depth of element in XML tree
	XML_Size                line;               // current object line number in osm file
	XML_Index               offset;             // current object offset in osm file
	char                    geo_name[200];      // current object name
	char                    geo_highway[200];   // current highway type
	char                    geo_amenity[200];   // current amenity
};

static void XMLCALL
start_element(void *data, const char *el, const char **attr)
{
	struct osm_parser* p = (struct osm_parser*)data;

	int i;

	p->depth++;

	if (strcmp("node", el) == 0)
	{
		struct node n = {0, 0, 0};
		for (i = 0; attr[i]; i += 2)
		{
			if (strcmp(attr[i], "id") == 0)
			{
				n.id = strtoul(attr[i+1], NULL, 10);
			}
			else if (strcmp(attr[i], "lat") == 0)
			{
				n.lat = atof(attr[i+1]);
			}
			else if (strcmp(attr[i], "lon") == 0)
			{
				n.lon = atof(attr[i+1]);
			}
		}

		p->id = n.id;
		p->line = XML_GetCurrentLineNumber(p->expat);
		p->offset = XML_GetCurrentByteIndex(p->expat);
		strcpy(p->geo_name, "");
		strcpy(p->geo_highway, "");
		strcpy(p->geo_amenity, "");
		p->print = 0;
		p->geo_type = NODE;
		p->in_rect = pt_in_rect(&p->bounds, n.lon, n.lat);
		if (p->in_rect)
		{
			struct node* nnode = node_create(n.id, n.lon, n.lat);
			struct node_key* nkey = malloc(sizeof(struct node_key));
			nkey->id = n.id;
			std_hash_table_insert(p->nodes_table, nkey, nnode);
		}
		return;
	}

	if (strcmp("way", el) == 0)
	{
		strcpy(p->geo_name, "");
		strcpy(p->geo_highway, "");
		strcpy(p->geo_amenity, "");
		p->geo_type = WAY;
		for (i = 0; attr[i]; i += 2)
		{
			if (strcmp(attr[i], "id") == 0)
			{
				p->id = strtoul(attr[i+1], NULL, 10);
				break;
			}
		}
		p->line = XML_GetCurrentLineNumber(p->expat);
		p->offset = XML_GetCurrentByteIndex(p->expat);
		p->in_rect = 0;
		p->print = 0;
		return;
	}

	if (p->geo_type != NONE && strcmp("nd", el) == 0)
	{
		struct node_key nkey;
		for (i = 0; attr[i]; i += 2)
		{
			if (strcmp(attr[i], "ref") == 0)
			{
				nkey.id = strtoul(attr[i+1], NULL, 10);
				break;
			}
		}

		struct node* n = std_hash_table_lookup(p->nodes_table, &nkey);
		if (n != NULL)
		{
			p->in_rect = 1;    // some node of the way in rect
		}

		return;
	}

	if (p->geo_type == NONE || strcmp("tag", el) != 0)
	{
		return;
	}

	p->print = 1; // print if object has any tag

	const char* key = NULL;
	const char* value = NULL;

	for (i = 0; attr[i]; i += 2)
	{
		if (strcmp(attr[i], "k") == 0)
		{
			key = attr[i+1];
		}
		else if (strcmp(attr[i], "v") == 0)
		{
			value = attr[i+1];
		}
	}

	if (strcmp("name", key) == 0)
	{
		strncpy(p->geo_name, value, sizeof(p->geo_name));
	}
	else if (strcmp("highway", key) == 0)
	{
		strncpy(p->geo_highway, value, sizeof(p->geo_highway));
	}
	else if (strlen(p->geo_amenity) == 0)
	{
		int i;
		for (i = 0; i < namenities; i++)
		{
			if (strcmp(amenities[i], key) == 0)
			{
				int maxs = sizeof(p->geo_amenity);
				strncpy(p->geo_amenity, key, maxs);
				strncat(p->geo_amenity, "-", maxs - strlen(p->geo_amenity));
				strncat(p->geo_amenity, value, maxs - strlen(p->geo_amenity));
				break;
			}
		}
	}
}

static void XMLCALL
end_element(void *data, const char *el)
{
	struct osm_parser* p = (struct osm_parser*)data;

	int i;
	p->depth--;

	XML_Size line_end = XML_GetCurrentLineNumber(p->expat);
	XML_Index el_length = XML_GetCurrentByteIndex(p->expat) -
	                      p->offset + sizeof("</>") + strlen(el);

	if (strcmp("node", el) == 0 || strcmp("way", el) == 0)
	{
		if (p->in_rect && p->print)
		{
			printf("%c %12llu %9lu %7lu %-50s %-20s %-20s\n",
			       p->geo_type, p->id, p->offset, el_length,
			       p->geo_name, p->geo_highway, p->geo_amenity);
		}
		p->in_rect = 0;
		p->geo_type = NONE;
		p->line = 0;
	}
}

void osm_parser_destroy(struct osm_parser* p)
{
	if (p == NULL)
	{
		return;
	}

	if (p->expat != NULL)
	{
		XML_ParserFree(p->expat);
	}

	if (p->nodes_table != NULL)
	{
		std_hash_table_foreach_remove(p->nodes_table, node_destroy, NULL);
		std_hash_table_destroy(p->nodes_table);
	}

	if (p->buff != NULL)
	{
		free(p->buff);
	}

	memset(p, 0, sizeof(struct osm_parser));
}

struct osm_parser* osm_parser_create()
{
	struct osm_parser* p = malloc(sizeof(struct osm_parser));
	if (p == NULL)
	{
		return NULL;
	}

	memset(p, 0, sizeof(struct osm_parser));

	p->expat = XML_ParserCreate(NULL);
	if (p->expat == NULL)
	{
		fprintf(stderr, "cannot create parser\n");
		goto err;
	}

	p->nodes_table = std_hash_table_new(node_hash, node_equal);
	if (p->nodes_table == NULL)
	{
		fprintf(stderr, "cannot create hashtable\n");
		goto err;
	}

	p->buff_size = 8192;
	p->buff = malloc(p->buff_size);
	if (p->buff == NULL)
	{
		fprintf(stderr, "cannot allocate read buffer\n");
		goto err;
	}

	XML_SetElementHandler(p->expat, start_element, end_element);
	XML_SetUserData(p->expat, p);

	struct rect bounds =
	{
		.l = -74.25,
		.t = 41.25,
		.r = -74.00,
		.b = 41.125
	};

	memcpy(&p->bounds, &bounds, sizeof(struct rect));

	return p;

err:
	osm_parser_destroy(p);
	return NULL;
};

int main(int argc, char** argv)
{
	int ret = 1;

	struct osm_parser* parser = osm_parser_create();
	if (parser == NULL)
	{
		fprintf(stderr, "cannot create parser\n");
		goto out;
	}

	int option_index = 0, c, n;

	while (1)
	{
		c = getopt_long (argc, argv, "hvf:", long_options, &option_index);
		if (c == -1)
		{
			break;
		}

		switch (c)
		{
		case 'v':
			puts ("option -v\n");
			break;

		case 'f':
			parser->osm_file_name = optarg;
			break;

		case 'b':
			n = sscanf(optarg, "%f,%f,%f,%f",
			           &parser->bounds.l, &parser->bounds.t,
			           &parser->bounds.r, &parser->bounds.b);
			if (n != 4)
			{
				fprintf(stderr, "Invalid bounds: %s\n", optarg);
				goto out;
			}
			break;

		case 'h':
			usage();
			goto out;
			break;

		default:
			fprintf(stderr, "Invalid option: %c\n", c);
			break;
		}
	}

	if (parser->osm_file_name == NULL)
	{
		fprintf(stderr, "filename is not spacified");
		goto out;
	}

	parser->osm_file = fopen(parser->osm_file_name, "rb");
	if (parser->osm_file == NULL)
	{
		fprintf(stderr, "unable to open %s\n", parser->osm_file_name);
		goto out;
	}

	for (;;)
	{
		int len = fread(parser->buff, 1, parser->buff_size, parser->osm_file);

		if (ferror(parser->osm_file))
		{
			fprintf(stderr, "Read error\n");
			goto out;
		}

		int done = feof(parser->osm_file);

		if (XML_Parse(parser->expat, parser->buff, len, done) == XML_STATUS_ERROR)
		{
			fprintf(stderr, "Parse error at line %lu:\n%s\n",
			        XML_GetCurrentLineNumber(parser->expat),
			        XML_ErrorString(XML_GetErrorCode(parser->expat)));
			goto out;
		}

		if (done)
		{
			break;
		}
	}

	ret = 0;

out:

	if (parser != NULL)
	{
		osm_parser_destroy(parser);
	}

	return ret;
}
