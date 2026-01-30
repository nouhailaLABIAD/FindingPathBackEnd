#include <cstdlib>
#include <emscripten.h>
#include "DijkstraSolver.h" 
#include <chrono>
#include <iostream>
#include <iomanip> // Pour contrôler le nombre de chiffres après la virgule

using namespace std;
extern "C" {

 int ReteiveDecimalPart(float nombre )
{
// 1. On s'assure que le nombre est positif pour le calcul
    if (nombre < 0) nombre = -nombre;

    // 2. On isole la partie décimale (ex: 5.333 -> 0.333)
    float partieDecimale = nombre - (int)nombre;

    // 3. On multiplie par 100 et on convertit en entier (ex: 0.333 -> 33)
    int resultat = (int)(partieDecimale * 100);

    return resultat;
}

EMSCRIPTEN_KEEPALIVE
int* dijkstra(
    int n,
    int* matrix,
    int start,
    int end
) {
    DijkstraSolver solver(n, matrix);
    auto start_time = chrono::high_resolution_clock::now();

    // Exécution de Dijkstra
    PathResult result = solver.solve(start, end);

    // Mesure du temps de fin
    auto end_time = chrono::high_resolution_clock::now();

    // Calcul de la durée en millisecondes (entier)
    //auto duration_ms = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
    std::chrono::duration<double , std::milli> duration_ms = end_time - start_time;
    std::cout << std::fixed << std::setprecision(2);
    auto convertDuration = duration_ms.count() ;

    int size = result.path.size();
    int sizeVisitedNodes = result.visitedNodes.size();

    /* 4 =>
         - 2 first case for size path and size NodesVisited 
         - 2 other case for execution time ( entier + decimal)
    */

    int* buffer = (int*)malloc((size + sizeVisitedNodes + 4) * sizeof(int));
    
    if (buffer == nullptr) {
        // En cas d'échec d'allocation (rare)
        return nullptr;
    }
      //cout << duration_ms ;

    buffer[0] = size;           // nombre de nœuds dans le chemin
    buffer[1] = sizeVisitedNodes ;
    buffer[2] = (int)convertDuration;    // temps en millisecondes
    buffer[3] = ReteiveDecimalPart(convertDuration);

    for (int i = 0; i < size; i++) {
        buffer[i + 4] = result.path[i];
    }
  
    int indexVisited = size + 4 ;
    for(int i = 0 ; i< sizeVisitedNodes; i++)
        buffer[i+indexVisited] = result.visitedNodes[i] ;
        

    return buffer;
}

}
