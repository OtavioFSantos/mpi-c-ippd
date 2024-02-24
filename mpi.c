#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// matriz NxN
#define N 10

// inicia a matriz com valores aleatorios entre 1 e 0
void initialize_board(int board[][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            board[i][j] = rand() % 2;
        }
    }
}

// conta o numero de vizinhos da celula
int count_neighbors(int board[][N], int x, int y) {
    int count = 0;
    for (int i = x - 1; i <= x + 1; i++) {
        for (int j = y - 1; j <= y + 1; j++) {
            // conta na vertical, horizontal e diagonal
            if ((i != x || j != y) && i >= 0 && j >= 0 && i < N && j < N) {
                count += board[(i + N) % N][(j + N) % N];
            }
        }
    }
    return count;
}

// calcula se uma celula deve morrer, renascer ou continuar no mesmo estado
int evolve_cell(int board[][N], int x, int y) {
    int neighbors = count_neighbors(board, x, y);
    if (board[x][y] == 1) {
        if (neighbors < 2 || neighbors > 3) {
            return 0; // solidao ou superpopulaçao
        } else {
            return 1; 
        }
    } else {
        if (neighbors == 3) {
            return 1; // renasce
        } else {
            return 0;
        }
    }
}

// evolui a geraçao
void evolve_generation(int local_board[][N], int rank, int size) {
    int temp_board[N][N];

    // cria uma copia local do tabuleiro
    memcpy(temp_board, local_board, sizeof(int) * N * N);

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            temp_board[i][j] = evolve_cell(local_board, i, j);
        }
    }

    memcpy(local_board, temp_board, sizeof(int) * N * N);
}


int main(int argc, char *argv[]) {
    int rank, size;
    int local_board[N][N];
    int global_board[N][N];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // divide a matriz entre os processos
    MPI_Scatter(global_board, N * N / size, MPI_INT, local_board, N * N / size, MPI_INT, 0, MPI_COMM_WORLD);

    // inicializa a matriz para cada processo
    initialize_board(local_board);

    for (int generation = 0; generation <= 10; generation++) {
        evolve_generation(local_board, rank, size);

        // reune todos no processo raiz
        MPI_Gather(local_board, N * N / size, MPI_INT, global_board, N * N / size, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            printf("\ngeracao %d:\n", generation);
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    printf("%d ", global_board[i][j]);
                }
                printf("\n");
            }
        }
    }

    MPI_Finalize();
    return 0;
}
