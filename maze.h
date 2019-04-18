#ifndef MAZE_H
#define MAZE_H

//define step cose and grid size
#define X_MAX 1000
#define X_STEP 20
#define Y_MAX 500
#define Y_STEP 20

#include <QList>
#include <QVariant>
#include <QVector>
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QPair>
#include <chrono>
#include <thread>
#include <QtConcurrent>

#include "node.h"

class Maze : public QObject {
    Q_OBJECT
    Q_PROPERTY(int nRow READ getNRow WRITE setNRow NOTIFY nRowChanged)
    Q_PROPERTY(int nCol READ getNCol WRITE setNCol NOTIFY nColChanged)
    Q_PROPERTY(Node* agent READ getAgent WRITE setAgent NOTIFY agentChanged)
    Q_PROPERTY(QList<Node*> nodes READ getNodes WRITE setNodes NOTIFY nodesChanged)
public:
    Q_INVOKABLE void readMazeFile(QString fileName);
    Q_INVOKABLE void search();
    Q_INVOKABLE QString getColor(int x, int y);

    Maze();

    int getNRow() { return this->nRow; }
    void setNRow(int nRow) { this->nRow = nRow; }

    int getNCol() { return this->nCol; }
    void setNCol(int nCol) { this->nCol = nCol; }

    Node* getAgent() { return this->agent; }
    void setAgent(Node* agent) { this->agent = agent; }

    QList<Node*> getNodes() { return this->nodes; }
    void setNodes(QList<Node*> nodes) { this->nodes = nodes; }

    bool isValid(int x, int y);
    bool isDestination(int x, int y);
    //calculate the H cost - cost of all adjacent node paths
    double calculateH(int x, int y, Node dest);

    QVariant findInModel(QList<QObject*> model, std::function<bool(QObject* obj)> const& criteria);
    Node* findNode(int x, int y);

    void readRowCol(std::string str);
    void readInitialState(std::string str);
    void readGoalStates(QString str);
    void readObstructionNodes(std::string str);
    void setType(int x, int y, QString type);

    Node* bfs();
    Node* dfs();
    Node* iddfs();
    bool dls(Node* node, int depth);
    QList<Node> aStar(Node player, Node dest);

private:
    int nRow;
    int nCol;
    Node *agent;
    QList<Node*> nodes;

signals:
    void nRowChanged();
    void nColChanged();
    void agentChanged();
    void nodesChanged();
    void nodesUpdated();
};

#endif // MAZE_H
