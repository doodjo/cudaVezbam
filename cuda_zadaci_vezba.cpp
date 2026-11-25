#include stdio.h
#include stdlib.h
#include cuda_runtime.h


// gpu funkcija
__global__ void detektujNezaliveneBiljke(const float* A, int* periodi, int M, int N) {

    int vrsta = blockIdx.x * blockDim.x + threadIdx.x;

    if (vrsta < M){
        int protekloSati = 0; // brojac sati od poslednjeg uspesnog zalivanja

        for (int kolona = N-1; kolona >=1; kolona--) {
            float trenutnaVlaznost = A[vrsta * N + kolona];
            float prethodnaVlaznost = A[vrsta * N + kolona-1];

            if (trenutnaVlaznost < 0.5f && trenutnaVlaznost < prethodnaVlaznost) {
                protekloSati++;
            } else {
                break; // jer je vlaznost dovolja
            }
        }
        periodi[vrsta] = protekloSati;
    }
    
}
//host kod
int main(){
    int M = 4;
    int N = 7;

    size_t velicinaA = M * N * sizeof(float); //ukupno bajtova za matricuA
    size_t velicinaPeriodi = M * sizeof(int); //ukupno bajtova za niz rezultata

    float* h_A = (float*)malloc(velicinaA); // cpu niz za smestanje matrice
    float* h_B = (int*)malloc(velicinaPeriodi);

    // Primer podataka sa senzora (4 biljke, 7 sati merenja: od Sata 0 do Sata 6)
    float primerPodataka[4][7] = {
        {0.80f, 0.60f, 0.45f, 0.40f, 0.35f, 0.30f, 0.25f}, // Biljka 0: 5 sati bez zalivanja (TREBA ZALITI)
        {0.40f, 0.35f, 0.85f, 0.70f, 0.45f, 0.40f, 0.35f}, // Biljka 1: 3 sata bez zalivanja (OK)
        {0.48f, 0.45f, 0.42f, 0.38f, 0.33f, 0.29f, 0.20f}, // Biljka 2: 6 sati bez zalivanja (TREBA ZALITI)
        {0.90f, 0.85f, 0.80f, 0.75f, 0.70f, 0.65f, 0.60f}  // Biljka 3: 0 sati bez zalivanja (OK)
    };

    //2d u 1d niz prebacivanje
    for (int i = 0; i<M; i++){
        for (int j = 0; j<N ; j++){
            h_A[i*N+j] = primerPodataka[i][j];
        }
    }

    float* d_A = NULL;
    float* d_Periodi = NULL;

    cudaMalloc((void**)&d_A, velicinaA);
    cudaMalloc((void**)&d_Periodi, velicinaPeriodi);

    cudaMemcpy(d_A, h_A, velicina_A, cudaMemcpyHostToDevice);

    int velicinaBloka = 256;
    int brojBlokova = (M + velicinaBloka - 1)/velicinaBloka; // koliko imamo blokova, zaokruzeno na vise jer moze da se desi da nije deljivo

    detektujNezaliveneBiljke<<<brojBlokova, velicinaBloka>>>(d_A, d_Periodi, M, N);

    // Čekamo da GPU završi sve niti pre nego što CPU nastavi dalje
    cudaDeviceSynchronize(); // Sinhronizacija CPU-a i GPU-a

    cudaMemcpy(h_periodi, d_periodi, velicinaPeriodi, cudaMemcpyDeviceToHost);

    printf("Izvestaj o zalivanju biljaka:\n");
        for (int i = 0; i < M; i++) { // Prolazimo kroz svaku biljku
            printf("Biljka %d: proteklo %d sat(i) od zalivanja", i, h_periodi[i]);
            if (h_periodi[i] >= 5) { // Proveravamo da li je prošlo 5 ili više sati
                printf(" --> [POTREBNO DOPUNITI VODU!]"); // Upozorenje korisniku
            }
            printf("\n"); // Novi red za sledeću biljku
    }

    // Oslobađanje zauzete memorije na GPU-u (sprečava curenje VRAM memorije)
    cudaFree(d_A);       // Brišemo matricu A sa GPU-a
    cudaFree(d_periodi); // Brišemo rezultate sa GPU-a

    // Oslobađanje zauzete memorije na CPU-u (sprečava curenje RAM memorije)
    free(h_A);           // Brišemo matricu A iz RAM-a
    free(h_periodi);     // Brišemo rezultate iz RAM-a

    return 0; // Uspešan završetak programa
}






