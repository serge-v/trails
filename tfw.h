struct tfw
{
	double pixel_size_x;
	double rot_y;
	double rot_x;
	double pixel_size_y;
	double upper_left_x;
	double upper_left_y;
};

int tfw_read(struct tfw* t, const char* fname);
int tfw_write(struct tfw* t, const char* fname);
void tfw_utm_to_px(struct tfw* t, double utm_lon, double utm_lat, int* x, int* y);
