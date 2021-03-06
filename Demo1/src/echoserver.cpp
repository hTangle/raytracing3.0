#include "echoserver.h"
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <QtCore/QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include "mesh.h"
#include "echoserver.h"
#include "tracer.h"
#include "QJsonArray"
QT_USE_NAMESPACE

//! [constructor]
EchoServer::EchoServer(quint16 port, bool debug, QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(new QWebSocketServer(QStringLiteral("Echo Server"),
                                            QWebSocketServer::NonSecureMode, this)),
    m_debug(debug)
{
    if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
        if (m_debug)
            qDebug() << "Echoserver listening on port" << port;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
                this, &EchoServer::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &EchoServer::closed);
    }
}
//! [constructor]

EchoServer::~EchoServer()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

//! [onNewConnection]
void EchoServer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &EchoServer::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &EchoServer::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &EchoServer::socketDisconnected);
    qDebug() << "socket connected:";
    m_clients << pSocket;
}
//! [onNewConnection]

//! [processTextMessage]
void EchoServer::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
//        qDebug() << "Message received:" << message;
    if (pClient) {
        if(message == "1"){
            updateBuilding();
            QJsonObject qjss=QJsonObject::fromVariantMap(mapMap);
            QJsonDocument doc(qjss);
            QString strJson(doc.toJson(QJsonDocument::Compact));
            pClient->sendTextMessage(strJson);

            //QString bound = '';
        }else if(message == "2"){
//          射线追踪

//            QString data = rayTracing();
            QString data = VPL();
            pClient->sendTextMessage(data);
        }else if(message == '3'){
//          更新道路
            updateRoad();
            QJsonObject qjss=QJsonObject::fromVariantMap(mapMap);
            QJsonDocument doc(qjss);
            QString strJson(doc.toJson(QJsonDocument::Compact));
            pClient->sendTextMessage(strJson);
        }else if(message == "cordinate"){
            QMap<QString, QVariant> map;
            map.insert("xmax",xmax);
            map.insert("xmin",xmin);
            map.insert("ymax",ymax);
            map.insert("ymin",ymin);
            QJsonObject qjss = QJsonObject::fromVariantMap(map);
            QJsonDocument doc(qjss);
            QString strJson(doc.toJson(QJsonDocument::Compact));
            pClient->sendTextMessage(strJson);
        }else{//Tx Rx

            QJsonDocument jsonDocument = QJsonDocument::fromJson(message.toLocal8Bit().data());
               if(jsonDocument.isNull())
               {
                   qDebug()<< "String NULL"<< message.toLocal8Bit().data();
               }
               QJsonObject jsonObject = jsonDocument.object();

               QString type = jsonObject["type"].toString();
               if(type == "tx"){
                   double longitude =  jsonObject["longitude"].toDouble();
                   double latitude=  jsonObject["latitude"].toDouble();
                   double x = (longitude - xmin)/(xmax - xmin);
                   double y = (latitude - ymin)/(xmax - xmin);
                   qDebug() << "x:" << x*30 << "y:" << y*30;
                   receivedTx = new Node(x,y);
               }else if(type == "rx"){
                   double longitude =  jsonObject["longitude"].toDouble();
                   double latitude=  jsonObject["latitude"].toDouble();
                   double x = (longitude - xmin)/(xmax - xmin);
                   double y = (latitude - ymin)/(xmax - xmin);
                   qDebug() << "x:" << x*30 << "y:" << y*30;
                   receivedRx = new Node(x, y);
               }else if(type == "senario"){

                    sceneDate = jsonObject;
                    updateScene(sceneDate);
               }



        }

    }
}
//! [processTextMessage]

//! [processBinaryMessage]
//!
//!
void EchoServer::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "Binary Message received:" << message;
    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}
//! [processBinaryMessage]

//! [socketDisconnected]
void EchoServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "socketDisconnected:" << pClient;
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}
//! [socketDisconnected]


//QJsonObject json;
//json.insert("type", QString("output"));
//QJsonArray paths;
//for(int i = 0; i < 10; i++){
//    QJsonObject path;
//    for(int j = 0; j < 5 ; j++){
//        path.insert("pathloss",j+2);
//        QJsonArray nodeList;
//        for(int k = 0; k < 3; k++){
//            QJsonObject point;
//            point.insert("x",1);
//            point.insert("y",1);
//            point.insert("z",1);
//            nodeList.insert(k,point);
//        }
//        path.insert("nodeList",nodeList);
//    }
//    paths.insert(i,path);
//}
//json.insert("paths",paths);
//QJsonDocument doc(json);
//QString strJson(doc.toJson(QJsonDocument::Compact));
QString EchoServer::VPL(){
    updateScene(sceneDate);
    qDebug() << "scene size : "<<scene->objList.size();
    Node* rx = new Node(receivedRx->x, receivedRx->y, 0.0, true);
    Mesh* mesh = new Mesh(30, scene, rx);
    Node* source = new Node(receivedTx->x * mesh->size, receivedTx->y * mesh->size, 0);
    source->type = NodeType::Tx;
//    射线追踪器
    Tracer*  tracer = new Tracer(mesh,source);
    tracer->verticalPlane(source);
    qDebug()<<tracer->allPath.size()<<" paths has been accepted";
//    QMap<QString,QVariant> data;
    QJsonObject json;
    json.insert("type", QString("output"));
    QJsonArray paths;
    for(int i = 0; i < tracer->allPath.size(); i++){
        QJsonObject path;
        Path* p = tracer->allPath[i];
        path.insert("pathloss",p->channelGain(0));
        QJsonArray nodeList;
        for(int j = 0; j < p->nodeSet.size() ; j++){
            Node* node = p->nodeSet[j];
            QJsonObject point;
            point.insert("x",node->x);
            point.insert("y",node->y);
            point.insert("z",node->z);
            nodeList.insert(j,point);
        }
        path.insert("nodeList",nodeList);
        paths.insert(i,path);
    }
    json.insert("paths",paths);
    QJsonDocument doc(json);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    return strJson;
}


void EchoServer::updateScene(QJsonObject jsonObject){
    QJsonArray features = jsonObject["features"].toArray();
    QJsonArray bbox = jsonObject["bbox"].toArray();
    scene = new Scene();
//    reset
    scene->bbox[0] = xmin = bbox[0].toDouble();
    scene->bbox[1] = ymin = bbox[1].toDouble();
    scene->bbox[2] = xmax = bbox[2].toDouble();
    scene->bbox[3] = ymax = bbox[3].toDouble();
    scene->objList.clear();
    qDebug() << xmin << ymin << xmax << ymax;
    for(QJsonValue feature : features){
        Object* obj = new Object();
        QJsonArray coordinates = feature["coordinates"].toArray();
        double z = feature["z"].toString().toDouble();
        obj->z = z;
        for(QJsonValue coord : coordinates){
            double x = coord.toArray()[0].toDouble();
            double y = coord.toArray()[1].toDouble();
            x = (x - xmin) / (xmax - xmin);
            y = (y - ymin) / (xmax - xmin);
//            qDebug() << "x:" << x << "y:" << y << "z:" << z;
            obj->pointList.push_back(new Point(x, y ,z ));

        }

        obj->setEdgeList(obj->pointList);
//        qDebug() << obj->pointList.size();
//        qDebug() << obj->edgeList.size();
        scene->objList.push_back(obj);
    }
}
void EchoServer::updateRoad(){
    mapMap.clear();
    FilePoint *filePoint = new FilePoint();
    FileManager *fileManager = new FileManager("./R.shp","./R.dbf");
    fileManager->readRoadDbf(filePoint);
    fileManager->readRoadShp(filePoint);
    qDebug() <<"xmax:"<<filePoint->Xmax;
    qDebug() <<"xmin"<<filePoint->Xmin;
    qDebug() <<"ymax"<<filePoint->Ymax;
    qDebug() <<"ymin"<<filePoint->Ymin;
    QMap<QString,QVariant> boundMap;
    filePoint->uniformlize(80,0,xmax - xmin);//(0,1)
    Scene *scene = new Scene(filePoint->allPointList, filePoint->index);
    QJsonObject qjs;
    for(int i = 0; i < scene->objList.size(); i++){
        QMap<QString,QVariant> myMap;
        QList<QVariant> posList;
        for(int j=0;j<scene->objList[i]->pointList.size();j++){
            QMap<QString,QVariant> tempMap;
            tempMap.insert(QString("x"),QVariant((scene->objList[i]->pointList[j]->x )));
            tempMap.insert(QString("y"),QVariant((scene->objList[i]->pointList[j]->y )));
//            tempMap.insert(QString("x"),QVariant((scene->objList[i]->pointList[j]->x - 0.5)*80));
//            tempMap.insert(QString("y"),QVariant((scene->objList[i]->pointList[j]->y - 0.5)*80));
            tempMap.insert(QString("z"),QVariant(0) );
            posList.append(QVariant(tempMap));
        }

        mapMap.insert(QString::number(i,10),QVariant(posList));
    }
}
void EchoServer::updateBuilding(){
    mapMap.clear();
    FilePoint *filePoint = new FilePoint();
    FileManager *fileManager = new FileManager("E:/QT5_11_1/raytracing1.1/rayTracing1.1/RayTracing/BUILDING_nanjing.shp",
                                               "E:/QT5_11_1/raytracing1.1/rayTracing1.1/RayTracing/BUILDING_nanjing.dbf");
    fileManager->readDbfFile(filePoint);
    fileManager->readShpFile(filePoint);
    qDebug() <<"xmax:"<<filePoint->Xmax;
    qDebug() <<"xmin"<<filePoint->Xmin;
    qDebug() <<"ymax"<<filePoint->Ymax;
    qDebug() <<"ymin"<<filePoint->Ymin;
    QMap<QString,QVariant> boundMap;
    xmax =filePoint->Xmax;
    xmin =filePoint->Xmin;
    ymax =filePoint->Ymax;
    ymin =filePoint->Ymin;
    filePoint->uniformlize(80,50,xmax - xmin);//(0,1)
    Scene *scene = new Scene(filePoint->allPointList, filePoint->index);
    QJsonObject qjs;
    for(int i = 0; i < scene->objList.size(); i++){
        QMap<QString,QVariant> myMap;
        QList<QVariant> posList;
        for(int j=0;j<scene->objList[i]->pointList.size();j++){
            QMap<QString,QVariant> tempMap;
            tempMap.insert(QString("x"),QVariant((scene->objList[i]->pointList[j]->x )));
            tempMap.insert(QString("y"),QVariant((scene->objList[i]->pointList[j]->y)));
            tempMap.insert(QString("z"),QVariant(scene->objList[i]->pointList[j]->z ));
//            tempMap.insert(QString("x"),QVariant((scene->objList[i]->pointList[j]->x - 0.5)*80));
//            tempMap.insert(QString("y"),QVariant((scene->objList[i]->pointList[j]->y - 0.5)*80));
//            tempMap.insert(QString("z"),QVariant(scene->objList[i]->pointList[j]->z *10) );
            posList.append(QVariant(tempMap));
        }

        mapMap.insert(QString::number(i,10),QVariant(posList));
    }

}
