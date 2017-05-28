/*
    Chris M. Thomasson's Hybrid Reverse 2-ary Julia

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*_____________________________________________________________*/


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <complex.h>
#include <tgmath.h>
#include <stdbool.h>


#define CT_RAND() (rand() / (RAND_MAX - 0.0))


struct ct_axes
{
    double xmin;
    double xmax;
    double ymin;
    double ymax;
};


struct ct_canvas
{
    unsigned long width;
    unsigned long height;
    unsigned char* buf;
};

bool
ct_canvas_create(
    struct ct_canvas* const self,
    unsigned long width,
    unsigned long height
){
    size_t size = width * height;

    self->buf = calloc(1, size);

    if (self->buf)
    {
        self->width = width;
        self->height = height;

        return true;
    }

    return false;
}

void
ct_canvas_destroy(
    struct ct_canvas const* const self
){
    free(self->buf);
}

bool
ct_canvas_save_ppm(
    struct ct_canvas const* const self,
    char const* fname
){
    FILE* fout = fopen(fname, "w");

    if (fout)
    {
        char const ppm_head[] =
            "P3\n"
            "# Chris M. Thomasson RIFC Cipher Renderer ver:0.0.0.0 (pre-alpha)";

        fprintf(fout, "%s\n%lu %lu\n%u\n",
                ppm_head,
                self->width, self->height,
                255U);

        size_t size = self->width * self->height;

        for (size_t i = 0; i < size; ++i)
        {
            unsigned int c = self->buf[i];
            fprintf(fout, "%u %u %u  ", c, 0U, 0U);
        }

        if (! fclose(fout))
        {
            return true;
        }
    }

    return false;
}


struct ct_plane
{
    struct ct_axes axes;
    struct ct_canvas* canvas;
};

size_t
ct_plane_project(
    struct ct_plane const* const self,
    double complex c
){
    double awidth = self->axes.xmax - self->axes.xmin;
    double aheight = self->axes.ymax - self->axes.ymin;

    double xstep = awidth / (self->canvas->width - 1.0);
    double ystep = aheight / (self->canvas->height - 1.0);

    size_t x = (creal(c) - self->axes.xmin) / xstep;
    size_t y = (self->axes.ymax - cimag(c)) / ystep;

    size_t i = x + y * self->canvas->height;

    return i;
}

bool
ct_plane_plot(
    struct ct_plane* const self,
    double complex c,
    unsigned char color
){
    size_t cp = ct_plane_project(self, c);

    if (cp < self->canvas->height * self->canvas->width)
    {
        self->canvas->buf[cp] = color;
        return true;
    }

    return false;
}


// Compute the fractal
void
ct_ifs(
    struct ct_plane* const self,
    double complex z,
    double complex c,
    double ratio,
    unsigned long n
){
    printf("ct_ifs: %lf%+lfi, ratio:%lf\n", creal(c), cimag(c), ratio);

    // 2 sets
    double complex jp[] = {
        .0 + I*.0,
        -5.5 + I*.0
    };

    for (unsigned long i = 0; i < n; ++i)
    {
        double rn0 = rand() / (RAND_MAX - .0);
        double rn1 = rand() / (RAND_MAX - .0);

        if (rn0 > .5)
        {
            c = jp[0];
        }

        else
        {
            c = jp[1];
        }


        double complex d = z - c;
        double complex root = csqrt(d);

        z = root;

        if (rn1 > ratio)
        {
            z = -root;
        }

        ct_plane_plot(self, z, 255);
        ct_plane_plot(self, root, 255);

        if (! (i % 256))
        {
            printf("rendering: %lu of %lu\r", i + 1, n);
        }
    }

    printf("rendering: %lu of %lu\n", n, n);
}


#define CT_WIDTH 1024
#define CT_HEIGHT 1024
#define CT_N 10000000

int main(void)
{
    struct ct_canvas canvas;

    bool status = ct_canvas_create(&canvas, CT_WIDTH, CT_HEIGHT);
    assert(status);

    double radius = 3.14;
    struct ct_axes axes = { -radius, radius, -radius, radius };
    struct ct_plane plane = { axes, &canvas };

    // Julia circle at origin z and Julia point c = 0+0i
    double complex z = 0+0*I;
    double complex c = 0+0*I; // Julia
    double ratio = .5;

    ct_ifs(&plane, z, c, ratio, CT_N);

    status = ct_canvas_save_ppm(&canvas, "ct_cipher_rifc.ppm");
    assert(status);
    printf("\ncreated: ct_cipher_rifc.ppm\n");

    ct_canvas_destroy(&canvas);

    return 0;
}
