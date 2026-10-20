# Smart City Road Network Optimization

A website-style DAA mini-project with a C++ backend. The user only chooses a **source** and **destination**. All locations and road distances are predefined in `server.cpp`.

## What it does
- Uses **Dijkstra's algorithm** to find the shortest route between the selected source and destination.
- Uses **Prim's algorithm** and **Kruskal's algorithm** to find and compare a minimum spanning tree (MST) for the whole city network.
- Displays the predefined weighted graph and highlights the shortest route.
- No user distance entry, graph editor, database, or external library is required.

## Predefined network
Locations: Central, North Park, East Market, West End, University, Airport.

Roads (undirected, distances in km):
- Central — North Park: 4
- Central — East Market: 3
- Central — West End: 7
- North Park — East Market: 2
- North Park — University: 5
- East Market — West End: 4
- East Market — University: 6
- West End — Airport: 5
- University — Airport: 3

## Requirements
- Windows 10/11
- VS Code (or another editor)
- MinGW-w64 / GCC with `g++` added to PATH

Check the compiler in the VS Code terminal:
```powershell
g++ --version
```

## Run
1. Extract the ZIP.
2. Open the `SmartCityMST` folder in VS Code.
3. Run `run.bat` in the terminal, or double-click it.
4. Keep the terminal open.
5. Open `http://localhost:8080` in Chrome/Edge.
6. Select source and destination, then click **Find optimal route**.

The batch file compiles with:
```powershell
g++ -std=c++17 -O2 server.cpp -o server.exe -lws2_32
```

## Expected sample result
For **Central → Airport**, Dijkstra returns a shortest distance of **12 km**. One shortest route is Central → East Market → West End → Airport (3 + 4 + 5 = 12). Another route with the same distance is Central → East Market → University → Airport (3 + 6 + 3 = 12). The MST cost from Prim and Kruskal is **17**.

## DAA complexity
- Dijkstra with adjacency list and priority queue: `O((V + E) log V)`.
- Prim with priority queue: `O((V + E) log V)`.
- Kruskal: `O(E log E)` due to sorting edges.
- Space for graph and algorithm structures: `O(V + E)`.

## Where to change the city map
Edit `LOCATIONS` and `ROADS` near the top of `server.cpp`. Each road `{u, v, w}` connects location index `u` to index `v` with weight/distance `w`. Indexing starts at zero and must match the order in `LOCATIONS`. Recompile by running `run.bat` again.

## Viva note: MST vs shortest path
- **Shortest path (Dijkstra):** best route between the user's selected source and destination.
- **Minimum spanning tree (Prim/Kruskal):** lowest-total-cost set of roads that connects every location.
These are different graph problems, so the project demonstrates both.
