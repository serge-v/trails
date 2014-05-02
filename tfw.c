#include "tfw.h"
#include <stdio.h>
#include <stdlib.h>

int tfw_read(struct tfw* t, const char* fname)
{
	const int SIZE = 100;
	char buf[6][100];
	char* p;
	int cnt = 0, i, rc = -1;

	FILE* f = fopen(fname, "rt");
	if (f == NULL)
	{
		fprintf(stderr, "Cannot open tfw file: %s", fname);
		return rc;
	}

	for (i = 0; i < 6; i++)
	{
		p = fgets(buf[i], SIZE-1, f);
		if (p == NULL)
		{
			fprintf(stderr, "Cannot read tfw file. It should contain 6 lines. File: %s", fname);
			goto out;
		}
	}

	t->pixel_size_x = atof(buf[0]);
	t->rot_y = atof(buf[1]);
	t->rot_x = atof(buf[2]);
	t->pixel_size_y = atof(buf[3]);
	t->upper_left_x = atof(buf[4]);
	t->upper_left_y = atof(buf[5]);

	rc = 0;
out:
	fclose(f);
	return rc;
}

int tfw_write(struct tfw* t, const char* fname)
{
	FILE* f = fopen(fname, "wt");
	if (f == NULL)
	{
		fprintf(stderr, "Cannot open tfw file: %s", fname);
		return -1;
	}

	fprintf(f, "%f\n", t->pixel_size_x);
	fprintf(f, "%f\n", t->rot_y);
	fprintf(f, "%f\n", t->rot_x);
	fprintf(f, "%f\n", t->pixel_size_y);
	fprintf(f, "%f\n", t->upper_left_x);
	fprintf(f, "%f\n", t->upper_left_y);
	fclose(f);
	return 0;
}

void tfw_utm_to_px(struct tfw* t, double utm_lon, double utm_lat, int* x, int* y)
{
	double A = t->pixel_size_x;
	double B = t->rot_x;
	double C = t->upper_left_x;
	double D = t->rot_y;
	double E = t->pixel_size_y;
	double F = t->upper_left_y;

	*x = (int)((E * utm_lon - B * utm_lat + B * F - E * C) / (A * E - D * B));
	*y = (int)((-D * utm_lon + A * utm_lat + D * C - A * F) / (A * E - D * B));
}
