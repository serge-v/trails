#include <libxml/xmlreader.h>
#include "coordconv.h"

enum parent_type
{
    NONE    = ' ',
    WAY     = 'w',
    NODE    = 'n'
};

enum object_type
{
    AMNT_NONE,
    AMNT_PARKING,
    AMNT_MOUNTAIN
};

struct way
{
	uint64_t                id;		// osm id
	char                    name[200];
	int                     print;		// output flag
	xmlListPtr              nodes;		// all way nodes
	struct node*            last_node;      // last node for printing annotation
};

struct place
{
	uint64_t                id;            // osm id
	char                    name[200];
	int                     print;         // output flag
	struct node*            node;
};

struct osm_parser
{
	const char*             osm_file_names[10];   // process up to 10 osm files
	int                     n_osm_file_names;     // number of osm files
	const char*             tfw_file_name;
	const char*             mvg_file_name;
	const char*             log_file_name;
	FILE*                   log_file;
	FILE*                   mvg_file;
	struct std_hashtable*   nodes_table;          // all osm nodes
	struct coord_convertor* cconv;
	enum parent_type        geo_type;             // current context parent type (way or node)
	enum object_type        obj_type;             // current geo object type
	struct way*             curr_way;             // currently parsing way
	struct place*           curr_place;           // currently parsing place
	int                     annotate_num;         // current sequential annotation number
};

struct way* way_create();
void way_clear(struct way* p);
int way_destroy(struct way* p);
int parser_init(struct osm_parser* p);
int parser_destroy(struct osm_parser* p);
void parse_node_attrs(xmlTextReaderPtr r, struct node* n);
