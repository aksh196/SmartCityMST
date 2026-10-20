#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <numeric>
#include <limits>
#include <tuple>
#include <cstdio>
#include <functional>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

struct Edge
{
    int u, v, w;
};

struct TreeResult
{
    vector<Edge> edges;
    long long cost = 0;
};

struct RouteResult
{
    vector<int> path;
    long long distance = -1;
};

const vector<string> LOCATIONS = {
    "Central", "North Park", "East Market",
    "West End", "University", "Airport"};

const vector<Edge> ROADS = {
    {0, 1, 4}, {0, 2, 3}, {0, 3, 7}, {1, 2, 2}, {1, 4, 5}, {2, 3, 4}, {2, 4, 6}, {3, 5, 5}, {4, 5, 3}};

string readFile(const string &path)
{
    FILE *f = fopen(path.c_str(), "rb");

    if (!f)
        return "";

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    string content(size > 0 ? size : 0, '\0');

    if (size > 0)
        fread(&content[0], 1, size, f);

    fclose(f);
    return content;
}

string jsonEscape(const string &s)
{
    string out;

    for (size_t i = 0; i < s.size(); i++)
    {
        unsigned char c = (unsigned char)s[i];

        if (c == '"' || c == '\\')
        {
            out += '\\';
            out += (char)c;
        }
        else if (c == '\n')
            out += "\\n";
        else if (c == '\r')
            out += "\\r";
        else if (c == '\t')
            out += "\\t";
        else if (c >= 32)
            out += (char)c;
    }

    return out;
}

struct DSU
{
    vector<int> parent, rankv;

    DSU(int n) : parent(n), rankv(n, 0)
    {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x)
    {
        if (parent[x] != x)
            parent[x] = find(parent[x]);

        return parent[x];
    }

    bool unite(int a, int b)
    {
        a = find(a);
        b = find(b);

        if (a == b)
            return false;

        if (rankv[a] < rankv[b])
            swap(a, b);

        parent[b] = a;

        if (rankv[a] == rankv[b])
            rankv[a]++;

        return true;
    }
};

TreeResult primMST()
{
    int n = (int)LOCATIONS.size();

    vector<vector<pair<int, int>>> adj(n);

    for (size_t i = 0; i < ROADS.size(); i++)
    {
        Edge e = ROADS[i];

        adj[e.u].push_back(make_pair(e.v, e.w));
        adj[e.v].push_back(make_pair(e.u, e.w));
    }

    vector<bool> seen(n, false);

    typedef tuple<int, int, int> PQNode;

    priority_queue<
        PQNode,
        vector<PQNode>,
        greater<PQNode>>
        pq;

    TreeResult r;

    seen[0] = true;

    for (size_t i = 0; i < adj[0].size(); i++)
    {
        int v = adj[0][i].first;
        int w = adj[0][i].second;

        pq.push(make_tuple(w, 0, v));
    }

    while (!pq.empty())
    {
        PQNode node = pq.top();
        pq.pop();

        int w = get<0>(node);
        int u = get<1>(node);
        int v = get<2>(node);

        if (seen[v])
            continue;

        seen[v] = true;

        Edge e;
        e.u = u;
        e.v = v;
        e.w = w;

        r.edges.push_back(e);
        r.cost += w;

        for (size_t i = 0; i < adj[v].size(); i++)
        {
            int next = adj[v][i].first;
            int weight = adj[v][i].second;

            if (!seen[next])
                pq.push(make_tuple(weight, v, next));
        }
    }

    return r;
}

TreeResult kruskalMST()
{
    vector<Edge> sorted = ROADS;

    sort(sorted.begin(), sorted.end(),
         [](const Edge &a, const Edge &b)
         {
             return a.w < b.w;
         });

    DSU d((int)LOCATIONS.size());
    TreeResult r;

    for (size_t i = 0; i < sorted.size(); i++)
    {
        Edge e = sorted[i];

        if (d.unite(e.u, e.v))
        {
            r.edges.push_back(e);
            r.cost += e.w;
        }
    }

    return r;
}

RouteResult dijkstra(int source, int destination)
{
    int n = (int)LOCATIONS.size();

    vector<vector<pair<int, int>>> adj(n);

    for (size_t i = 0; i < ROADS.size(); i++)
    {
        Edge e = ROADS[i];

        adj[e.u].push_back(make_pair(e.v, e.w));
        adj[e.v].push_back(make_pair(e.u, e.w));
    }

    const long long INF = numeric_limits<long long>::max() / 4;

    vector<long long> dist(n, INF);
    vector<int> parent(n, -1);

    typedef pair<long long, int> DistanceNode;

    priority_queue<
        DistanceNode,
        vector<DistanceNode>,
        greater<DistanceNode>>
        pq;

    dist[source] = 0;
    pq.push(make_pair(0LL, source));

    while (!pq.empty())
    {
        DistanceNode node = pq.top();
        pq.pop();

        long long d = node.first;
        int u = node.second;

        if (d != dist[u])
            continue;

        if (u == destination)
            break;

        for (size_t i = 0; i < adj[u].size(); i++)
        {
            int v = adj[u][i].first;
            int w = adj[u][i].second;

            if (d + w < dist[v])
            {
                dist[v] = d + w;
                parent[v] = u;

                pq.push(make_pair(dist[v], v));
            }
        }
    }

    RouteResult r;

    if (dist[destination] == INF)
        return r;

    r.distance = dist[destination];

    for (int at = destination; at != -1; at = parent[at])
        r.path.push_back(at);

    reverse(r.path.begin(), r.path.end());

    return r;
}

string edgeJson(const Edge &e)
{
    ostringstream o;

    o << "{\"u\":" << e.u
      << ",\"v\":" << e.v
      << ",\"w\":" << e.w
      << ",\"from\":\"" << jsonEscape(LOCATIONS[e.u])
      << "\",\"to\":\"" << jsonEscape(LOCATIONS[e.v])
      << "\"}";

    return o.str();
}

string treeJson(const TreeResult &r)
{
    ostringstream o;

    o << "{\"cost\":" << r.cost << ",\"edges\":[";

    for (size_t i = 0; i < r.edges.size(); i++)
    {
        if (i)
            o << ",";

        o << edgeJson(r.edges[i]);
    }

    o << "]}";

    return o.str();
}

string dataJson()
{
    ostringstream o;

    o << "{\"locations\":[";

    for (size_t i = 0; i < LOCATIONS.size(); i++)
    {
        if (i)
            o << ",";

        o << "\"" << jsonEscape(LOCATIONS[i]) << "\"";
    }

    o << "],\"roads\":[";

    for (size_t i = 0; i < ROADS.size(); i++)
    {
        if (i)
            o << ",";

        o << edgeJson(ROADS[i]);
    }

    o << "],\"prim\":" << treeJson(primMST())
      << ",\"kruskal\":" << treeJson(kruskalMST())
      << "}";

    return o.str();
}

string routeJson(int source, int destination)
{
    int n = (int)LOCATIONS.size();

    if (source < 0 || destination < 0 ||
        source >= n || destination >= n)
    {
        return "{\"error\":\"Please select valid source and destination locations.\"}";
    }

    if (source == destination)
        return "{\"error\":\"Source and destination must be different.\"}";

    RouteResult r = dijkstra(source, destination);

    if (r.distance < 0)
        return "{\"error\":\"No route exists between these locations.\"}";

    ostringstream o;

    o << "{\"source\":\"" << jsonEscape(LOCATIONS[source])
      << "\",\"destination\":\"" << jsonEscape(LOCATIONS[destination])
      << "\",\"distance\":" << r.distance
      << ",\"path\":[";

    for (size_t i = 0; i < r.path.size(); i++)
    {
        if (i)
            o << ",";

        int id = r.path[i];

        o << "{\"id\":" << id
          << ",\"name\":\"" << jsonEscape(LOCATIONS[id])
          << "\"}";
    }

    o << "],\"prim\":" << treeJson(primMST())
      << ",\"kruskal\":" << treeJson(kruskalMST())
      << "}";

    return o.str();
}

string getMime(const string &p)
{
    if (p.size() >= 5 && p.substr(p.size() - 5) == ".html")
        return "text/html; charset=utf-8";

    if (p.size() >= 4 && p.substr(p.size() - 4) == ".css")
        return "text/css; charset=utf-8";

    if (p.size() >= 3 && p.substr(p.size() - 3) == ".js")
        return "application/javascript; charset=utf-8";

    return "text/plain; charset=utf-8";
}

void sendResponse(
    SOCKET c,
    int status,
    const string &statusText,
    const string &type,
    const string &body)
{
    ostringstream o;

    o << "HTTP/1.1 " << status << " " << statusText
      << "\r\nContent-Type: " << type
      << "\r\nContent-Length: " << body.size()
      << "\r\nCache-Control: no-store"
      << "\r\nConnection: close\r\n\r\n"
      << body;

    string data = o.str();

    size_t sent = 0;

    while (sent < data.size())
    {
        int result = send(
            c,
            data.c_str() + sent,
            (int)(data.size() - sent),
            0);

        if (result == SOCKET_ERROR || result == 0)
            break;

        sent += result;
    }
}

void handleClient(SOCKET client)
{
    string req;
    char buf[8192];
    int got;

    while ((got = recv(client, buf, sizeof(buf), 0)) > 0)
    {
        req.append(buf, got);

        size_t h = req.find("\r\n\r\n");

        if (h != string::npos)
        {
            size_t len = 0;
            string headers = req.substr(0, h);

            size_t p = headers.find("Content-Length:");

            if (p != string::npos)
            {
                p += 15;

                while (p < headers.size() && headers[p] == ' ')
                    p++;

                try
                {
                    len = (size_t)stoul(headers.substr(p));
                }
                catch (...)
                {
                    sendResponse(
                        client, 400, "Bad Request",
                        "application/json",
                        "{\"error\":\"Invalid Content-Length\"}");
                    return;
                }
            }

            if (req.size() >= h + 4 + len)
                break;
        }

        if (req.size() > 1024 * 1024)
        {
            sendResponse(
                client, 413, "Payload Too Large",
                "application/json",
                "{\"error\":\"Request too large\"}");
            return;
        }
    }

    size_t le = req.find("\r\n");

    if (le == string::npos)
    {
        sendResponse(
            client, 400, "Bad Request",
            "application/json",
            "{\"error\":\"Bad request\"}");
        return;
    }

    istringstream first(req.substr(0, le));

    string method, path, version;
    first >> method >> path >> version;

    size_t query = path.find('?');

    if (query != string::npos)
        path = path.substr(0, query);

    if (method == "GET" && path == "/api/data")
    {
        sendResponse(client, 200, "OK", "application/json", dataJson());
        return;
    }

    if (method == "POST" && path == "/api/route")
    {
        size_t h = req.find("\r\n\r\n");

        if (h == string::npos)
        {
            sendResponse(
                client, 400, "Bad Request",
                "application/json",
                "{\"error\":\"Missing body\"}");
            return;
        }

        istringstream body(req.substr(h + 4));

        int s, d;

        if (!(body >> s >> d))
        {
            sendResponse(
                client, 400, "Bad Request",
                "application/json",
                "{\"error\":\"Send source and destination indexes\"}");
            return;
        }

        string ans = routeJson(s, d);
        bool hasError = ans.find("\"error\"") != string::npos;

        sendResponse(
            client,
            hasError ? 400 : 200,
            hasError ? "Bad Request" : "OK",
            "application/json",
            ans);

        return;
    }

    if (method == "GET")
    {
        if (path == "/")
            path = "/index.html";

        if (path.find("..") != string::npos)
        {
            sendResponse(client, 403, "Forbidden", "text/plain", "Forbidden");
            return;
        }

        string file = readFile(path.substr(1));

        if (file.empty())
        {
            sendResponse(
                client, 404, "Not Found",
                "text/plain",
                "File not found. Run server.exe from the project folder.");
            return;
        }

        sendResponse(client, 200, "OK", getMime(path), file);
        return;
    }

    sendResponse(
        client, 404, "Not Found",
        "application/json",
        "{\"error\":\"Route not found\"}");
}

int main()
{
    WSADATA w;

    if (WSAStartup(MAKEWORD(2, 2), &w) != 0)
    {
        cerr << "Could not initialize Windows sockets.\n";
        return 1;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s == INVALID_SOCKET)
    {
        cerr << "Could not create server socket.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in a = {};

    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.sin_port = htons(8080);

    if (bind(s, (sockaddr *)&a, sizeof(a)) == SOCKET_ERROR)
    {
        cerr << "Port 8080 is busy. Close the other app using it and retry.\n";
        closesocket(s);
        WSACleanup();
        return 1;
    }

    if (listen(s, SOMAXCONN) == SOCKET_ERROR)
    {
        cerr << "Could not start listening.\n";
        closesocket(s);
        WSACleanup();
        return 1;
    }

    cout << "Smart City Road Network server running at http://localhost:8080\n";
    cout << "Predefined roads are stored in server.cpp.\n";
    cout << "Keep this terminal open. Press Ctrl+C to stop.\n";

    while (true)
    {
        SOCKET c = accept(s, NULL, NULL);

        if (c == INVALID_SOCKET)
            continue;

        handleClient(c);

        shutdown(c, SD_SEND);
        closesocket(c);
    }

    closesocket(s);
    WSACleanup();

    return 0;
}
