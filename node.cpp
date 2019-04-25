#include "node.h"

Node::Node()
{

}

Node::Node(int x, int y) {
    this->x = x;
    this->y = y;
}

Node::Node(int x, int y, QString type) {
    this->x = x;
    this->y = y;
    this->type = type;
}

Node::Node(int x, int y, QString type, int hCost) {
    this->x = x;
    this->y = y;
    this->type = type;
    this->hCost = hCost;
}
