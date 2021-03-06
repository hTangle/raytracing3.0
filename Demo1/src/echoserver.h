#ifndef ECHOSERVER_H
#define ECHOSERVER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QtCore/QVariant>
#include <QMap>
#include <QList>
#include <QPointF>
#include "FileManager.h"
#include "FilePoint.h"
#include "scene.h"
#include <QVariant>
#include "path.h"
#include <QJsonObject>
QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class EchoServer : public QObject
{
    Q_OBJECT
public:
    QMap<QString,QVariant> mapMap;
    double xmax, xmin, ymax, ymin;
    Scene* scene;
    QJsonObject sceneDate;
// {"0":[{}]}
    explicit EchoServer(quint16 port, bool debug = false, QObject *parent = nullptr);
    ~EchoServer();

Q_SIGNALS:
    void closed();

private Q_SLOTS:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();
    void updateBuilding();
    void updateRoad();
    void updateScene(QJsonObject jsonObject);
    QString VPL();
private:
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
    bool m_debug;
    Node* receivedTx;
    Node* receivedRx;
};

#endif //ECHOSERVER_H
