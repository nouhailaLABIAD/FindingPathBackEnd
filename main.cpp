#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <cassert>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

class PathfindingServer {
private:
#ifdef _WIN32
    SOCKET serverSocket;
    SOCKET clientSocket;
    WSADATA wsaData;
#else
    int serverSocket;
    int clientSocket;
#endif
    
    int port;
    
    // Fonction pour générer un labyrinthe améliorée
    std::vector<std::vector<int>> generateMaze(int rows, int cols, 
                                               int startRow, int startCol,
                                               int endRow, int endCol) {
        // S'assurer que les dimensions sont valides
        rows = std::max(5, std::min(rows, 50));  // Limiter à 50
        cols = std::max(5, std::min(cols, 50));  // Limiter à 50
        
        std::vector<std::vector<int>> maze(rows, std::vector<int>(cols, 0));
        
        std::srand(std::time(nullptr));
        
        // Générer des murs aléatoires (20-30% de chance)
        int wallPercentage = 20 + (std::rand() % 11); // 20-30%
        
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                // Ne pas mettre de mur sur start ou end
                if ((i == startRow && j == startCol) || 
                    (i == endRow && j == endCol)) {
                    maze[i][j] = 0;
                    continue;
                }
                
                // Ajouter des murs sur les bords
                if (i == 0 || i == rows-1 || j == 0 || j == cols-1) {
                    maze[i][j] = 1; // Murs sur les bords
                    continue;
                }
                
                // Éviter les murs trop près du départ/arrivée
                int distToStart = abs(i - startRow) + abs(j - startCol);
                int distToEnd = abs(i - endRow) + abs(j - endCol);
                
                if (distToStart <= 2 || distToEnd <= 2) {
                    maze[i][j] = 0; // Zone libre autour du départ/arrivée
                    continue;
                }
                
                // Chance d'avoir un mur
                if (std::rand() % 100 < wallPercentage) {
                    maze[i][j] = 1;
                }
            }
        }
        
        // Assurer que start et end sont accessibles
        maze[startRow][startCol] = 0;
        maze[endRow][endCol] = 0;
        
        // S'assurer qu'il y a au moins un chemin (simplifié)
        if (startRow > 0) maze[startRow-1][startCol] = 0;
        if (startRow < rows-1) maze[startRow+1][startCol] = 0;
        if (startCol > 0) maze[startRow][startCol-1] = 0;
        if (startCol < cols-1) maze[startRow][startCol+1] = 0;
        
        if (endRow > 0) maze[endRow-1][endCol] = 0;
        if (endRow < rows-1) maze[endRow+1][endCol] = 0;
        if (endCol > 0) maze[endRow][endCol-1] = 0;
        if (endCol < cols-1) maze[endRow][endCol+1] = 0;
        
        return maze;
    }
    
    // Extraire valeur JSON simple
    bool extractIntFromJson(const std::string& json, const std::string& key, int& value) {
        std::string searchKey = "\"" + key + "\":";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return false;
        
        pos += searchKey.length();
        while (pos < json.length() && (json[pos] == ' ' || json[pos] == ':' || json[pos] == '"')) {
            pos++;
        }
        
        size_t end = json.find_first_of(",}\r\n", pos);
        if (end == std::string::npos) return false;
        
        try {
            std::string numStr = json.substr(pos, end - pos);
            // Nettoyer les guillemets
            numStr.erase(std::remove(numStr.begin(), numStr.end(), '"'), numStr.end());
            value = std::stoi(numStr);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error parsing " << key << ": " << e.what() << std::endl;
            return false;
        }
    }
    
    // Extraire l'objet start ou end
    bool extractStartEnd(const std::string& json, const std::string& key, int& row, int& col) {
        std::string searchKey = "\"" + key + "\":";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return false;
        
        // Trouver le début de l'objet
        pos = json.find('{', pos);
        if (pos == std::string::npos) return false;
        
        // Extraire row et col de cet objet
        size_t objEnd = json.find('}', pos);
        if (objEnd == std::string::npos) return false;
        
        std::string obj = json.substr(pos, objEnd - pos + 1);
        
        bool hasRow = extractIntFromJson(obj, "row", row);
        bool hasCol = extractIntFromJson(obj, "col", col);
        
        return hasRow && hasCol;
    }
    
    // Traiter la requête POST /api/maze - CORRIGÉE
    std::string handleMazeRequest(const std::string& requestBody) {
        std::cout << "Handling maze request, body: " << requestBody << std::endl;
        
        int rows = ROWS;
        int cols = COLS;
        int startRow = START_ROW;
        int startCol = START_COL;
        int endRow = END_ROW;
        int endCol = END_COL;
        
        // Extraire les valeurs de base
        extractIntFromJson(requestBody, "rows", rows);
        extractIntFromJson(requestBody, "cols", cols);
        
        // Extraire start
        if (!extractStartEnd(requestBody, "start", startRow, startCol)) {
            std::cout << "Using default start position" << std::endl;
        }
        
        // Extraire end
        if (!extractStartEnd(requestBody, "end", endRow, endCol)) {
            std::cout << "Using default end position" << std::endl;
        }
        
        // Valider les positions
        rows = std::max(5, std::min(rows, 50));
        cols = std::max(5, std::min(cols, 50));
        startRow = std::max(0, std::min(startRow, rows - 1));
        startCol = std::max(0, std::min(startCol, cols - 1));
        endRow = std::max(0, std::min(endRow, rows - 1));
        endCol = std::max(0, std::min(endCol, cols - 1));
        
        std::cout << "Generating maze " << rows << "x" << cols 
                  << " start:(" << startRow << "," << startCol 
                  << ") end:(" << endRow << "," << endCol << ")" << std::endl;
        
        // Générer le labyrinthe
        auto maze = generateMaze(rows, cols, startRow, startCol, endRow, endCol);
        
        // Construire la réponse JSON
        std::ostringstream response;
        response << "{\"success\":true,\"message\":\"Maze generated by C++ backend\",\"grid\":[";
        
        for (size_t i = 0; i < maze.size(); i++) {
            response << "[";
            for (size_t j = 0; j < maze[i].size(); j++) {
                response << maze[i][j];
                if (j < maze[i].size() - 1) response << ",";
            }
            response << "]";
            if (i < maze.size() - 1) response << ",";
        }
        response << "]}";
        
        return response.str();
    }
    
    // Analyser la requête HTTP
    void parseHttpRequest(const std::string& request, 
                          std::string& method, 
                          std::string& path,
                          std::string& body) {
        std::istringstream requestStream(request);
        std::string line;
        
        // Lire la première ligne
        if (std::getline(requestStream, line)) {
            std::istringstream lineStream(line);
            lineStream >> method >> path;
        }
        
        // Chercher le Content-Length
        int contentLength = 0;
        body.clear();
        
        while (std::getline(requestStream, line)) {
            // Nettoyer le \r
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            if (line.find("Content-Length:") == 0) {
                try {
                    contentLength = std::stoi(line.substr(16));
                } catch (...) {
                    contentLength = 0;
                }
            }
            
            if (line.empty()) {
                // Lire le body
                if (contentLength > 0) {
                    std::vector<char> buffer(contentLength + 1);
                    requestStream.read(buffer.data(), contentLength);
                    buffer[contentLength] = '\0';
                    body = buffer.data();
                }
                break;
            }
        }
    }
    
    // Envoyer une réponse HTTP
    void sendHttpResponse(int clientSocket, const std::string& content, 
                         const std::string& contentType = "application/json",
                         int statusCode = 200) {
        std::string statusText;
        switch (statusCode) {
            case 200: statusText = "OK"; break;
            case 400: statusText = "Bad Request"; break;
            case 404: statusText = "Not Found"; break;
            case 500: statusText = "Internal Server Error"; break;
            default: statusText = "OK";
        }
        
        std::string response = 
            "HTTP/1.1 " + std::to_string(statusCode) + " " + statusText + "\r\n" +
            "Content-Type: " + contentType + "\r\n" +
            "Access-Control-Allow-Origin: *\r\n" +
            "Access-Control-Allow-Methods: GET, POST, OPTIONS, PUT, DELETE\r\n" +
            "Access-Control-Allow-Headers: Content-Type, Authorization\r\n" +
            "Access-Control-Max-Age: 86400\r\n" +
            "Content-Length: " + std::to_string(content.length()) + "\r\n" +
            "Connection: close\r\n" +
            "\r\n" + 
            content;
        
#ifdef _WIN32
        send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0);
#else
        send(clientSocket, response.c_str(), response.length(), 0);
#endif
    }
    
public:
    // Constantes pour les valeurs par défaut
    static const int ROWS = 15;
    static const int COLS = 30;
    static const int START_ROW = 7;
    static const int START_COL = 5;
    static const int END_ROW = 7;
    static const int END_COL = 25;
    
    PathfindingServer(int port = 8080) : port(port) {
#ifdef _WIN32
        serverSocket = INVALID_SOCKET;
        clientSocket = INVALID_SOCKET;
#else
        serverSocket = -1;
        clientSocket = -1;
#endif
    }
    
    bool initialize() {
#ifdef _WIN32
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return false;
        }
#endif
        
#ifdef _WIN32
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed" << std::endl;
#ifdef _WIN32
            WSACleanup();
#endif
            return false;
        }
#else
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            std::cerr << "Socket creation failed" << std::endl;
            return false;
        }
#endif
        
        // Configurer le socket
        int opt = 1;
#ifdef _WIN32
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, 
                      reinterpret_cast<char*>(&opt), sizeof(opt)) < 0) {
            std::cerr << "setsockopt failed" << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return false;
        }
#else
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "setsockopt failed" << std::endl;
            close(serverSocket);
            return false;
        }
#endif
        
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) < 0) {
            std::cerr << "Bind failed on port " << port << std::endl;
#ifdef _WIN32
            closesocket(serverSocket);
            WSACleanup();
#else
            close(serverSocket);
#endif
            return false;
        }
        
        if (listen(serverSocket, 10) < 0) {
            std::cerr << "Listen failed" << std::endl;
#ifdef _WIN32
            closesocket(serverSocket);
            WSACleanup();
#else
            close(serverSocket);
#endif
            return false;
        }
        
        std::cout << " Pathfinding Server listening on port " << port << std::endl;
        std::cout << " Ready to accept connections from React frontend" << std::endl;
        return true;
    }
    
    void start() {
        struct sockaddr_in clientAddress;
#ifdef _WIN32
        int clientAddrLen = sizeof(clientAddress);
#else
        socklen_t clientAddrLen = sizeof(clientAddress);
#endif
        
        while (true) {
            std::cout << "\nWaiting for connection..." << std::endl;
            
#ifdef _WIN32
            clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddrLen);
            if (clientSocket == INVALID_SOCKET) {
                std::cerr << " Accept failed" << std::endl;
                continue;
            }
#else
            clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddrLen);
            if (clientSocket < 0) {
                std::cerr << " Accept failed" << std::endl;
                continue;
            }
#endif
            
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN);
            std::cout << " Client connected from: " << clientIP << std::endl;
            
            // Lire la requête
            std::vector<char> buffer(8192);
#ifdef _WIN32
            int bytesRead = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size() - 1), 0);
#else
            int bytesRead = read(clientSocket, buffer.data(), buffer.size() - 1);
#endif
            
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                std::string request(buffer.data(), bytesRead);
                
                std::cout << " Request received (" << bytesRead << " bytes)" << std::endl;
                
                // Parser la requête
                std::string method, path, body;
                parseHttpRequest(request, method, path, body);
                
                std::cout << " Parsed: " << method << " " << path << std::endl;
                
                // Gérer CORS preflight
                if (method == "OPTIONS") {
                    std::cout << " Handling CORS preflight request" << std::endl;
                    sendHttpResponse(clientSocket, "");
                }
                // Routes principales
                else if (path == "/" && method == "GET") {
                    std::string welcome = " Pathfinding Server v1.0\n"
                                        "Endpoints:\n"
                                        "- GET  /           - Server status\n"
                                        "- POST /api/maze   - Generate maze\n"
                                        "- POST /api/visualize - Run algorithms";
                    sendHttpResponse(clientSocket, welcome, "text/plain", 200);
                }
                else if (path == "/api/maze" && method == "POST") {
                    std::cout << " Generating maze..." << std::endl;
                    try {
                        std::string mazeResponse = handleMazeRequest(body);
                        sendHttpResponse(clientSocket, mazeResponse);
                        std::cout << " Maze sent to client" << std::endl;
                    } catch (const std::exception& e) {
                        std::cerr << " Error generating maze: " << e.what() << std::endl;
                        std::string error = "{\"success\":false,\"error\":\"Internal server error: " + 
                                           std::string(e.what()) + "\"}";
                        sendHttpResponse(clientSocket, error, "application/json", 500);
                    }
                }
                else if (path == "/api/visualize" && method == "POST") {
                    std::string algoResponse = "{\"message\":\"Algorithm visualization endpoint\",\"success\":true}";
                    sendHttpResponse(clientSocket, algoResponse);
                }
                else {
                    std::string notFound = "{\"error\":\"Endpoint not found\",\"path\":\"" + path + "\"}";
                    sendHttpResponse(clientSocket, notFound, "application/json", 404);
                    std::cout << " Endpoint not found: " << path << std::endl;
                }
            } else if (bytesRead == 0) {
                std::cout << " Client disconnected" << std::endl;
            } else {
                std::cerr << " Error reading from client" << std::endl;
            }
            
            // Fermer la connexion
#ifdef _WIN32
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
#else
            close(clientSocket);
            clientSocket = -1;
#endif
            
            std::cout << " Connection closed" << std::endl;
        }
    }
    
    ~PathfindingServer() {
#ifdef _WIN32
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
        }
        WSACleanup();
#else
        if (serverSocket >= 0) {
            close(serverSocket);
        }
#endif
        std::cout << "Server shutdown complete" << std::endl;
    }
};

const int PathfindingServer::ROWS;
const int PathfindingServer::COLS;
const int PathfindingServer::START_ROW;
const int PathfindingServer::START_COL;
const int PathfindingServer::END_ROW;
const int PathfindingServer::END_COL;

int main() {
    std::cout << "=========================================" << std::endl;
    std::cout << "     PATHFINDING SERVER v1.0 - C++" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "Maze generation with Dijkstra/BFS support" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    PathfindingServer server(8080);
    
    if (server.initialize()) {
        std::cout << "\nServer initialized successfully!" << std::endl;
        std::cout << " Access URLs:" << std::endl;
        std::cout << "   - Backend: http://localhost:8080" << std::endl;
        std::cout << "   - Frontend: http://localhost:3000" << std::endl;
        std::cout << "\n Waiting for React frontend to connect..." << std::endl;
        
        try {
            server.start();
        } catch (const std::exception& e) {
            std::cerr << "Server error: " << e.what() << std::endl;
            return 1;
        }
    } else {
        std::cerr << " Server initialization failed!" << std::endl;
        return 1;
    }
    
    return 0;
}