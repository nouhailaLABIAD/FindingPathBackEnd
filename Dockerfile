# ─── Backend: Serve compiled WebAssembly files ────────────────────
FROM python:3.12-slim

WORKDIR /app

# Copy all compiled WASM + JS files
COPY BackendBFS/bfs.js        ./bfs.js
COPY BackendBFS/bfs.wasm      ./bfs.wasm
COPY BackendDijkstra/DijkstraLogique.cpp ./dijkstra_src.cpp
COPY BackendAstar/astar.js    ./astar.js
COPY BackendAstar/astar.wasm  ./astar.wasm
COPY BackendBFS/bfs.js        ./bfs.js
COPY Maze/maze.js             ./maze.js
COPY Maze/maze.wasm           ./maze.wasm

# Simple CORS-enabled HTTP server script
RUN echo '\
import http.server\n\
import socketserver\n\
\n\
class CORSRequestHandler(http.server.SimpleHTTPRequestHandler):\n\
    def end_headers(self):\n\
        self.send_header("Access-Control-Allow-Origin", "*")\n\
        self.send_header("Access-Control-Allow-Methods", "GET, OPTIONS")\n\
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")\n\
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")\n\
        super().end_headers()\n\
    def guess_type(self, path):\n\
        if path.endswith(".wasm"):\n\
            return "application/wasm"\n\
        return super().guess_type(path)\n\
\n\
PORT = 8080\n\
with socketserver.TCPServer(("", PORT), CORSRequestHandler) as httpd:\n\
    print(f"Serving WASM backend on port {PORT}")\n\
    httpd.serve_forever()\n\
' > server.py

EXPOSE 8080
CMD ["python", "server.py"]