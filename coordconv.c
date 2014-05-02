#include "coordconv.h"
#include <stdio.h>
#include <memory.h>

struct coord_convertor
{
	struct tfw tfw;
	projPJ pj_merc;
	projPJ pj_latlong;
	projPJ pj_nad27;
	projPJ pj_nad83;
	projPJ pj_utm83;
};

struct node*
node_create(uint64_t id, double lon, double lat)
{
	struct node* p = malloc(sizeof(struct node));
	memset(p, 0, sizeof(struct node));
	p->id = id;
	p->lon = lon;
	p->lat = lat;
	return p;
}

int
node_compare_seq(const void * data0, const void * data1)
{
	struct node* n0 = (struct node*)data0;
	struct node* n1 = (struct node*)data1;
	if (n0->seq_num > n1->seq_num)
		return 1;

	if (n0->seq_num < n1->seq_num)
		return -1;

	return 0;
}

unsigned int
node_hash(const void *key)
{
	struct node_key* n = (struct node_key*)key;
	return n->id % 0xFFFFFFFF;
}

int
node_equal(const void *a, const void *b)
{
	struct node_key* na = (struct node_key*)a;
	struct node_key* nb = (struct node_key*)b;
	return (na->id == nb->id) ? 1 : 0;
}

void
node_print(void *key, void *value, void *user_data)
{
	struct node* n = (struct node*)value;
	printf("%llu %f %f\n", n->id, n->lon, n->lat);
}

int
node_destroy(void *key, void *value, void *user_data)
{
	struct node_key* kn = (struct node_key*)key;
	struct node* kv = (struct node*)value;
	free(kn);
	free(kv);
	return 1;
}

struct coord_convertor*
coordconv_create(struct tfw* tfw)
{
	struct coord_convertor* p = malloc(sizeof(struct coord_convertor));
	if (p == NULL)
		goto err;

	memset(p, 0, sizeof(struct coord_convertor));

	p->pj_merc = pj_init_plus("+proj=utm +ellps=clrk66 +zone=18");
	if (p->pj_merc == NULL)
		goto err;

	p->pj_latlong = pj_init_plus("+proj=latlong +ellps=clrk66");
	if (p->pj_latlong == NULL)
		goto err;

	p->pj_nad27 = pj_init_plus("+proj=latlong +datum=NAD27");
	if (p->pj_nad27 == NULL)
		goto err;

	p->pj_nad83 = pj_init_plus("+proj=latlong +datum=NAD83");
	if (p->pj_nad83 == NULL)
		goto err;

	p->pj_utm83 = pj_init_plus("+proj=utm +datum=NAD83 +zone=18");
	if (p->pj_utm83 == NULL)
		goto err;

	if (tfw != NULL)
		p->tfw = *tfw;

	return p;

err:

	coordconv_destroy(p);

	if (p != NULL)
		free(p);

	return NULL;
}

int
coordconv_destroy(struct coord_convertor* c)
{
	if (c->pj_merc != NULL)
		pj_free(c->pj_merc);

	if (c->pj_latlong != NULL)
		pj_free(c->pj_latlong);

	if (c->pj_nad27 != NULL)
		pj_free(c->pj_nad27);

	if (c->pj_nad83 != NULL)
		pj_free(c->pj_nad83);

	if (c->pj_utm83 != NULL)
		pj_free(c->pj_utm83);

	pj_deallocate_grids();
	memset(c, 0, sizeof(struct coord_convertor));
	free(c);

	return 0;
}

int
coordconv_load_tfw(struct coord_convertor* c, const char* fname)
{
	return tfw_read(&c->tfw, fname);
}

void
coordconv_calc_pixels(struct coord_convertor* c, struct node* n)
{
	tfw_utm_to_px(&c->tfw, n->utm_lon, n->utm_lat, &n->x, &n->y);
}

int
coordconv_calc_utm(struct coord_convertor* c, struct node* n)
{
	int rc;
	n->utm_lat = n->lat * DEG_TO_RAD;
	n->utm_lon = n->lon * DEG_TO_RAD;
//	rc = pj_transform(c->pj_latlong, c->pj_merc, 1, 1, &n->utm_lon, &n->utm_lat, NULL);
	rc = pj_transform(c->pj_nad83, c->pj_utm83, 1, 1, &n->utm_lon, &n->utm_lat, NULL);
	return rc;
}

int
coordconv_calc_nad83_to_nad27(struct coord_convertor* c, struct node* n)
{
	int rc;
	n->lat = n->lat * DEG_TO_RAD;
	n->lon = n->lon * DEG_TO_RAD;
	rc = pj_transform(c->pj_nad83, c->pj_nad27, 1, 1, &n->lon, &n->lat, NULL);
	n->lat = n->lat * RAD_TO_DEG;
	n->lon = n->lon * RAD_TO_DEG;

	return rc;
}
