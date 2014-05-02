#include <stdlib.h>
#include <png.h>
#include <memory.h>

struct read_params
{
    FILE* f;
    const unsigned char* data;
    size_t len;
    size_t pos;
};

void read_file_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    png_voidp a = png_get_io_ptr(png_ptr);
    struct read_params* p = (struct read_params*)a;
    size_t was_read = fread(data, 1, length, p->f);
    if (was_read < 0)
    {
        png_error(png_ptr, "cannot read file");
    }
}

int pic_load_png(const char* fname)
{
    struct read_params params;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    int bit_depth, color_type, rowbytes, channels = 1, rc = -1;
    png_uint_32 twidth, theight, i;
    png_byte* image_data = NULL;
    png_bytep* row_pointers = NULL;

    memset(&params, 0, sizeof(struct read_params));
    params.f = fopen(fname, "rb");
    if (params.f == NULL)
        goto out;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
        goto out;

    png_set_read_fn(png_ptr, &params, read_file_fn);

    info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
        goto out;

    png_read_info(png_ptr, info_ptr);
     
    if (png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL) != 1)
        goto out;

    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(png_ptr);
        channels = 3;
    }

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {

        png_set_tRNS_to_alpha(png_ptr);
        channels += 1;
    }

    png_read_update_info(png_ptr, info_ptr);
    rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    
    image_data = (png_byte*)malloc(sizeof(png_byte) * rowbytes * theight);
    if (image_data == NULL)
        goto out;

    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * theight);
    if (row_pointers == NULL)
        goto out;

    for (i = 0; i < theight; ++i)
        row_pointers[theight - 1 - i] = image_data + i * rowbytes;
 
    png_read_image(png_ptr, row_pointers);
    rc = 0;

out:
    
    if (png_ptr != NULL && info_ptr != NULL)
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    else if (png_ptr != NULL)
        png_destroy_read_struct(&png_ptr, 0, 0);

    if (image_data != NULL)
        free(image_data);
    
    if (row_pointers != NULL)
        free(row_pointers);

    if (params.f != NULL)
        fclose(params.f);

    return rc;
}

int main(int argc, char** argv)
{
    pic_load_png("b2.png");

    return 0;
}

