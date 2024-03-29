// AED 2021/2022 - Aula Pratica 11
// Pedro Ribeiro (DCC/FCUP) [17/01/2022]

#include "graph.h"
#include <climits>
#include <limits>
#include <chrono>
#include <set>
#include <map>

#define INF (INT_MAX/2)

// Constructor: nr nodes and direction (default: undirected)
Graph::Graph(int num, bool dir) : n(num), hasDir(dir), nodes(num+1) {

    for (int i = 1; i<= n; i++) nodes[i].id = i;
}

// Add edge from source to destination with a certain weight
void Graph::addEdge(int src, int dest, int capacity, int flow, int time, int resCap) {
    if (src<1 || src>n || dest<1 || dest>n) return;
    nodes[src].adj.push_back({src, dest, capacity, time, flow, resCap});
    nodes[dest].res.push_back({src, 0, time});
    if (!hasDir) nodes[dest].adj.push_back({src, capacity, time});  //src to dest?
}


//to remove
void Graph::print() const {
    int edgeCount = 0;
    for(int i=1;i<=n;i++){
        std::cout << "src: " << i << ":\n";
        for(auto e : nodes[i].adj){
            std::cout << "\tdest: " << e.dest
                      << "\n\t\tcapacity: " << e.capacity
                      << "\n\t\tduration: " << e.time
                      << std::endl;
            edgeCount++;
        }
        std::cout << "-residual: \n";
        for(auto e : nodes[i].res){
            std::cout << "\tdest: " << e.dest
                      << "\n\t\tcapacity: " << e.capacity
                      << "\n\t\tduration: " << e.time
                      << std::endl;
        }
    }
    std::cout << "\nNodes: " << n
              << "\nEdges: " << edgeCount
              << std::endl;
}

void Graph::dijkstra(int s) {
    MinHeap<int, int> q(n, -1);
    for (int v=1; v<=n; v++) {
        nodes[v].dist = INF;
        nodes[v].pred = 0;
        q.insert(v, nodes[v].dist);
        nodes[v].visited = false;
    }
    nodes[s].dist = 0;
    q.decreaseKey(s, 0);
    nodes[s].pred = s;
    while (q.getSize()>0) {
        int u = q.removeMin();
        //cout << "Node " << u << " with dist = " << nodes[u].dist << endl;
        nodes[u].visited = true;
        for (auto e : nodes[u].adj) {
            int v = e.dest;
            if (!nodes[v].visited && nodes[u].dist + 1 < nodes[v].dist && e.resCap > 0) {
                nodes[v].dist = nodes[u].dist + 1;
                q.decreaseKey(v, nodes[v].dist);
                nodes[v].pred = u;
            }
        }
    }
}


void Graph::maximumFlowPath(int s) {

    MaxHeap<int, int> q(n, -1);

    for (int v=1; v<=n; v++) {
        //nodes[v].dist = INF;
        nodes[v].pred = 0;
        nodes[v].capacity = 0;
    }

    nodes[s].capacity = INF;

    for (int v=1; v<=n; v++) {
        q.insert(v, nodes[v].capacity);
    }

    while (q.getSize()>0) {
        int u = q.removeMax();
        //cout << "Node " << u << " with dist = " << nodes[u].dist << endl;
        for (auto e : nodes[u].adj) {
            int v = e.dest;
            int mincap = min(nodes[u].capacity, e.capacity);
            if (mincap > nodes[v].capacity) {
                nodes[v].capacity = mincap;
                nodes[v].pred = u;
                q.increaseKey(v, nodes[v].capacity);
            }
        }
    }

}

vector<int> Graph::get_path(int a, int b, bool print) {


    list<int> path;
    int tmp;
    if(!nodes[b].visited){
        vector<int> pathv;
        return pathv;
    }
    path.push_back(b);
    int v = b;
    while (v != a) {
        if(v == 0){
            vector<int> pathv;
            return pathv;
        }
        tmp = nodes[v].pred;
        v = tmp;
        // POSSIVELMENTE ALTERAR -----------------------------------------------------------------------
        path.push_front(v); // IMPORTANTE FAZER PUSH_FRONT
    }
    vector<int> pathv;
    for (int k: path) {
        pathv.push_back(k);
    }

    if (print) {for(auto elem : pathv) cout << "[" << elem << "] " ;}

    cout << endl;


    return pathv;

}

int Graph::edmondKarpFlux(int start, int end) {
    Node start_Node;
    std::vector<int> path;
    int resCap = INF;
    Graph resGrid = Graph(n, true);

    reset_Flux();

    //determine residual grid
    resGrid = resGraph();
    resGrid.BFS(start, end);
    path = resGrid.get_path(start, end, true);
    Node destination = resGrid.nodes[end];

    //while there is a path in the Residual Grid
    while(destination.visited ){

        //find minimun Cf in path
        for(int i = 0; i < path.size()-1; i++){
            for(Edge edge : (resGrid.nodes[path[i]].adj)) {
                //found the edge of the path
                if(nodes[edge.dest].id == resGrid.nodes[path[i+1]].id) {
                    resCap = std::min(edge.resCap, resCap);
                }
            }
        }

        for(int i = 0; i < path.size()-1; i++){
            for(Edge &edge: nodes[path[i]].adj){
                //found the edge of the path
                if(nodes[edge.dest].id == nodes[path[i+1]].id){
                    edge.flow = resCap + edge.flow;
                    edge.resCap = edge.capacity - edge.flow;
                    if(edge.resCap < 0){
                        edge.resCap = 0;
                    }
                }
            }
        }

        //determine residual grid
        resGrid = resGraph();
        resGrid.BFS(start, end);
        if (destination.visited) {
            path = resGrid.get_path(start, end, true);
        }
        else {path = resGrid.get_path(start, end, false);}
        destination = resGrid.nodes[end];
        resCap = INF;
    }

    start_Node = nodes[start];
    int maxFlux = 0;
    for(Edge edge: start_Node.adj){
        maxFlux+= edge.flow;
    }

    cout << "Maximum flow obtained was: " << maxFlux << endl;

    return maxFlux;
}


Graph Graph::resGraph() {
    Graph residualGrid = Graph(n, true);
    //add all vertexes
    for(const auto& node : nodes){
        Node newNode = node;
        newNode.adj = {};
        newNode.visited = false;
        residualGrid.nodes.push_back(newNode);

    }
    for(Node& node : nodes){
        for(Edge& edge : node.adj){
            //Cf(u,v)
            if(edge.capacity - edge.flow > 0) {
                residualGrid.addEdge(node.id, nodes[edge.dest].id, edge.capacity, edge.flow, edge.time,
                                     edge.capacity - edge.flow);
            }
        }
    }

    for(Node& node : nodes){
        for(Edge& edge : node.adj){
            //Cf(v,u)
            // É mesmo suposto ser ao contrário
            if(edge.flow > 0) {
                int f = edge.flow;
                residualGrid.addEdge(nodes[edge.dest].id, node.id, edge.capacity, 0, edge.time, f);
            }
        }
    }
    return residualGrid;
}


int Graph::correctGroupSize(int start, int end, int increment, bool correct) {

    Node start_Node;
    std::vector<int> path;
    int resCap = INF;
    Graph resGrid = Graph(n, true);
    int startFlow = 0;
    int endFlow = 0;
    if(!correct){
        reset_Flux();
    }
    else{
        for(auto e : nodes[start].adj){
            startFlow += e.flow;
        }
    }

    endFlow = startFlow;

    int incrementTemp = increment;



    //determine residual grid
    resGrid = resGraph();
    resGrid.BFS(start, end);
    path = resGrid.get_path(start, end, true);
    Node dest = resGrid.nodes[end];

    //while there is a path in the Residual Grid
    while(dest.visited && (endFlow - startFlow < increment)){

        //find minimun Cf in path
        for(int i = 0; i < path.size()-1; i++){
            for(Edge edge : (resGrid.nodes[path[i]].adj)) {
                //found the edge of the path
                if(nodes[edge.dest].id == resGrid.nodes[path[i+1]].id) {
                    resCap = std::min(edge.resCap, resCap);
                }
            }
        }

        for(int i = 0; i < path.size()-1; i++){
            for(Edge &edge: nodes[path[i]].adj){
                //found the edge of the path
                if(nodes[edge.dest].id == nodes[path[i+1]].id){
                    int tempFlow = 0;
                    for(auto e : nodes[start].adj){
                        tempFlow += e.flow;
                    }
                    if(resCap > incrementTemp){
                        edge.flow = incrementTemp + edge.flow;
                    }
                    else{
                        edge.flow = resCap + edge.flow;
                    }
                    edge.resCap = edge.capacity - edge.flow;
                    if(edge.resCap < 0){
                        edge.resCap = 0;
                    }
                }
            }
        }

        incrementTemp -= resCap;

        //determine residual grid
        resGrid = resGraph();
        resGrid.BFS(start, end);

        resCap = INF;
        dest = resGrid.nodes[end];

        start_Node = nodes[start];
        endFlow = 0;
        for(Edge edge: start_Node.adj){
            endFlow+= edge.flow;
        }
        if (dest.visited && (endFlow - startFlow < increment)) {
            path = resGrid.get_path(start, end, true);
        }
        else {
            path = resGrid.get_path(start, end, false);
        }
    }



    if(endFlow - startFlow < increment){
        if (correct) {
            cout << "Group size exceeds capacity of the buses" << endl;
            return -1;
        }
        return endFlow;
    }

    return endFlow - startFlow;
}

//sets all fluxes to 0
void Graph::reset_Flux() {
    for(auto& i : nodes){
        for(auto itr = i.adj.begin(); itr != i.adj.end() ; itr++){
            itr->flow = 0;
        }
    }
}

void Graph::BFS(int a, int b) {

    // initialize all nodes as unvisited
    for (int v=1; v<=n; v++) nodes[v].visited = false;
    queue<int> q; // queue of unvisited nodes
    q.push(a);
    nodes[a].dist=0;
    nodes[a].visited=true;
    while (!q.empty()) { // while there are still unvisited nodes
        int u = q.front(); q.pop();
        for (auto e : nodes[u].adj) {
            int w = e.dest;
            if (!nodes[w].visited && e.resCap > 0) {
                q.push(w);
                nodes[w].pred = u;
                nodes[w].visited = true;
                //nodes[w].dist = nodes[u].dist + e.weight;
                nodes[w].dist = nodes[u].dist + 1;
            }
            if(w==b) {
                break;
            }
        }
    }

}

int Graph::earliestStart() {
    int minDuration = -1;
    int vf = 0;

    queue<int> S;

    for (int i = 1; i<=n; i++) {
        nodes[i].pred = 0;
        nodes[i].ES = 0;
        nodes[i].EF = 0;
        nodes[i].eDeg = 0;
    }
    for (int i = 1; i<=n; i++) {
        for (Graph::Edge edge : nodes[i].adj) {
            nodes[edge.dest].eDeg += 1;
        }
    }
    for (int i = 1; i<=n; i++) {
        if (nodes[i].eDeg == 0) S.push(i);
    }

    while(!S.empty()) {
        int v = S.front();
        S.pop();

        if (minDuration < nodes[v].ES) {
            minDuration = nodes[v].ES;
            vf = v;
        }

        for (auto k : nodes[v].adj) {
            if (nodes[k.dest].ES < nodes[v].ES + k.time) {
                nodes[k.dest].ES = nodes[v].ES + k.time;
                nodes[k.dest].pred = v;
            }

            nodes[k.dest].eDeg = nodes[k.dest].eDeg - 1;

            if (nodes[k.dest].eDeg == 0) {
                S.push(k.dest);
            }
        }
    }

   /*cout << minDuration << endl;*/

    return minDuration;
}

void Graph::topSort(int start, std::stack<int> &stack) {

    nodes[start].visited = true;

    for (Edge edge : nodes[start].adj) {
        if (!(nodes[edge.dest].visited) && (edge.flow != 0)) topSort(edge.dest, stack);
    }
    stack.push(start);
}

int Graph::longestPath(int start, int end) {

    auto init = chrono::steady_clock::now();

    if (start < 0 || end > n) {
        cout << "Values of start or end out of bounds" << endl;
        return 0;
    }

    edmondKarpFlux(start, end);

    int NINF = std::numeric_limits<int>::min();

    std::stack<int> aux_stack;

// init all nodes as non-visited
    for (int i = 1; i <= n; i++) {
        nodes[i].visited = false;
        nodes[i].pred = -1;
    }
// store topo order
    for (int i = 1; i <= n; i++) if (!nodes[i].visited) topSort(i, aux_stack);
// set all distances to inf
    for (int i = 1; i <= n; i++) nodes[i].dist = NINF;

    nodes[start].dist = 0;

    int dest;

    while (!aux_stack.empty()) {
        int node = aux_stack.top();
        aux_stack.pop();

        // adj

        if (nodes[node].dist != NINF) {
            for (Edge edge: nodes[node].adj) {
                if (edge.flow != 0) {
                    dest = edge.dest;

                    if (nodes[dest].dist < nodes[node].dist + edge.time) {
                        nodes[dest].dist = nodes[node].dist + edge.time;
                        nodes[dest].pred = node;
                    }
                }
            }
        }
    }

    int teste = end;
    stack<int> aux;
    while (nodes[teste].pred != -1) {
        aux.push(teste);
        teste = nodes[teste].pred;
    }
    aux.push(teste);

    cout << endl << "Longest path: ";
    while (aux.size() > 1) {
        cout << "[" << aux.top() << "]" << "->";
        aux.pop();
    }
    cout << "[" << aux.top() << "]" << endl;

    cout << "Minimum time waiting for everyone at the end: " << nodes[end].dist << " units." << endl;

    auto final = chrono::steady_clock::now();

    cout << "Algorithm execution time: " << chrono::duration_cast<chrono::milliseconds>(final - init).count()
         << " (milli seconds)." << endl;

    return nodes[end].dist;
}

void Graph::latestFinish(int minDur) {

    int minDuration = minDur;

    for (int i = 1; i <= n; i++) {
        nodes[i].LF = minDuration;
        nodes[i].sDeg = 0;
    }

    for (int i = 1; i <= n; i++) {
        for (Edge edge : nodes[i].adj) {
            nodes[edge.dest].sDeg += 1;
        }
    }
    // Supostamente isto é o graph transposto G^T
    Graph transposed = transposeGraph();
    // Não percebi qual é a estrutura, pois o V à frente vai ser
    queue<int> S;

    for (int i = 1; i <= n; i++) if (transposed.nodes[i].sDeg == 0) S.push(i);

    while (!S.empty()) {
        int v = S.front();
        S.pop();

            for (auto k : transposed.nodes[v].adj) {
                if (transposed.nodes[k.dest].LF > (transposed.nodes[v].LF - k.time)) {
                    transposed.nodes[k.dest].LF = transposed.nodes[v].LF - k.time;
                    transposed.nodes[k.dest].LS = transposed.nodes[v].LF - k.time;
                    nodes[k.dest].LF = nodes[v].LF - k.time;
                    nodes[k.dest].LS = nodes[v].LF - k.time;
                }
                transposed.nodes[k.dest].sDeg = transposed.nodes[k.dest].sDeg - 1;

                if (nodes[k.dest].sDeg == 0) S.push(k.dest);
       }
    }
}

void Graph::node_wait_times(int start, int end) {

    cout << "All paths used: " << endl;
    edmondKarpFlux(start, end);
    int minD = earliestStart();
    latestFinish(minD);

    int maxDuration = 0, count_nodes = 0;

    for (int i = 1; i <= n; i++) {
        for (Edge edge : nodes[i].adj) {
            if (edge.flow != 0) {
                if ((nodes[edge.dest].LF - nodes[edge.dest].ES) != 0) {
                    if ((nodes[edge.dest].LF - nodes[edge.dest].ES) > maxDuration) {
                        maxDuration = (nodes[i].LF - nodes[i].ES);
                    }


                    nodes[edge.dest].visited = true;
                }
            }

        }
    }

    std::map<int, int> biggest;


    for (int i = 1; i <= n; i++) {
        for (Edge edge: nodes[i].adj) {
            if (edge.flow != 0) {
                if ((nodes[edge.dest].LF - nodes[edge.dest].ES) == maxDuration) {
                    count_nodes++;
                    if (biggest.find(edge.dest) != biggest.end()) {
                        biggest.find(edge.dest)->second++;
                    }
                    else {
                        biggest.insert(make_pair(edge.dest, 1));
                    }
                }
            }
        }
    }

    cout << endl;
    for (auto l : biggest) {
        cout << "Largest waiting time happened on node: " << l.first << ", " << l.second << " time(s)." << endl;
    }

    cout << endl;
    std::cout << "The largest waiting time was: " << maxDuration << ". It happened: " << count_nodes << " time(s)." << endl;

}

int Graph::path_Capacity(const vector<int>& path) {

    /*vector<int> pathv;
    for (int const &k: path) {
        pathv.push_back(k);
    }*/

    int capacity = INF;
    for(int i = 0; i < path.size(); i++){
        for(auto e : nodes[path[i]].adj){
            if(e.dest==path[i+1])
                capacity=min(capacity, e.capacity);
        }
    }
    return capacity;
}

Graph Graph::transposeGraph() {

    Graph tGraph = Graph(n, true);
    int num_node = 0;

    for (const auto& k : nodes) {
        for (auto e : k.adj) {
            tGraph.addEdge(e.dest, num_node, e.capacity, e.flow, e.time);  //change init order
        }
        num_node++;
    }

    return tGraph;
}
