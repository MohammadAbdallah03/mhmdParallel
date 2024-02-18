#include <stdio.h>
#include <time.h>

#define WIDTH 640
#define HEIGHT 400
#define MAX_ITER 255

void generate_mandelbrot(int image[HEIGHT][WIDTH]) {
    int i, j;
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            double x = (j - WIDTH / 2.0) * 4.0 / WIDTH;
            double y = (i - HEIGHT / 2.0) * 4.0 / HEIGHT;

            double z_real = x;
            double z_imag = y;

            int iter;
            for (iter = 0; iter < MAX_ITER; iter++) {
                double z_real2 = z_real * z_real;
                double z_imag2 = z_imag * z_imag;
                if (z_real2 + z_imag2 > 4.0)
                    break;
                z_imag = 2 * z_real * z_imag + y;
                z_real = z_real2 - z_imag2 + x;
            }

            image[i][j] = iter;
        }
    }
}

void save_ppm(const char *filename, int image[HEIGHT][WIDTH]) {
    FILE *file = fopen(filename, "wb");

    fprintf(file, "P6\n%d %d\n255\n", WIDTH, HEIGHT);

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            unsigned char color = (unsigned char)((image[i][j] % 256) * 255 / MAX_ITER);
            fwrite(&color, sizeof(unsigned char), 1, file);
            fwrite(&color, sizeof(unsigned char), 1, file);
            fwrite(&color, sizeof(unsigned char), 1, file);
        }
    }

    fclose(file);
}

int main() {
    int image[HEIGHT][WIDTH];

    clock_t start_time = clock();

    generate_mandelbrot(image);

    clock_t end_time = clock();

    double total_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    save_ppm("mandelbrot.ppm", image);

    printf("Execution time: %f seconds\n", total_time);

    return 0;
}