#include <stdio.h>
#include <mpi.h>

#define WIDTH 800
#define HEIGHT 600
#define MAX_ITER 1000

void generate_mandelbrot(int image[HEIGHT][WIDTH], int start_row, int end_row) {
    int i, j;
    for (i = start_row; i < end_row; i++) {
        for (j = 0; j < WIDTH; j++) {
            double x = (j - WIDTH / 2.0) * 4.0 / WIDTH;
            double y = (i - HEIGHT / 2.0) * 4.0 / HEIGHT;

            double real_part = x;
            double imag_part = y;

            int iter;
            for (iter = 0; iter < MAX_ITER; iter++) {

                double real_part2 = real_part * real_part;
                double imag_part2 = imag_part * imag_part;

                if (real_part2 + imag_part2 > 4.0)
                    break;
                imag_part = 2 * real_part * imag_part + y;
                real_part = real_part2 - imag_part2 + x;
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
            
            unsigned char r = (unsigned char)((image[i][j] % 256) * 2);  // Red 
            unsigned char g = (unsigned char)((image[i][j] % 256) * 1.5);  // Green 
            unsigned char b = (unsigned char)((image[i][j] % 256) * 1);  // Blue 

            fwrite(&r, sizeof(unsigned char), 1, file);
            fwrite(&g, sizeof(unsigned char), 1, file);
            fwrite(&b, sizeof(unsigned char), 1, file);
        }
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows_per_process = HEIGHT / size;
    int start_row = rank * rows_per_process;
    int end_row = (rank + 1) * rows_per_process;

    int image[HEIGHT][WIDTH];

    double start_time = MPI_Wtime(); // Start measuring time

    generate_mandelbrot(image, start_row, end_row);

    double end_time = MPI_Wtime(); // End measuring time

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        for (int i = 1; i < size; i++) {
            MPI_Recv(&image[i * rows_per_process][0], rows_per_process * WIDTH, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        printf("Execution time: %f seconds\n", (end_time - start_time));
        save_ppm("mandelbrotstatic.ppm", image);
        
    } else {
        MPI_Send(&image[start_row][0], rows_per_process * WIDTH, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}