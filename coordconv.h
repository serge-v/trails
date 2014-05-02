#include <proj_api.h>
#include <stdint.h>
#include "tfw.h"

struct node_key
{
	uint64_t id;
};

struct node
{
	int         seq_num;
	uint64_t    id;
	double      lat;
	double      lon;
	double      utm_lat;
	double      utm_lon;
	int         zone;
	int         x;
	int         y;
};

struct node* node_create(uint64_t id, double lon, double lat);
int node_compare_seq(const void * data0, const void * data1);
unsigned int node_hash(const void *key);
int node_equal(const void *a, const void *b);
void node_print(void *key, void *value, void *user_data);
int node_destroy(void *key, void *value, void *user_data);

struct coord_convertor;

struct coord_convertor* coordconv_create(struct tfw* tfw);
int coordconv_destroy(struct coord_convertor* c);
int coordconv_load_tfw(struct coord_convertor* c, const char* fname);
void coordconv_calc_pixels(struct coord_convertor* c, struct node* n);
int coordconv_calc_utm(struct coord_convertor* c, struct node* n);
int coordconv_calc_nad83_to_nad27(struct coord_convertor* c, struct node* n);
void coordconv_px_to_utm(struct coord_convertor* c, double x, double y, double* utm_x, double* utm_y);
