#include <stdio.h>
#include <mpi.h>
#include <sys/time.h>

#define WIDTH 800
#define HEIGHT 600
#define MAX_ITER 1000

void generate_mandelbrot(int my_start_row, int end_row, int* chunk) {
    int i, j;
    int index = 0;
    for (i = my_start_row; i < end_row; i++) {
        for (j = 0; j < WIDTH; j++) {
            double x = (j - WIDTH / 2.0) * 4.0 / WIDTH;
            double y = (i - HEIGHT / 2.0) * 4.0 / HEIGHT;

            double real_part = x;
            double imag_part = y;

            int iter;
            for (iter = 0; iter < MAX_ITER; iter++) {
                double real_part_squared = real_part * real_part;
                double imag_part_squared = imag_part * imag_part;
                if (real_part_squared + imag_part_squared > 4.0)
                    break;
                imag_part = 2 * real_part * imag_part + y;
                real_part = real_part_squared - imag_part_squared + x;
            }

            chunk[index++] = iter;
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

int main(int argc, char *argv[]) {
    int image[HEIGHT][WIDTH];

    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int rows_per_proc = HEIGHT / num_procs;
    int my_start_row = rank * rows_per_proc;
    int end_row = (rank + 1) * rows_per_proc;

    int chunk_size = rows_per_proc * WIDTH;
    int chunk[chunk_size];

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // Use non-blocking send and receive
    MPI_Request send_request, recv_request;
    MPI_Status send_status, recv_status;

    // Non-blocking send
    MPI_Isend(chunk, chunk_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &send_request);

    generate_mandelbrot(my_start_row, end_row, chunk);

    // Non-blocking receive
    MPI_Irecv(&image[my_start_row][0], chunk_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &recv_request);

    // Wait for communication to complete
    MPI_Wait(&send_request, &send_status);
    MPI_Wait(&recv_request, &recv_status);

    gettimeofday(&end_time, NULL);
    double execution_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    if (rank == 0) {
        save_ppm("mandelbrotdynamic.ppm", image);
        printf("Execution time: %f seconds\n", execution_time);
    }

    MPI_Finalize();

    return 0;
}