#include <emscripten.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

extern "C" {

EMSCRIPTEN_KEEPALIVE
int* generateMaze(int rows, int cols, int startRow, int startCol, int endRow, int endCol) {
    // Adaptation de votre code C++ existant
    rows = std::max(5, std::min(rows, 50));
    cols = std::max(5, std::min(cols, 50));
    
    // Grille linéaire (rows*cols entiers)
    int* maze = (int*)malloc(rows * cols * sizeof(int));
    if (!maze) return nullptr;
    
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    int wallPercentage = 20 + (std::rand() % 11);
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            // Par défaut, pas de mur
            maze[idx] = 0;
            
            // Ne pas mettre de mur sur start ou end
            if ((i == startRow && j == startCol) || (i == endRow && j == endCol)) {
                continue;
            }
            
            // Murs sur les bords
            if (i == 0 || i == rows-1 || j == 0 || j == cols-1) {
                maze[idx] = 1;
                continue;
            }
            
            // Éviter les murs trop près du départ/arrivée
            int distToStart = abs(i - startRow) + abs(j - startCol);
            int distToEnd = abs(i - endRow) + abs(j - endCol);
            if (distToStart <= 2 || distToEnd <= 2) {
                continue;
            }
            
            // Chance d'avoir un mur
            if (std::rand() % 100 < wallPercentage) {
                maze[idx] = 1;
            }
        }
    }
    
    // Assurer que start et end sont accessibles (déjà faits)
    // Optionnel : forcer quelques chemins comme dans votre code
    if (startRow > 0) maze[(startRow-1)*cols + startCol] = 0;
    if (startRow < rows-1) maze[(startRow+1)*cols + startCol] = 0;
    if (startCol > 0) maze[startRow*cols + (startCol-1)] = 0;
    if (startCol < cols-1) maze[startRow*cols + (startCol+1)] = 0;
    
    if (endRow > 0) maze[(endRow-1)*cols + endCol] = 0;
    if (endRow < rows-1) maze[(endRow+1)*cols + endCol] = 0;
    if (endCol > 0) maze[endRow*cols + (endCol-1)] = 0;
    if (endCol < cols-1) maze[endRow*cols + (endCol+1)] = 0;
    
    return maze;
}

// Pour libérer la mémoire allouée (optionnel, vous pouvez aussi utiliser free depuis JS)
EMSCRIPTEN_KEEPALIVE
void freeMaze(int* ptr) {
    free(ptr);
}

} // extern "C"