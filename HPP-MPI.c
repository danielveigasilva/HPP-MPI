#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <time.h>
#include <mpi.h>
#include <math.h>

typedef enum {DIREITA = 0, CIMA = 1, ESQUERDA = 2, BAIXO = 3} Direcao;

int * int_to_bits(int num) {
    int *bits = (int *) calloc(4, sizeof(int));
    for (int i = 0; i < 4; i++)
        bits[4 - 1 - i] = (num >> i) & 1;
    return bits;
}

int bits_to_int(int bits[4]) {
    int num = 0;
    for (int i = 0; i < 4; i++)
        num = (num << 1) | bits[i];
    return num;
}

void set_particle(int ** X, int p, int q, Direcao i, int status){
    int * site = int_to_bits(X[p][q]);
    site[i] = status;
    X[p][q] = bits_to_int(site);
}

int get_particle(int ** X, int p, int q, Direcao i){
    int * site = int_to_bits(X[p][q]);
    return site[i];
}

int * matrix_to_vector(int ** X, int p, int q){
    int * v = (int*) calloc(p*q, sizeof(int));
    for (int i = 0; i < p; i++)
        for (int j = 0; j < q; j++)
             v[(i * q) + j] = X[i][j];
    return v;
}

int ** vector_to_matrix(int * v, int p, int q){
    int ** X = (int**) calloc(p, sizeof(int*));
    for (int i = 0; i < p; i++){
        X[i] = (int*) calloc(q, sizeof(int));
        for (int j = 0; j < q; j++)
            X[i][j] =v[(i * q) + j];
    }
    return X;
}

void print_X(int ** X, int n_rows, int n_col){
    for (int j = 0; j < n_col+1; j++)
        printf("__");
    printf("\n");

    for (int i = 0; i < n_rows; i++){
        for (int j = 0; j < n_col; j++){
            if (j == 0)
                printf("|");
            if (!get_particle(X,i,j,ESQUERDA) && !get_particle(X,i,j,DIREITA) && !get_particle(X,i,j,BAIXO) && !get_particle(X,i,j,CIMA))
                printf("  ");
            else if (get_particle(X,i,j,ESQUERDA) && !get_particle(X,i,j,DIREITA) && !get_particle(X,i,j,BAIXO) && !get_particle(X,i,j,CIMA))
                printf("← ");
            else if (!get_particle(X,i,j,ESQUERDA) && get_particle(X,i,j,DIREITA) && !get_particle(X,i,j,BAIXO) && !get_particle(X,i,j,CIMA))
                printf("→ ");
            else if (!get_particle(X,i,j,ESQUERDA) && !get_particle(X,i,j,DIREITA) && get_particle(X,i,j,BAIXO) && !get_particle(X,i,j,CIMA))
                printf("↓ ");
            else if (!get_particle(X,i,j,ESQUERDA) && !get_particle(X,i,j,DIREITA) && !get_particle(X,i,j,BAIXO) && get_particle(X,i,j,CIMA))
                printf("↑ ");
            else if (get_particle(X,i,j,ESQUERDA) && get_particle(X,i,j,DIREITA) && !get_particle(X,i,j,BAIXO) && !get_particle(X,i,j,CIMA))
                printf("⇆ ");
            else if (!get_particle(X,i,j,ESQUERDA) && !get_particle(X,i,j,DIREITA) && get_particle(X,i,j,BAIXO) && get_particle(X,i,j,CIMA))
                printf("⇅ ");
            else
                printf("+ ");
            if (j == n_col-1)
                printf("|");
        }
        printf("\n");
    }
    for (int j = 0; j < n_col+1; j++)
        printf("__");
    printf("\n");
}

int sigma(int ** X, int n, int p, int q, Direcao i){
    if (p < 0 || q < 0 || p > n-1 || q > n-1)
        return 0;
    return get_particle(X, p, q, i);
}

int psi(int ** X, int n, int p, int q){
    int psi_1 = sigma(X, n, p-1, q, BAIXO) & sigma(X, n, p+1, q, CIMA) & !sigma(X, n, p, q-1, DIREITA) & !sigma(X, n, p, q+1, ESQUERDA);
    int psi_2 = !sigma(X, n, p-1, q, BAIXO) & !sigma(X, n, p+1, q, CIMA) & sigma(X, n, p, q-1, DIREITA) & sigma(X, n, p, q+1, ESQUERDA);
    return psi_1 - psi_2;
}

int ** init_X(int n) {
    int ** X = (int**) calloc(n, sizeof(int*));
    for (int i = 0; i < n; i++)
        X[i] = (int*) calloc(n, sizeof(int));
    return X;
}

int ** random_X(int n, int np) {
    int ** X = init_X(n);
    
    srand(time(NULL));
    while (np > 0){
        Direcao direcao = rand() % 4;
        //impede geracao nas fronteiras
        int p = (rand() % (n-2)) + 1;
        int q = (rand() % (n-2)) + 1;

        if (get_particle(X, p, q, direcao) == 0){
            set_particle(X, p, q, direcao, 1);
            np --;
        }
    }
    return X;
}

int get_chunk(int n, int size, int rank){
    int extra_work = n % size;
    int chunk = n/size;
    if (rank < extra_work)
        return chunk + 1;
    return chunk;
}

int get_p_global(int p_local, int n, int size, int rank){
    int increment = 0;
    for (int i = 0; i < rank; i ++)
        increment += get_chunk(n, size, i);
    return p_local + increment;
}

int ** get_next_X_MPI(int ** X_current, int n, int chunk, int size, int rank){
	
    int ** X_next = (int**) calloc(chunk, sizeof(int*));
    for (int i = 0; i < chunk; i++)
        X_next[i] = (int*) calloc(n, sizeof(int));

    for (int p = 0; p < chunk ; p++){
        
        int p_global = get_p_global(p, n, size, rank);

        for (int q = 0; q < n ; q++){

            set_particle(X_next, p, q, BAIXO, sigma(X_current, n, p_global-1, q, BAIXO) - psi(X_current, n, p_global, q));
            set_particle(X_next, p, q, DIREITA, sigma(X_current, n, p_global, q-1, DIREITA) + psi(X_current, n, p_global, q));
            set_particle(X_next, p, q, CIMA, sigma(X_current, n, p_global+1, q, CIMA) - psi(X_current, n, p_global, q));
            set_particle(X_next, p, q, ESQUERDA, sigma(X_current, n, p_global, q+1, ESQUERDA) + psi(X_current, n, p_global, q));
        
            //Colisao bordas
            if (q==0 && get_particle(X_next,p,q,ESQUERDA)){
                set_particle(X_next,p,q,ESQUERDA,0);
                set_particle(X_next,p,q,DIREITA,1);
            }
            else if (q==n-1 && get_particle(X_next,p,q,DIREITA)){
                set_particle(X_next,p,q,DIREITA,0);
                set_particle(X_next,p,q,ESQUERDA,1);
            }
            else if (p_global==0 && get_particle(X_next,p,q,CIMA)){
                set_particle(X_next,p,q,CIMA,0);
                set_particle(X_next,p,q,BAIXO,1);
            }
            else if (p_global==n-1 && get_particle(X_next,p,q,BAIXO)){
                set_particle(X_next,p,q,BAIXO,0);
                set_particle(X_next,p,q,CIMA,1);
            }
        }    
    }

    return X_next;
}

int ** get_next_X(int ** X_current, int n){
	
    int ** X_next = (int**) calloc(n, sizeof(int*));
    for (int i = 0; i < n; i++)
        X_next[i] = (int*) calloc(n, sizeof(int));

    for (int p = 0; p < n ; p++){
        for (int q = 0; q < n ; q++){
            set_particle(X_next, p, q, BAIXO, sigma(X_current, n, p-1, q, BAIXO) - psi(X_current, n, p, q));
            set_particle(X_next, p, q, DIREITA, sigma(X_current, n, p, q-1, DIREITA) + psi(X_current, n, p, q));
            set_particle(X_next, p, q, CIMA, sigma(X_current, n, p+1, q, CIMA) - psi(X_current, n, p, q));
            set_particle(X_next, p, q, ESQUERDA, sigma(X_current, n, p, q+1, ESQUERDA) + psi(X_current, n, p, q));
        
            //Colisao bordas
            if (q==0 && get_particle(X_next,p,q,ESQUERDA)){
                set_particle(X_next,p,q,ESQUERDA,0);
                set_particle(X_next,p,q,DIREITA,1);
            }
            else if (q==n-1 && get_particle(X_next,p,q,DIREITA)){
                set_particle(X_next,p,q,DIREITA,0);
                set_particle(X_next,p,q,ESQUERDA,1);
            }
            else if (p==0 && get_particle(X_next,p,q,CIMA)){
                set_particle(X_next,p,q,CIMA,0);
                set_particle(X_next,p,q,BAIXO,1);
            }
            else if (p==n-1 && get_particle(X_next,p,q,BAIXO)){
                set_particle(X_next,p,q,BAIXO,0);
                set_particle(X_next,p,q,CIMA,1);
            }
        }    
    }

    return X_next;
}

void HPP(int * v_current, int n, int np, int ni){
    for (int i = 0; i < ni; i++){ 
        int * v_next = matrix_to_vector(get_next_X(vector_to_matrix(v_current, n, n), n), n, n);
        v_current = v_next;
    }
}

void HPP_MPI(int * v_current, int n, int np, int com_grafico, int delta_T, int ni, int size, int rank){

    int * v_next;
    int chunk = get_chunk(n, size, rank);

    int counts_per_rank[size];
    for (int rank_i = 0; rank_i < size; rank_i++)
        counts_per_rank[rank_i] = get_chunk(n,size,rank_i) * n;

    int displs_per_rank[size];
    displs_per_rank[0] = 0;
    for (int rank_i = 1; rank_i < size; rank_i++)
        displs_per_rank[rank_i] = displs_per_rank[rank_i-1] + counts_per_rank[rank_i-1];

    for (int i = 0; i < ni; i++){
        MPI_Bcast(v_current, n*n, MPI_INT, 0, MPI_COMM_WORLD);

        if (com_grafico && rank == 0){
            system("clear");
            printf(">> Interação %d\n", i+1);
            print_X(vector_to_matrix(v_current, n, n), n, n);
            usleep(delta_T);
        }
        
        if (rank == 0)
            v_next = matrix_to_vector(init_X(n), n, n);

        int * v_next_local = matrix_to_vector(get_next_X_MPI(vector_to_matrix(v_current, n, n), n, chunk, size, rank), chunk, n);

        MPI_Gatherv(v_next_local, chunk * n, MPI_INT, v_next, counts_per_rank, displs_per_rank,MPI_INT, 0, MPI_COMM_WORLD);
        //MPI_Gather(v_next_local, chunk * n, MPI_INT, v_next, chunk * n, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank == 0)
            v_current = v_next;
    }
}

int main(int argc, char ** argv){

    int size,rank;
    MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = atoi(argv[1]);
    int np = atoi(argv[2]);
    int ni = atoi(argv[3]);

    int com_grafico = 0;
    int delta_T = 0;
    if (argc > 4){
        delta_T = atoi(argv[4]);
        com_grafico = 1;
    }

    int * v_current = matrix_to_vector(init_X(n), n, n);
    if (rank == 0)
        v_current = matrix_to_vector(random_X(n, np), n, n);
    
    double hpp_time, hpp_mpi_time;
    if (rank == 0 && !com_grafico){
        
        int * v_current_sequential = (int *) calloc(n * n, sizeof(int));
        for (int i = 0; i < n*n; i++)
            v_current_sequential[i] = v_current[i];

        double hpp_start = MPI_Wtime();

        HPP(v_current_sequential, n, np, ni);
        
        double hpp_end = MPI_Wtime();
        hpp_time = hpp_end - hpp_start;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double hpp_mpi_start = MPI_Wtime();

    HPP_MPI(v_current, n, np, com_grafico, delta_T, ni, size, rank);
    
    MPI_Barrier(MPI_COMM_WORLD);
    double hpp_mpi_end = MPI_Wtime();
    hpp_mpi_time = hpp_mpi_end - hpp_mpi_start;

    if (rank == 0 && !com_grafico){
        printf("* Tamanho Matriz: %d\n", n);
        printf("* Número Particulas: %d\n", np);
        printf("* Número Processos: %d\n", size);
        printf("* Tempo Computacional Sequencial: %f\n", hpp_time);
        printf("* Tempo Computacional Paralelo: %f\n", hpp_mpi_time);
        printf("* Speedup: %f\n", hpp_time/hpp_mpi_time);
        printf("\n");
    }

    MPI_Finalize();
    return 0;
}