#include "maze.h"

Maze::Maze()
{
}

void Maze::search(QString algorithm) {
    resetMaze();
    if(algorithm == "Breadth-First") {
        QFuture<Node*> future = QtConcurrent::run(this, &Maze::bfs);
    }
    else if(algorithm == "Depth-First") {
        QFuture<Node*> future = QtConcurrent::run(this, &Maze::dfs);
    }
    else if(algorithm == "Greedy Best-First") {
        QFuture<Node*> future = QtConcurrent::run(this, &Maze::gbfs);
    }
    else if(algorithm == "Iterative Deepening") {
        QFuture<Node*> future = QtConcurrent::run(this, &Maze::iddfs);
    }
//    QFuture<Node*> future = QtConcurrent::run(this, &Maze::gbfs);
//    Node* returned = future.result();
//    Node* node = bfs();
//    qDebug() << "Returned Node:" << returned->getX() << returned->getY();
}

QString Maze::getColor(int x, int y) {
    //find the state of the node from x and y
    auto nodeObj = *std::find_if(
      nodes.begin(), nodes.end(),
      [&x, &y](Node *node) { return (node->getX() == x && node->getY() == y); }
    );
    //if the node was found
    if (nodeObj != *nodes.end())
    {
        //return a color based on type
        if(nodeObj->getType() == "goal") {
            return "#70EC84";
        }
        else if(nodeObj->getType() == "obstruction") {
            return "#C9B2A6";
        }
        else if(nodeObj->getType() == "agent") {
            return "#f44242";
        }
        else if(nodeObj->getType() == "neighbour") {
            return "#f4ce42";
        }
        else if (nodeObj->getType() == "used") {
            return "#c4c4c4";
        }
        else if(nodeObj->getType() == "path") {
            return "#98bdf0";
        }
        else {
            return "#ffffff";
        }
    }
    //if the node wasnt found
    else
    {
      qDebug() << "Matching node not found";
    }
}

//Lambda Function which finds associated properties in a model
QVariant Maze::findInModel(QList<QObject*> model, std::function<bool(QObject* obj)> const& criteria) {
    for(int i = 0; i < model.count(); ++i)
        if (criteria(model[i]))
            return QVariant::fromValue(model[i]);
    return QVariant::fromValue(NULL);
}

Node* Maze::findNode(int x, int y) {
    //find the matching node and return it
    auto nodeObj = *std::find_if(
      nodes.begin(), nodes.end(),
      [&x, &y](Node *node) { return ((node->getX() == x) && (node->getY() == y)); }
    );
    return nodeObj;
}

void Maze::setType(int x, int y, QString type) {
    //find the matching node and return it
    auto nodeObj = *std::find_if(
      nodes.begin(), nodes.end(),
      [&x, &y](Node *node) { return ((node->getX() == x) && (node->getY() == y)); }
    );
    //if the node was found
    if (nodeObj != *nodes.end())
    {
        //update the node state
        nodeObj->setType(type);
    }
    //if the node wasnt found
    else
    {
      qDebug() << "Matching node not found";
    }
    qDebug() << "Node at" << nodeObj->getX() << nodeObj->getY() << "set to" << nodeObj->getType();
}

//return true if the node is not an obstacle
bool Maze::isValid(int x, int y) {
    //if the cell is within the maze
    if (!(x >= nCol || x < 0 || y >= nRow || y < 0)) {
        //if the cell isnt an obstacle
        if(!(findNode(x, y)->getType() == "obstruction")) {
            return true;
        }
        return false;
    }
    return false;
}

//return true if the current position is a destination node
bool Maze::isDestination(int x, int y) {
    if(findNode(x, y)->getType() == "goal") {
        return true;
    }
    return false;
}

//calculate the straight line distance from current location to destination
double Maze::calculateH(Node* source) {
    //initialize a large hCost to compare against
    double hCost = 999999;

    //loop through all the nodes to find a goal node
    for (int i = 0; i < nodes.size(); i++) {
        if(nodes[i]->getType() == "goal") {
            //calculate the heuristic cost to the goal node
            double hTemp = (sqrt((source->getX() - nodes[i]->getX())*(source->getX() - nodes[i]->getX())
                + (source->getY() - nodes[i]->getY())*(source->getY() - nodes[i]->getY())));
            //find the smallest hCost to any goal node
            if(hTemp < hCost) {
                hCost = hTemp;
            }
        }
    }

    return hCost;
}

//reset maze back to original state
void Maze::resetMaze() {
    //clear all node types
    for (int i = 0; i < nodes.size(); i++) {
        nodes[i]->setType("");
    }
    //read original types from file
    readMazeFile();
}

void Maze::readMazeFile() {
    //open the file
    QFile inFile(fileName);
    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

    //read the maze size
    readRowCol(inFile.readLine().constData());
    readInitialState(inFile.readLine().constData());
    readGoalStates(inFile.readLine());
    //read all the obstructions
    while(!inFile.atEnd()) {
        readObstructionNodes(inFile.readLine().constData());
    }
    emit nColChanged();
    emit nRowChanged();
    emit nodesChanged();
    emit agentChanged();
    inFile.close();
}

void Maze::readRowCol(std::string str) {
    //split out the number of rows and columns
    unsigned first = str.find("[")+1;
    unsigned last = str.find("]");
    std::string subDimensionString = str.substr (first,last-first);
    QStringList splitDimension = QString::fromStdString(subDimensionString).split(QRegExp("(\\,)"));

    //set the number of rows and columns
    setNRow(splitDimension.at(0).toInt());
    setNCol(splitDimension.at(1).toInt());

    //create all the nodes
    for (int x = 0; x < getNCol(); x++) {
        for (int y = 0; y < getNRow(); y++) {
            Node *node = new Node(x, y);
            nodes.append(node);
        }
    }
    qDebug() << "Maze Size:" << nodes.size();
    qDebug() << "N Row:" << nRow;
    qDebug() << "N Col:" << nCol;
}

void Maze::readInitialState(std::string str) {
    qDebug() << "reading initial state";
    //split out the x and y of initial state
    unsigned first = str.find("(")+1;
    unsigned last = str.find(")");
    std::string subStateString = str.substr (first,last-first);
    QStringList splitState = QString::fromStdString(subStateString).split(QRegExp("(\\,)"));

    //get the x and y coords
    int x = splitState.at(0).toInt();
    int y = splitState.at(1).toInt();

    //find the matching node and update the state
    auto nodeObj = *std::find_if(
      nodes.begin(), nodes.end(),
      [&x, &y](Node *node) { return (node->getX() == x && node->getY() == y); }
    );
    //if the node was found
    if (nodeObj != *nodes.end())
    {
        //update the node state
        nodeObj->setType("agent");
        agent = nodeObj;
        qDebug() << "agent at" << nodeObj->getX() << nodeObj->getY();
    }
    //if the node wasnt found
    else
    {
      qDebug() << "Matching node not found";
    }
}

void Maze::readGoalStates(QString str) {
    qDebug() << "reading goal states";
    //split by ' | '
    QStringList goalStrings = str.split('|');

    //loop through all the goal string
    for (int i = 0; i < goalStrings.size(); i++) {
        std::string stdString = goalStrings.at(i).toUtf8().constData();
        //contained in ()
        unsigned first = stdString.find("(")+1;
        unsigned last = stdString.find(")");
        std::string subStateString = stdString.substr (first,last-first);
        QStringList splitState = QString::fromStdString(subStateString).split(QRegExp("(\\,)"));

        //get the x and y coords
        int x = splitState.at(0).toInt();
        int y = splitState.at(1).toInt();

        //find the matching node and update the state
        auto nodeObj = *std::find_if(
          nodes.begin(), nodes.end(),
          [&x, &y](Node *node) { return (node->getX() == x && node->getY() == y); }
        );
        //if the node was found
        if (nodeObj != *nodes.end())
        {
            //update the node state
            nodeObj->setType("goal");
            qDebug() << "goal at" << nodeObj->getX() << nodeObj->getY();
        }
        //if the node wasnt found
        else
        {
          qDebug() << "Matching node not found";
        }
    }
}

void Maze::readObstructionNodes(std::string str) {
    qDebug() << "reading obstruction";
    //split out the x and y of initial state
    unsigned first = str.find("(")+1;
    unsigned last = str.find(")");
    std::string subStateString = str.substr (first,last-first);
    QStringList splitState = QString::fromStdString(subStateString).split(QRegExp("(\\,)"));

    //get the initx inity w h coords
    int initx = splitState.at(0).toInt();
    int inity = splitState.at(1).toInt();

    int w = splitState.at(2).toInt();
    int h = splitState.at(3).toInt();

    //get the final x and y
    int endx = initx + w;
    int endy = inity + h;

    QList<QPair<int, int>> xyList;
    //loop through all the possible obstructions
    for (int x = initx; x < endx; x++) {
        for (int y = inity; y < endy; y++) {
            qDebug() << "x and y:" << x << y;
            //add the x and y coord to the list
            xyList.append(qMakePair(x, y));
        }
    }

    //loop through all the xy coords
    for (int i = 0; i < xyList.size(); i++) {
        int x = xyList[i].first;
        int y = xyList[i].second;

        //find the matching node and update the state
        auto nodeObj = *std::find_if(
          nodes.begin(), nodes.end(),
          [&x, &y](Node *node) { return (node->getX() == x && node->getY() == y); }
        );
        //if the node was found
        if (nodeObj != *nodes.end())
        {
            //update the node state
            nodeObj->setType("obstruction");
            qDebug() << "obstruction at" << nodeObj->getX() << nodeObj->getY();
        }
        //if the node wasnt found
        else
        {
          qDebug() << "Matching node not found";
        }
    }
}

Node* Maze::bfs() {
    qDebug() << "Running BFS!";
    //queue of nodes
    QList<Node*> queue;
    //used nodes
    QList<Node*> usedNodes;
    //map of node to parent <Child, Parent>
    QMap<Node*, Node*> parentMap;
    Node* start;

    //move list
    QVector<std::tuple<QString, int, int>> moves;
    moves.append(std::make_tuple("up", 0, -1));
    moves.append(std::make_tuple("left", -1, 0));
    moves.append(std::make_tuple("down", 0, 1));
    moves.append(std::make_tuple("right", 1, 0));

    //add the agent starting position to frontier
    queue.append(agent);
    start = agent;

    //loop through all the nodes in queue
    while (!queue.isEmpty()) {
        //sleep 500 ms
        std::this_thread::sleep_for (std::chrono::milliseconds(500));

        //set the last used node as a used node
        if(usedNodes.size() > 0) {
            usedNodes.last()->setType("used");
        }

        //take the first node out of queue and put it into used nodes
        Node *current = queue.takeFirst();

        qDebug() << "current" << current->getX() << current->getY();
        usedNodes.append(current);

        //if the current node is the destination
        if(current->getType() == "goal") {
            return current;
        }
        //set the current node as the agent
        current->setType("agent");
        emit nodesUpdated();
        foreach (auto move, moves) {
            //if the neighbour is valid
            if(isValid(current->getX() + std::get<1>(move), current->getY() + std::get<2>(move))) {
                //get the node neighbouring node
                Node* neighbour = findNode(current->getX() + std::get<1>(move), current->getY() + std::get<2>(move));
                //if the neighbour hasnt been used before
                if(!(usedNodes.contains(neighbour) || queue.contains(neighbour))) {
                    //add the map between the child and parent node
                    parentMap.insert(neighbour, current);

                    //if the neighbour is the solution then return it
                    if(neighbour->getType() == "goal") {
                        qDebug() << "Found goal:" << neighbour->getX() << neighbour->getY();
                        //set the goal node as an agent
                        neighbour->setType("agent");
                        //set the last used node as a neighbour - required for the final move
                        usedNodes.last()->setType("used");

                        QList<Node*> path = backtrace(parentMap, start, neighbour);
                        for(int i = 0; i < path.size(); i++) {
                            path[i]->setType("path");
                        }

                        emit nodesUpdated();
                        return neighbour;
                    } else {
                        //set the nodes type to neighbour
                        neighbour->setType("neighbour");
                        //add the neighbour to the END of the queue
                        queue.append(neighbour);
                    }
                }
            }
        }
    }
}

Node* Maze::dfs() {
    qDebug() << "Running DFS!";

    //queue of nodes
    QList<Node*> queue;
    //used nodes
    QList<Node*> usedNodes;
    //map of node to parent <Child, Parent>
    QMap<Node*, Node*> parentMap;
    Node* start;

    //move list
    //move list for dfs needs to be reversed to bfs as neighbours are prepended not appended
    QVector<std::tuple<QString, int, int>> moves;
    moves.append(std::make_tuple("right", 1, 0));
    moves.append(std::make_tuple("down", 0, 1));
    moves.append(std::make_tuple("left", -1, 0));
    moves.append(std::make_tuple("up", 0, -1));

    //add the agent starting position to frontier
    queue.append(agent);
    start = agent;

    //loop through all the nodes in queue
    while (!queue.isEmpty()) {
        //sleep 500 ms
        std::this_thread::sleep_for (std::chrono::milliseconds(500));

        //set the last used node as a used node
        if(usedNodes.size() > 0) {
            usedNodes.last()->setType("used");
        }

        //take the first node out of queue and put it into used nodes
        Node *current = queue.takeFirst();

        qDebug() << "current" << current->getX() << current->getY();
        usedNodes.append(current);

        //if the current node is the destination
        if(current->getType() == "goal") {
            return current;
        }
        //set the current node as the agent
        current->setType("agent");
        emit nodesUpdated();
        foreach (auto move, moves) {
            //if the neighbour is valid
            if(isValid(current->getX() + std::get<1>(move), current->getY() + std::get<2>(move))) {
                //get the node neighbouring node
                Node* neighbour = findNode(current->getX() + std::get<1>(move), current->getY() + std::get<2>(move));
                //if the neighbour hasnt been used before
                if(!(usedNodes.contains(neighbour) || queue.contains(neighbour))) {
                    //add the map between the child and parent node
                    parentMap.insert(neighbour, current);

                    //if the neighbour is the solution then return it
                    if(neighbour->getType() == "goal") {
                        qDebug() << "Found goal:" << neighbour->getX() << neighbour->getY();
                        //set the goal node as an agent
                        neighbour->setType("agent");
                        //set the last used node as a neighbour - required for the final move
                        usedNodes.last()->setType("used");

                        QList<Node*> path = backtrace(parentMap, start, neighbour);
                        for(int i = 0; i < path.size(); i++) {
                            path[i]->setType("path");
                        }

                        emit nodesUpdated();
                        return neighbour;
                    } else {
                        //set the nodes type to neighbour
                        neighbour->setType("neighbour");
                        //add the neighbour to the FRONT of the queue
                        queue.prepend(neighbour);
                    }
                }
            }
        }
    }
}

//this function runs the iterative deepening search
//depth increases from 0 through infinity until goal node is found
Node* Maze::iddfs() {
    qDebug() << "Running IDDFS!";
    //initialise usedNodes list for use in depth limited search
    QList<Node*> usedNodes;
    //map of node to parent <Child, Parent>
    QMap<Node*, Node*> parentMap;

    //get the initial states of all of the nodes and store them to reset the maze
    QList<QString> initialNodesTypes;
    for(int i = 0; i < nodes.size(); i++) {
        initialNodesTypes.append(nodes[i]->getType());
    }

    //set the agent node as the starting node
    Node* start = agent;

    int i = 0;
    bool found = false;
    //loop from 0 to infinity
    do {
        qDebug() << "at depth " << i;

        //every time the depth changes reset the node states back to original
        for(int j = 0; j < nodes.size(); j++) {
            nodes[j]->setType(initialNodesTypes[j]);
        }

        //run the depth limited search function to specified depth - return a matching node and path to get there
        QPair<Node*, QMap<Node*, Node*>> found = dls(start, i, usedNodes, parentMap);
        //if a node was found in the depth limited search then it is solution
        if(found.first != nullptr) {
            qDebug() << "found at depth " << i;
            found.first->setType("agent");

            //create a backtrace of the path to get to the returned goal node
            QList<Node*> path = backtrace(found.second, start, found.first);
            for(int k = 0; k < path.size(); k++) {
                path[k]->setType("path");
            }

            //update the UI and return the goal node
            emit nodesUpdated();
            return found.first;
        }
        i++;
    } while (found == false);

    //if no node was found then there is no goal node
    qDebug() << "node not found";
    return nullptr;
}

//this is the depth limited search function which is recursively called until desired depth is reached
QPair<Node*, QMap<Node*, Node*>> Maze::dls(Node* node, int depth, QList<Node*> usedNodes, QMap<Node*, Node*> parentMap) {
    //change any nodes marked as agent to used nodes
    agentToUsed();

    //move list
    //move list for dfs needs to be reversed to bfs as neighbours are prepended not appended
    QVector<std::tuple<QString, int, int>> moves;
    moves.append(std::make_tuple("up", 0, -1));
    moves.append(std::make_tuple("left", -1, 0));
    moves.append(std::make_tuple("down", 0, 1));
    moves.append(std::make_tuple("right", 1, 0));

    //if the node is a goal then return with path
    if (node->getType() == "goal") {
        return qMakePair(node, parentMap);
    }
    node->setType("agent");
    //only continue if the depth is greater than 0 - at 0 return
    if (depth > 0) {
        //sleep 20ms
        std::this_thread::sleep_for (std::chrono::milliseconds(20));
        //add the current node to used nodes list
        usedNodes.append(node);

        emit nodesUpdated();
        //loop through all of the possible moves
        foreach (auto move, moves) {
            //if the neighbour is valid
            if(isValid(node->getX() + std::get<1>(move), node->getY() + std::get<2>(move))) {
                //get the neighbouring child node
                Node* child = findNode(node->getX() + std::get<1>(move), node->getY() + std::get<2>(move));
                //only run dls again if node wasn't previously used
                if(!usedNodes.contains(child)) {
                    //add the map between the child and parent node
                    parentMap.insert(child, node);
                    QPair<Node*, QMap<Node*, Node*>> found = dls(child, depth - 1, usedNodes, parentMap);
                    if (found.first != nullptr) {
                        return found;
                    }
                }
            }
        }
    }
    //if no node was found in desired depth then return null
    return qMakePair(nullptr, parentMap);
}

Node* Maze::gbfs() {
    qDebug() << "Running Greedy Best-First Search!";
    //queue of nodes
    QList<Node*> queue;
    //used nodes
    QList<Node*> usedNodes;
    //map of node to parent <Child, Parent>
    QMap<Node*, Node*> parentMap;
    Node* start;

    //move list
    QVector<std::tuple<QString, int, int>> moves;
    moves.append(std::make_tuple("up", 0, -1));
    moves.append(std::make_tuple("left", -1, 0));
    moves.append(std::make_tuple("down", 0, 1));
    moves.append(std::make_tuple("right", 1, 0));

    //add the agent starting position to frontier and calculate its heuristic cost
    agent->setHCost(calculateH(agent));
    queue.append(agent);
    start = agent;

    //loop through all the nodes in queue
    while (!queue.isEmpty()) {
        //sleep 500 ms
        std::this_thread::sleep_for (std::chrono::milliseconds(500));

        //set the last used node as a used node
        if(usedNodes.size() > 0) {
            usedNodes.last()->setType("used");
        }

        //find the node with the lowest H value in the queue
        Node* minHNode = *std::min_element(queue.begin(), queue.end(),
                                         []( Node* &a, Node* &b)
                                        {
                                            return a->getHCost() < b->getHCost();
                                        }  );

        //remove any instances of the node from the queue and add it to usedNodes list
        queue.removeAll(minHNode);
        qDebug() << "current" << minHNode->getX() << minHNode->getY();
        usedNodes.append(minHNode);

        //if the current node is the destination
        if(minHNode->getType() == "goal") {
            return minHNode;
        }
        //set the current node as the agent
        minHNode->setType("agent");
        emit nodesUpdated();

        //loop through all of the possible moves
        foreach (auto move, moves) {
            //if the neighbour is valid
            if(isValid(minHNode->getX() + std::get<1>(move), minHNode->getY() + std::get<2>(move))) {
                //get the node neighbouring node
                Node* neighbour = findNode(minHNode->getX() + std::get<1>(move), minHNode->getY() + std::get<2>(move));

                //if the neighbour hasnt been used before
                if(!(usedNodes.contains(neighbour) || queue.contains(neighbour))) {
                    //calculate the heuristic cost and set it to the neighbouring node
                    neighbour->setHCost(calculateH(neighbour));

                    //add the map between the child and parent node
                    parentMap.insert(neighbour, minHNode);

                    //if the neighbour is the solution then return it
                    if(neighbour->getType() == "goal") {
                        qDebug() << "Found goal:" << neighbour->getX() << neighbour->getY();
                        //set the goal node as an agent
                        neighbour->setType("agent");
                        //set the last used node as a neighbour - required for the final move
                        usedNodes.last()->setType("used");

                        //create a backtrace of the path to get to the returned goal node
                        QList<Node*> path = backtrace(parentMap, start, neighbour);
                        for(int i = 0; i < path.size(); i++) {
                            path[i]->setType("path");
                        }

                        emit nodesUpdated();
                        return neighbour;
                    } else {
                        //set the nodes type to neighbour
                        neighbour->setType("neighbour");
                        //add the neighbour to the END of the queue
                        queue.append(neighbour);
                    }
                }
            }
        }
    }
}

//reset any agent nodes to used nodes
void Maze::agentToUsed() {
    for (int i = 0; i < nodes.size(); i++) {
        if(nodes[i]->getType() == "agent") {
            nodes[i]->setType("used");
        }
    }
}

//create a backtrace from start to end node using the mapping between parent and child nodes
QList<Node*> Maze::backtrace(QMap<Node*, Node*> parentMap, Node* start, Node* end) {
    QList<Node*> path;
    //add the end node to the start of the path
    path.append(end);

    //while the last element in the path isnt the starting node
    while(path.last() != start) {
        //add the node from the parentMap to the path
        path.append(parentMap[path.last()]);
    }

    //reverse the path so start of the path is at the start of the list
    std::reverse(path.begin(), path.end());

    return path;
}


//QList<Node> Maze::aStar(Node player, Node dest) {
//    QList<Node> empty;
//    if (!isValid(dest.getX(), dest.getY())) {
//        qDebug << "Destination is an obstacle";
//        return empty;
//        //Destination is invalid
//    }
//    if (isDestination(player.x, player.y, dest)) {
//        cout << "You are the destination" << endl;
//        return empty;
//        //You clicked on yourself
//    }
//    bool closedList[(X_MAX / X_STEP)][(Y_MAX / Y_STEP)];

//    //Initialize whole map
//    //Node allMap[50][25];
//    array<array < Node, (Y_MAX / Y_STEP)>, (X_MAX / X_STEP)> allMap;
//    for (int x = 0; x < (X_MAX / X_STEP); x++) {
//        for (int y = 0; y < (Y_MAX / Y_STEP); y++) {
//            allMap[x][y].fCost = FLT_MAX;
//            allMap[x][y].gCost = FLT_MAX;
//            allMap[x][y].hCost = FLT_MAX;
//            allMap[x][y].parentX = -1;
//            allMap[x][y].parentY = -1;
//            allMap[x][y].x = x;
//            allMap[x][y].y = y;

//            closedList[x][y] = false;
//        }
//    }

//    //Initialize our starting list
//    int x = player.x;
//    int y = player.y;
//    allMap[x][y].fCost = 0.0;
//    allMap[x][y].gCost = 0.0;
//    allMap[x][y].hCost = 0.0;
//    allMap[x][y].parentX = x;
//    allMap[x][y].parentY = y;

//    vector<Node> openList;
//    openList.emplace_back(allMap[x][y]);
//    bool destinationFound = false;

//    while (!openList.empty()&&openList.size()<(X_MAX / X_STEP)*(Y_MAX / Y_STEP)) {
//        Node node;
//        do {
//            //This do-while loop could be replaced with extracting the first
//            //element from a set, but you'd have to make the openList a set.
//            //To be completely honest, I don't remember the reason why I do
//            //it with a vector, but for now it's still an option, although
//            //not as good as a set performance wise.
//            float temp = FLT_MAX;
//            vector<Node>::iterator itNode;
//            for (vector<Node>::iterator it = openList.begin();
//                it != openList.end(); it = next(it)) {
//                Node n = *it;
//                if (n.fCost < temp) {
//                    temp = n.fCost;
//                    itNode = it;
//                }
//            }
//            node = *itNode;
//            openList.erase(itNode);
//        } while (isValid(node.x, node.y) == false);

//        x = node.x;
//        y = node.y;
//        closedList[x][y] = true;

//        //For each neighbour starting from North-West to South-East
//        for (int newX = -1; newX <= 1; newX++) {
//            for (int newY = -1; newY <= 1; newY++) {
//                double gNew, hNew, fNew;
//                if (isValid(x + newX, y + newY)) {
//                    if (isDestination(x + newX, y + newY, dest))
//                    {
//                        //Destination found - make path
//                        allMap[x + newX][y + newY].parentX = x;
//                        allMap[x + newX][y + newY].parentY = y;
//                        destinationFound = true;
//                        return makePath(allMap, dest);
//                    }
//                    else if (closedList[x + newX][y + newY] == false)
//                    {
//                        gNew = node.gCost + 1.0;
//                        hNew = calculateH(x + newX, y + newY, dest);
//                        fNew = gNew + hNew;
//                        // Check if this path is better than the one already present
//                        if (allMap[x + newX][y + newY].fCost == FLT_MAX ||
//                            allMap[x + newX][y + newY].fCost > fNew)
//                        {
//                            // Update the details of this neighbour node
//                            allMap[x + newX][y + newY].fCost = fNew;
//                            allMap[x + newX][y + newY].gCost = gNew;
//                            allMap[x + newX][y + newY].hCost = hNew;
//                            allMap[x + newX][y + newY].parentX = x;
//                            allMap[x + newX][y + newY].parentY = y;
//                            openList.emplace_back(allMap[x + newX][y + newY]);
//                        }
//                    }
//                }
//            }
//        }
//        }
//        if (destinationFound == false) {
//            cout << "Destination not found" << endl;
//            return empty;
//    }
//}

