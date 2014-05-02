#include "osm-parsing.h"
#include <memory.h>
#include "std_hash.h"

// =================== way =========================================

struct way*
way_create()
{
	struct way* p = malloc(sizeof(struct way));
	if (p == NULL)
	{
		goto err;
	}

	memset(p, 0, sizeof(struct way));
	p->nodes = xmlListCreate(NULL, node_compare_seq);
	if (p->nodes == NULL)
	{
		goto err;
	}

	return p;

err:

	if (p->nodes != NULL)
	{
		xmlListDelete(p->nodes);
	}
	if (p != NULL)
	{
		free(p);
	}

	return NULL;
}

void
way_clear(struct way* p)
{
	*p->name = 0;
	xmlListClear(p->nodes);
	p->print = 0;
	p->id = 0;
}

int
way_destroy(struct way* p)
{
	if (p->nodes != NULL)
	{
		xmlListDelete(p->nodes);
	}
	free(p);
	return 0;
}

// =================== parser =========================================

int
parser_init(struct osm_parser* p)
{
	memset(p, 0, sizeof(struct osm_parser));
	p->nodes_table = std_hash_table_new(node_hash, node_equal);
	if (p->nodes_table == NULL)
	{
		goto err;
	}

	p->cconv = coordconv_create(NULL);
	if (p->cconv == NULL)
	{
		goto err;
	}

	p->curr_way = way_create();
	if (p->curr_way == NULL)
	{
		goto err;
	}

	p->curr_place = malloc(sizeof(struct place));
	if (p->curr_place == NULL)
	{
		goto err;
	}

	return 0;
err:
	parser_destroy(p);
	return -1;
}

int
parser_destroy(struct osm_parser* p)
{
	if (p->nodes_table != NULL)
	{
		std_hash_table_foreach_remove(p->nodes_table, node_destroy, NULL);
		std_hash_table_destroy(p->nodes_table);
	}

	if (p->cconv != NULL)
	{
		coordconv_destroy(p->cconv);
	}

	if (p->curr_way != NULL)
	{
		way_destroy(p->curr_way);
	}

	if (p->curr_place != NULL)
	{
		free(p->curr_place);
	}

	memset(p, 0, sizeof(struct osm_parser));

	return 0;
}

void
parse_node_attrs(xmlTextReaderPtr r, struct node* n)
{
	int rc = xmlTextReaderMoveToFirstAttribute(r);

	while (rc == 1)
	{
		const xmlChar* name = xmlTextReaderConstName(r);
		const xmlChar* value = xmlTextReaderConstValue(r);

		if (xmlStrcmp("lat", name) == 0)
			n->lat = atof(value);
		else if (xmlStrcmp("lon", name) == 0)
			n->lon = atof(value);
		else if (xmlStrcmp("id", name) == 0)
			n->id = strtoul(value, NULL, 10);

		rc = xmlTextReaderMoveToNextAttribute(r);
	}
}
