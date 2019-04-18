#ifndef NODE_H
#define NODE_H

#include <QObject>
#include <QString>

class Node : public QObject {
    Q_OBJECT
    Q_PROPERTY(int x READ getX WRITE setX NOTIFY xChanged)
    Q_PROPERTY(int y READ getY WRITE setY NOTIFY yChanged)
    Q_PROPERTY(QString type READ getType WRITE setType NOTIFY typeChanged)

public:
    Node();
    Node(int x, int y);
    Node(int x, int y, QString type);

    int getX() { return this->x; }
    void setX(int x) { this->x = x; }

    int getY() { return this->y; }
    void setY(int y) { this->y = y; }

    QString getType() { return this->type; }
    void setType(QString type) { this->type = type; }

private:
    int y;
    int x;
    QString type;

signals:
    void xChanged();
    void yChanged();
    void typeChanged();
};

#endif // NODE_H
