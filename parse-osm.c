#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "getopt.h"
#include <memory.h>
#include <libxml/xmlreader.h>
#include <libxml/list.h>
#include "std_hash.h"
#include "osm-parsing.h"

static int
print_node(const void* data, const void* user)
{
	struct osm_parser* p = (struct osm_parser*)user;
	struct node* n = (struct node*)data;
	if (n->seq_num % 10 == 0)
	{
		fprintf(p->mvg_file, "\n        ");
	}
	fprintf(p->mvg_file, "%d,%d ", n->x, n->y);
	if (p->log_file != NULL)
		fprintf(p->log_file, "    mvg node: %d %llu %f,%f %f,%f %d,%d\n",
		        n->seq_num, n->id, n->lon, n->lat, n->utm_lon, n->utm_lat, n->x, n->y);
	return 1;
}

static void
replace_apostrophe(char* ptr)
{
	ptr = strchr(ptr, '\'');
	while (ptr != NULL)
	{
		*ptr = '-';
		ptr = strchr(ptr + 1, '\'');
	}
}

static void
annotate_way(struct osm_parser* p)
{
	fprintf(p->mvg_file, "  stroke black\n");
	fprintf(p->mvg_file, "  text-undercolor none\n");
	fprintf(p->mvg_file, "  font courier\n");
	fprintf(p->mvg_file, "  font-size 40\n");
	fprintf(p->mvg_file, "  stroke-width 1\n");
	p->annotate_num++;
	fprintf(p->mvg_file, "  text %d,%d '%d'\n", p->curr_way->last_node->x, p->curr_way->last_node->y, p->annotate_num);
	if (p->log_file != NULL)
	{
		fprintf(p->log_file, "text %3d %12llu %s\n", p->annotate_num, p->curr_way->id, p->curr_way->name);
	}
}

static void
draw_way_polyline(struct osm_parser* p, const char* color)
{
	fprintf(p->mvg_file, "  stroke-width 16\n");
	fprintf(p->mvg_file, "  stroke %s\n", "lightgray");
	fprintf(p->mvg_file, "  stroke-dasharray 0\n");
	fprintf(p->mvg_file, "  polyline ");
	xmlListWalk(p->curr_way->nodes, print_node, p);
	fprintf(p->mvg_file, "\n");

	fprintf(p->mvg_file, "  stroke-width 12\n");
	fprintf(p->mvg_file, "  stroke %s\n", color);
	fprintf(p->mvg_file, "  stroke-dasharray 0\n");
	fprintf(p->mvg_file, "  polyline ");
	xmlListWalk(p->curr_way->nodes, print_node, p);
	fprintf(p->mvg_file, "\n");
}

static void
draw_way_dashed_polyline(struct osm_parser* p, const char* color)
{
	draw_way_polyline(p, "white");

	fprintf(p->mvg_file, "  stroke-width 12\n");
	fprintf(p->mvg_file, "  stroke %s\n", color);
	fprintf(p->mvg_file, "  stroke-dasharray 20 30\n");
	fprintf(p->mvg_file, "  polyline ");
	xmlListWalk(p->curr_way->nodes, print_node, p);
	fprintf(p->mvg_file, "\n");
}

static void
draw_utm_grid(struct osm_parser* p)
{
	struct node nw, sw, ne, se;
	int x, y;

	fprintf(p->mvg_file, "  stroke green\n");
	fprintf(p->mvg_file, "  stroke-width 2\n");

	// vertical

	nw.lat = 41.25;
	nw.lon = -74.25;

	sw.lat = 41.125;
	sw.lon = -74.25;

	ne.lat = 41.25;
	ne.lon = -74.000;

	se.lat = 41.125;
	se.lon = -74.000;

	coordconv_calc_utm(p->cconv, &nw);
	coordconv_calc_utm(p->cconv, &sw);
	coordconv_calc_utm(p->cconv, &ne);
	coordconv_calc_utm(p->cconv, &se);

//    printf("se: %f %f\n", se.utm_lon, se.utm_lat);

	coordconv_calc_pixels(p->cconv, &se);
	x = se.x;
	y = se.y;
	fprintf(p->mvg_file, "  line %d,%d %d,%d line %d,%d %d,%d \n", x-20, y, x+20, y, x, y-20, x, y+20);
	/*
	    int i;
	    int utm_start_x = (int)nw.utm_lon / 1000 * 1000;
	    int utm_end_x = (int)ne.utm_lon / 1000 * 1000;

	    for (i = utm_start_x; i <= utm_end_x; i += 1000 )
	    {
	        nw.utm_lon = i;
	        coordconv_calc_pixels(p->cconv, &nw);

	        sw.utm_lon = i;
	        coordconv_calc_pixels(p->cconv, &sw);

	        fprintf(p->mvg_file, "  line %d,%d, %d,%d\n", nw.x, nw.y, sw.x, sw.y);
	        fprintf(p->mvg_file, "  text %d,%d '%d'\n", nw.x, nw.y - 30, (int)nw.utm_lon / 1000);
	        fprintf(p->mvg_file, "  text %d,%d '%d'\n", sw.x, sw.y + 30, (int)sw.utm_lon / 1000);
	    }

	    // horizontal

	    nw.lat = 41.25;
	    nw.lon = -74.25;

	    sw.lat = 41.125;
	    sw.lon = -74.25;

	    ne.lat = 41.25;
	    ne.lon = -74.000;

	    coordconv_calc_utm(p->cconv, &nw);
	    coordconv_calc_utm(p->cconv, &sw);
	    coordconv_calc_utm(p->cconv, &ne);

	    int utm_start_y = (int)sw.utm_lat / 1000 * 1000;
	    int utm_end_y = (int)nw.utm_lat / 1000 * 1000;

	    for (i = utm_start_y; i <= utm_end_y; i += 1000 )
	    {
	        nw.utm_lat = i;
	        coordconv_calc_pixels(p->cconv, &nw);

	        ne.utm_lat = i;
	        coordconv_calc_pixels(p->cconv, &ne);

	        fprintf(p->mvg_file, "  line %d,%d, %d,%d\n", nw.x, nw.y, ne.x, ne.y);
	        fprintf(p->mvg_file, "  text %d,%d '%d'\n", nw.x - 30, nw.y, (int)nw.utm_lat / 1000);
	        fprintf(p->mvg_file, "  text %d,%d '%d'\n", ne.x + 30, ne.y, (int)ne.utm_lat / 1000);
	    }
	*/
}

static void
add_node_to_table(struct osm_parser* p, xmlTextReaderPtr r)
{
	struct node* nnode;
	struct node_key* nkey;
	struct node n = {0, 0, 0};
	parse_node_attrs(r, &n);

	nnode = node_create(n.id, n.lon, n.lat);
	nkey = malloc(sizeof(struct node_key));

	nkey->id = n.id;
	std_hash_table_insert(p->nodes_table, nkey, nnode);

//    if (coordconv_calc_nad83_to_nad27(p->cconv, nnode) != 0)
//        fprintf(stderr, "cannot convert to NAD27\n");
	if (coordconv_calc_utm(p->cconv, nnode) != 0)
	{
		fprintf(stderr, "cannot convert to UTM\n");
	}
	coordconv_calc_pixels(p->cconv, nnode);

	p->curr_place->id = n.id;
	p->curr_place->print = 0;
	p->curr_place->node = nnode;
	p->geo_type = NODE;

	if (p->log_file != NULL)
		fprintf(p->log_file, "node %llu %f,%f %f,%f %d,%d\n",
		        nnode->id, nnode->lon, nnode->lat, nnode->utm_lon, nnode->utm_lat, nnode->x, nnode->y);
}

static void
add_ndref_to_way(struct osm_parser* p, xmlTextReaderPtr r)
{
	struct node* n;
	struct node_key nkey = {0};

	int rc = xmlTextReaderMoveToFirstAttribute(r);

	while (rc == 1)
	{
		const xmlChar* name = xmlTextReaderConstName(r);
		const xmlChar* value = xmlTextReaderConstValue(r);

		if (xmlStrcmp("ref", name) == 0)
		{
			nkey.id = strtoul(value, NULL, 10);
		}

		rc = xmlTextReaderMoveToNextAttribute(r);
	}

	n = std_hash_table_lookup(p->nodes_table, &nkey);
	if (n != NULL)
	{
		n->seq_num = xmlListSize(p->curr_way->nodes);
		if (p->log_file != NULL)
			fprintf(p->log_file, "    nd %d %llu %f,%f %f,%f %d,%d\n",
			        n->seq_num, n->id, n->lon, n->lat, n->utm_lon, n->utm_lat, n->x, n->y);
		if (xmlListAppend(p->curr_way->nodes, n) != 0)
		{
			fprintf(stderr, "cannot insert node %llu into way node list\n", nkey.id);
		}
		p->curr_way->last_node->x = n->x;
		p->curr_way->last_node->y = n->y;
	}
	else
	{
		fprintf(stderr, "wrong node ref id: %llu\n", nkey.id);
	}
}

static void
start_new_way(struct osm_parser* p, xmlTextReaderPtr r)
{
	int rc = 0;

	way_clear(p->curr_way);

	rc = xmlTextReaderMoveToFirstAttribute(r);

	while (rc == 1)
	{
		const xmlChar* name = xmlTextReaderConstName(r);
		const xmlChar* value = xmlTextReaderConstValue(r);

		if (xmlStrcmp("id", name) == 0)
		{
			p->curr_way->id = strtoul(value, NULL, 10);
		}

		rc = xmlTextReaderMoveToNextAttribute(r);
	}

	p->geo_type = WAY;

	if (p->log_file != NULL)
	{
		fprintf(p->log_file, "way %llu\n", p->curr_way->id);
	}
}

static void
process_tag(struct osm_parser* p, xmlTextReaderPtr r)
{
	const xmlChar* k = NULL;
	const xmlChar* v = NULL;

	int rc = xmlTextReaderMoveToFirstAttribute(r);

	while (rc == 1)
	{
		const xmlChar* name = xmlTextReaderConstName(r);
		const xmlChar* value = xmlTextReaderConstValue(r);

		if (xmlStrcmp("k", name) == 0)
		{
			k = value;
		}
		else if (xmlStrcmp("v", name) == 0)
		{
			v = value;
		}

		rc = xmlTextReaderMoveToNextAttribute(r);
	}

	if (p->geo_type == WAY)
	{
		if (xmlStrcmp("name", k) == 0)
		{
			strncpy(p->curr_way->name, v, 200);
			replace_apostrophe(p->curr_way->name);
		}

		if (xmlStrcmp("highway", k) == 0 && xmlStrcmp("footway", v) == 0)
		{
			p->curr_way->print = 1;
		}
	}
	else if (p->geo_type == NODE)
	{
		if (xmlStrcmp("name", k) == 0)
		{
			strncpy(p->curr_place->name, v, 200);
			replace_apostrophe(p->curr_place->name);
		}

		if (xmlStrcmp("natural", k) == 0 && xmlStrcmp("peak", v) == 0)
		{
			p->curr_place->print = 1;
			p->obj_type = AMNT_MOUNTAIN;
		}
		else if (xmlStrcmp("amenity", k) == 0 && xmlStrcmp("parking", v) == 0)
		{
			p->curr_place->print = 1;
			p->obj_type = AMNT_PARKING;
		}
	}

	if (p->log_file != NULL)
	{
		fprintf(p->log_file, "    %s %s, %c\n", k, v, p->geo_type);
	}
}

struct trail
{
	const char* name;
	const char* color;
	int         dashed;
};

static struct trail trails[] =
{
	{ "Racoon Brook Hills - black on white",    "black",        1 },
	{ "Blue Disc",                              "blue",         0 },
	{ "White Bar",                              "white",        0 },
	{ "Stony Brook - yellow",                   "goldenrod1",   0 },
	{ "Blue",                                   "blue",         0 },
	{ "HTS - orange",                           "orange",       0 },
	{ "Pine Meadow red on white",               "red",          1 },
	{ "Reeve-s Brook - white",                  "white",        0 },
	{ "7 Hills Trail",                          "blue",         1 },
	{ "default",                                "red",          0 }
};

static const int ntrails = sizeof(trails) / sizeof(trails[0]);

static void
print_way(struct osm_parser* p)
{
	int i;
	fprintf(p->mvg_file, "  stroke-width 4\n");
	for (i = 0; i < ntrails; i++)
	{
		if (strcmp(trails[i].name, p->curr_way->name) == 0 || strcmp(trails[i].name, "default") == 0)
		{
			if (trails[i].dashed)
			{
				draw_way_dashed_polyline(p, trails[i].color);
			}
			else
			{
				draw_way_polyline(p, trails[i].color);
			}
			break;
		}
	}

	if (p->curr_way->name != NULL)
	{
		annotate_way(p);
	}
}

static void
annotate_place(struct osm_parser* p)
{
	fprintf(p->mvg_file, "  stroke black\n");
	fprintf(p->mvg_file, "  text-undercolor none\n");
	fprintf(p->mvg_file, "  font courier\n");
	fprintf(p->mvg_file, "  font-size 40\n");
	fprintf(p->mvg_file, "  stroke-width 1\n");
	p->annotate_num++;
	fprintf(p->mvg_file, "  text %d,%d '%d'\n", p->curr_place->node->x, p->curr_place->node->y, p->annotate_num);
	if (p->log_file != NULL)
		fprintf(p->log_file, "text %3d %d,%d '%s'\n", p->annotate_num,
		        p->curr_place->node->x, p->curr_place->node->y, p->curr_place->name);
}

static void
print_place(struct osm_parser* p)
{
	fprintf(p->mvg_file, "  stroke-width 4\n");

	if (p->obj_type == AMNT_PARKING)
	{
		fprintf(p->mvg_file, "  stroke black\n");
		fprintf(p->mvg_file, "  font courier\n");
		fprintf(p->mvg_file, "  font-size 60\n");
		fprintf(p->mvg_file, "  text %d,%d 'P'\n",
		        p->curr_place->node->x - 25, p->curr_place->node->y + 8);

		fprintf(p->mvg_file, "  circle %d,%d %d,%d\n",
		        p->curr_place->node->x - 10, p->curr_place->node->y - 10,
		        p->curr_place->node->x + 10, p->curr_place->node->y + 10);
	}
	else if (p->obj_type == AMNT_MOUNTAIN)
	{
		fprintf(p->mvg_file, "  circle %d,%d %d,%d\n",
		        p->curr_place->node->x - 5, p->curr_place->node->y - 5,
		        p->curr_place->node->x + 5, p->curr_place->node->y + 5);
	}

	if (p->curr_place->name != NULL)
	{
		annotate_place(p);
	}
}

static void
process_xml_element(struct osm_parser* p, xmlTextReaderPtr reader)
{
	const xmlChar* name = NULL;
	int type = xmlTextReaderNodeType(reader);

	if (type != XML_READER_TYPE_ELEMENT && type != XML_READER_TYPE_END_ELEMENT)
	{
		return;
	}

	name = xmlTextReaderConstName(reader);

	if (type == XML_READER_TYPE_END_ELEMENT)
	{
		if (xmlStrcmp("way", name) == 0)
		{
			if (p->curr_way->print)
			{
				print_way(p);
			}

			if (p->log_file != NULL)
			{
				fprintf(p->log_file, "way-end, printed: %d\n", p->curr_way->print);
			}
			way_clear(p->curr_way);
		}

		if (xmlStrcmp("node", name) == 0)
		{
			if (p->curr_place->print)
			{
				print_place(p);
			}

			if (p->log_file != NULL)
			{
				fprintf(p->log_file, "place, printed: %d\n", p->curr_place->print);
			}

			p->curr_place->id = 0;
			p->curr_place->print = 0;
			*p->curr_place->name = 0;
		}

		p->geo_type = NONE;
		p->obj_type = AMNT_NONE;

		return;
	}

	if (type == XML_READER_TYPE_ELEMENT)
	{
		if (xmlStrcmp("way", name) == 0)
		{
			start_new_way(p, reader);
		}
		else if (xmlStrcmp("node", name) == 0)
		{
			add_node_to_table(p, reader);
		}
		else if (xmlStrcmp("nd", name) == 0)
		{
			add_ndref_to_way(p, reader);
		}
		else if (xmlStrcmp("tag", name) == 0)
		{
			process_tag(p, reader);
		}
	}
}

static int
parser_process_files(struct osm_parser* p)
{
	int i;
	int ret = 1;
	xmlTextReaderPtr reader = NULL;

	if (coordconv_load_tfw(p->cconv, p->tfw_file_name) == -1)
	{
		return 1;
	}

	if (p->mvg_file_name != NULL)
	{
		p->mvg_file = fopen(p->mvg_file_name, "wt");
		if (p->mvg_file == NULL)
		{
			perror("Cannot open output file");
			goto out;
		}
	}
	else
	{
		p->mvg_file = stdout;
	}

	fprintf(p->mvg_file,
	        "push graphic-context\n"
	        "    viewbox 0 0 7200 8700\n"
	        "    stroke-width 4\n"
	        "    stroke red\n"
	        "    fill none\n"
	       );

	if (p->log_file_name != NULL)
	{
		p->log_file = fopen(p->log_file_name, "wt");
		if (p->log_file == NULL)
		{
			perror("Cannot open log file");
			goto out;
		}
	}

	for (i = 0; i < p->n_osm_file_names; i++)
	{
		if (p->log_file != NULL)
		{
			fprintf(p->log_file, "parsing %s\n", p->osm_file_names[i]);
		}

		reader = xmlReaderForFile(p->osm_file_names[i], NULL, 0);
		if (reader == NULL)
		{
			fprintf(stderr, "Unable to open %s\n", p->osm_file_names[i]);
			goto out;
		}

		ret = xmlTextReaderRead(reader);
		while (ret == 1)
		{
			process_xml_element(p, reader);
			ret = xmlTextReaderRead(reader);
		}

		draw_utm_grid(p);

		if (ret != 0)
		{
			fprintf(stderr, "%s : failed to parse\n", p->osm_file_names[i]);
			goto out;
		}

		if (reader != NULL)
		{
			xmlFreeTextReader(reader);
			reader = NULL;
		}
	}

	fprintf(p->mvg_file, "pop graphic-context\n");

	ret = 0;

out:

	if (p->mvg_file != NULL)
	{
		fclose(p->mvg_file);
		p->mvg_file = NULL;
	}

	if (p->log_file != NULL)
	{
		fclose(p->log_file);
		p->log_file = NULL;
	}

	if (reader != NULL)
	{
		xmlFreeTextReader(reader);
		reader = NULL;
	}

	return ret;
}

static struct option long_options[] =
{
	{"help",        no_argument,       0, 'h'},
	{"version",     no_argument,       0, 'v'},
	{"osm-file",    required_argument, 0, 'f'},
	{"out-file",    required_argument, 0, 'o'},
	{"tfw-file",    required_argument, 0, 'w'},
	{"log-file",    required_argument, 0, 'l'},
	{0, 0, 0, 0}
};

void
usage()
{
	printf(
	    "parse-osm. Version 0.1\n"
	    "Parse OSM XML file and produce Imagemagick MVG file.\n"
	    "Usage: parse-osm [OPTIONS]\n"
	    "Options:\n"
	    "    --help,-h                   -- show help\n"
	    "    --version,-v                -- show version\n"
	    "    --osm-file,-f FILENAME      -- specify input osm file(s)\n"
	    "    --tfw-file,-w FILENAME      -- specify input tfw file\n"
	    "    --out-file,-o FILENAME      -- specify output MVG file\n"
	    "    --log-file,-l FILENAME      -- specify optional output log file\n"
	    "\n"
	);
}

int
read_args(struct osm_parser* p, int argc, char **argv)
{
	int option_index = 0, c, ret = 0;

	while (1)
	{
		c = getopt_long (argc, argv, "hvf:w:o:l:", long_options, &option_index);
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
			p->osm_file_names[p->n_osm_file_names] = optarg;
			p->n_osm_file_names++;
			break;

		case 'h':
			ret = 1;
			break;

		case 'o':
			p->mvg_file_name = optarg;
			break;

		case 'w':
			p->tfw_file_name = optarg;
			break;

		case 'l':
			p->log_file_name = optarg;
			break;

		default:
			fprintf(stderr, "Invalid option: %c\n", c);
			ret = 1;
			break;
		}
	}

	return ret;
}

int main(int argc, char **argv)
{
	struct osm_parser parser;
	int ret = 1;

	LIBXML_TEST_VERSION

	if (parser_init(&parser) == -1)
	{
		return -1;
	}

	if (read_args(&parser, argc, argv) != 0)
	{
		usage();
		goto out;
	}

	if (parser.osm_file_names[0] == NULL)
	{
		fprintf(stderr, "input osm file(s) is not specified\n");
		goto out;
	}

	if (parser.tfw_file_name == NULL)
	{
		fprintf(stderr, "input tfw file is not specified\n");
		goto out;
	}

	ret = parser_process_files(&parser);

out:

	parser_destroy(&parser);

	xmlCleanupCharEncodingHandlers();
	xmlDictCleanup();
	xmlCleanupParser();
	xmlMemoryDump();

	return ret;
}
