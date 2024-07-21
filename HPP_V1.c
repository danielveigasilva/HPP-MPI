#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <time.h>

typedef enum {DIREITA = 1, CIMA = 2, ESQUERDA = 3, BAIXO = 4} Direcao;

void print_X(int *** X, int n){
    for (int j = 0; j < n+1; j++)
        printf("__");
    printf("\n");

    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            if (j == 0)
                printf("|");
            if (!X[i][j][ESQUERDA] && !X[i][j][DIREITA] && !X[i][j][BAIXO] && !X[i][j][CIMA])
                printf("  ");
            else if (X[i][j][ESQUERDA] && !X[i][j][DIREITA] && !X[i][j][BAIXO] && !X[i][j][CIMA])
                printf("← ");
            else if (!X[i][j][ESQUERDA] && X[i][j][DIREITA] && !X[i][j][BAIXO] && !X[i][j][CIMA])
                printf("→ ");
            else if (!X[i][j][ESQUERDA] && !X[i][j][DIREITA] && X[i][j][BAIXO] && !X[i][j][CIMA])
                printf("↓ ");
            else if (!X[i][j][ESQUERDA] && !X[i][j][DIREITA] && !X[i][j][BAIXO] && X[i][j][CIMA])
                printf("↑ ");
            else if (X[i][j][ESQUERDA] && X[i][j][DIREITA] && !X[i][j][BAIXO] && !X[i][j][CIMA])
                printf("⇆ ");
            else if (!X[i][j][ESQUERDA] && !X[i][j][DIREITA] && X[i][j][BAIXO] && X[i][j][CIMA])
                printf("⇅ ");
            else
                printf("+ ");
            if (j == n-1)
                printf("|");
        }
        printf("\n");
    }
    for (int j = 0; j < n+1; j++)
        printf("__");
    printf("\n");
}

int sigma(int *** X, int n, int p, int q, Direcao i){
    if (p < 0 || q < 0 || p > n-1 || q > n-1)
        return 0;
    return X[p][q][i];
}

int psi(int *** X, int n, int p, int q){
    int psi_1 = sigma(X, n, p-1, q, BAIXO) & sigma(X, n, p+1, q, CIMA) & !sigma(X, n, p, q-1, DIREITA) & !sigma(X, n, p, q+1, ESQUERDA);
    int psi_2 = !sigma(X, n, p-1, q, BAIXO) & !sigma(X, n, p+1, q, CIMA) & sigma(X, n, p, q-1, DIREITA) & sigma(X, n, p, q+1, ESQUERDA);
    return psi_1 - psi_2;
}

int *** init_X(int n) {
    int *** X = (int***) calloc(n, sizeof(int**));
    for (int i = 0; i < n; i++){
        X[i] = (int**) calloc(n, sizeof(int*));
        for (int j = 0; j < n; j++)
            X[i][j] = (int*) calloc(5, sizeof(int));
    }
    return X;
}

int *** random_X(int n, int np) {
    int *** X = init_X(n);

    //X[5][0][BAIXO] = 1;
    //X[11][0][CIMA] = 1;
    //X[5][9][CIMA] = 1;
    //X[10][4][DIREITA] = 1;
    //X[10][7][ESQUERDA] = 1;

    srand(time(NULL));
    while (np > 0){
        Direcao direcao = (rand() % 4) + 1;
        //impede geracao nas fronteiras
        int x = (rand() % (n-2)) + 1;
        int y = (rand() % (n-2)) + 1;

        int posicao_valida = 1;
        for (int k = 1; k < 5; k++)
            posicao_valida &= !X[x][y][k];

        if (posicao_valida){
            X[x][y][direcao] = 1;
            np --;
        }
    }
    
    return X;
}

int *** get_next_X(int *** X_current, int n){
    int *** X_next = init_X(n);
    for (int p = 0; p < n ; p++){
        for (int q = 0; q < n ; q++){
            X_next[p][q][BAIXO]     = sigma(X_current, n, p-1, q, BAIXO) - psi(X_current, n, p, q);
            X_next[p][q][DIREITA]   = sigma(X_current, n, p, q-1, DIREITA) + psi(X_current, n, p, q);
            X_next[p][q][CIMA]      = sigma(X_current, n, p+1, q, CIMA) - psi(X_current, n, p, q);
            X_next[p][q][ESQUERDA]  = sigma(X_current, n, p, q+1, ESQUERDA) + psi(X_current, n, p, q);

            //Colisao bordas
            if (q==0 && X_next[p][q][ESQUERDA]){
                X_next[p][q][ESQUERDA] = 0;
                X_next[p][q][DIREITA] = 1;
            }
            else if (q==n-1 && X_next[p][q][DIREITA]){
                X_next[p][q][DIREITA] = 0;
                X_next[p][q][ESQUERDA] = 1;
            }
            else if (p==0 && X_next[p][q][CIMA]){
                X_next[p][q][CIMA] = 0;
                X_next[p][q][BAIXO] = 1;
            }
            else if (p==n-1 && X_next[p][q][BAIXO]){
                X_next[p][q][BAIXO] = 0;
                X_next[p][q][CIMA] = 1;
            }
        }    
    }

    return X_next;
}

void HPP(int n, int np, int com_grafico, int delta_T, int ni){
    int *** X_current = random_X(n, np);
    for (int i = 0; i < ni; i++){
        if (com_grafico){
            system("clear");
            printf(">> Interação %d\n", i+1);
            print_X(X_current, n);
            usleep(delta_T);
        }
        int *** X_next = get_next_X(X_current, n);
        X_current = X_next;
    }
}

int main(int argc, char ** argv){
    int n = 20;
    int np = 300;
    int com_grafico = 1;
    int delta_T = 50000;
    int ni = 1000000;
    
    HPP(n, np, com_grafico, delta_T, ni);
    return 0;
}