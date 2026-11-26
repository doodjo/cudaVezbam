#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>

__global__ void kreirajMatricuC(float* A, float* B, float* C, int N){
    int tid = blockIdx * blockDim.x + threadIdx.x;  //jedinstveni id za nit

    int korak = blockDim.x * gridDim.x;
    int ukupnoElemenata = N*N;

    for (int idx = tid; idx < ukupnoElemenata; idx+=korak){
        int i = idx / N;
        int j = idx % N;
        float vrednostA = A[i*N+j];
        float vrednostB = B[j*N+i];
        if (vrednostA < vrednostB){
            C[idx] = vrednostA;
        } else {
            C[idx] = vrednostB;
        }
    }
}

__global__ void kreirajVekorSrednjaVrednostKolone(float* C, float* V, int N){
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    int korak = blockDim.x * gridDim.x;

    for (int kol = tid; kol<N; kol+=korak){
        float suma = 0.0f;
        for (int red = 0; red < N; red++){
            suma +=C[red * N + kol];
        }
        V[kol] = suma/N;
    }
}

int main(){
    int N = 0;
    scanf("%d", &N);

    size_t velicinaMatrice = (size_t)N*N*sizeof(float);
    size_t velicinaVektora = (size_t)N*sizeof(float);

    float* h_A = (float*)malloc(velicinaMatrice);
    float* h_B = (float*)malloc(velicinaMatrice);
    float* h_C = (float*)malloc(velicinaMatrice);
    float* h_V = (float*)malloc(velicinaVektora);

    for (int i = 0; i<N; i++){
        for (j = 0; j<N; j++){
            h_A[i*N+j] = (float)(i+j+1);
            h_B[i*N+j] = (float)(i+2);
        }
    }
    
    //deklaracija pokazivaca na mem na gpu
    float* d_A = NULL;
    float* d_B = NULL;
    float* d_C = NULL;
    float* d_V = NULL;

    //alokacija mem na gpu
    cudaMalloc((void**)&d_A, velicinaMatrice);
    cudaMalloc((void**)&d_B, velicinaMatrice);
    cudaMalloc((void**)&d_C, velicinaMatrice);
    cudaMalloc((void**)&d_V, velicinaMatrice);

    cudaMemcpy(d_A, h_A, velicinaMatrice, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, velicinaMatrice, cudaMemcpyHostToDevice);

    int velicinaBloka = 256;
    int brojBlokovaC = N * N + velicinaBloka - 1 / velicinaBloka;
    if (brojBlokovaC > 256) brojBlokovaC = 256;

    kreirajMatricuC<<<brojBlokovaC, velicinaBloka>>>(d_A, d_B, d_C, N);
    cudaDeviceSynchronize();

    cudaMemcpy(h_C, d_C, velicinaMatrice, cudaMemcpyDeviceToHost);

    int brojBlokovaV = N + velicinaBloka - 1 / velicinaBloka;
    if (brojBlokovaV > 256) brojBlokovaC = 256;

    kreirajVekorSrednjaVrednostKolone<<<brojBlokovaV, velicinaBloka>>>(d_C, d_V, N);
    cudaDeviceSynchronize();

    cudaMemcpy(h_V, d_V, velicinaVektora, cudaMemcpyDeviceToHost);

    // ispis razultata
    if (N <= 10){
        printf('\nMatrica C:\n');
        for(int i = 0; i < N; i++){
            for(int j = 0; j < N; j++){
                printf("%6.2f"h_C[i*N+j]);
            }
        }
    }

    printf('\nVektor V:\n');
    for(int i = 0; i < 10; i++) printf("V[%d], h_V[i]");

    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
    cudaFree(d_V);

    free(h_A);
    free(h_B);
    free(h_C);
    free(h_V);

    return 0;
}
