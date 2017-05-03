/* Chris M. Thomasson FFE Cipher Renderer ver:0.0.0.0 (pre-alpha)
   5/2/2017 - Raw Experimental
_________________________________________________*/

#include <stdio.h>
#include <assert.h>
#include <complex.h>
#include <tgmath.h>


unsigned int
ct_raise(double n)
{
    double c = fabs(n);
    if (c == 0.0) c = 3.14592653;
    double z = c;
    double mutate = 1.61803;
    while (z < 3049213.103)
    {
        z = z * mutate + c;
        mutate = mutate * 1.314;
    }
    return z;
}


void
ct_iterate_pixel(
    FILE* fout,
    double complex z,
    double complex c,
    unsigned int imax
){
    //printf("z = %.1f%+.1fi\n", creal(z), cimag(z));

    double o = 999999999999.0;

    for (unsigned int i = 0; i < imax; ++i)
    {
        z = z * z + c;

        double dis = cabs(z);

        o = (o < dis) ? o : dis;

        if (cabs(z) > 65536.0)
        {
            double sum = fabs(creal(z)) + fabs(cimag(z));
            unsigned int red = sum * 120303U;

            fprintf(fout, "%u %u %u  ", red % 256U, 0, 0);
            return;
        }
    }

    double oraised = ct_raise(o + fabs(creal(z)));
    double sum = fabs(creal(z)) + fabs(cimag(z));
    unsigned int green = (ct_raise(sum) + oraised);

    // cheet sheat image
    fprintf(fout, "%u %u %u  ", 0, green % 256U, 0);

    // cipher image
    //fprintf(fout, "%u %u %u  ", green % 256U, 0, 0);

    return;
}


void
ct_iterate_plane(
    FILE* fout,
    unsigned int width,
    unsigned int height,
    unsigned int imax
){
    assert(width > 1 && height > 1);

    char const ppm_head[] =
        "P3\n"
        "# Chris M. Thomasson FFE Cipher Renderer ver:0.0.0.0 (pre-alpha)";

    printf("fout:%p\n", (void*)fout);

    fprintf(fout, "%s\n%u %u\n%u\n", ppm_head, width, height, 255);

    double xstep = 4.0 / (width - 1.0);
    double ystep = 4.0 / (height - 1.0);

    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            double complex z = (-2.0 + x * xstep) + I * (2.0 - y * ystep);

            //printf("(%u, %u):z = %.1f%+.1fi\n", x, y, creal(z), cimag(z));

            ct_iterate_pixel(fout, z, z, imax);
        }

        printf("processing y:%u of %u\r", y + 1, height);
    }

    printf("\nrender complete!\n");
}


int main(void)
{
    FILE* fout = fopen("ct_ffe_cipher.ppm", "w");
    assert(fout != NULL);

    ct_iterate_plane(fout, 1024, 1024, 128);

    fclose(fout);

    return 0;
}
