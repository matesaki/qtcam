#ifndef MQTTWORKER_H
#define MQTTWORKER_H

#include <vector>
#include <QMqttClient>
#include "config.h"
#include "cameraproperty.h"

class CameraInfo
{
public:
    CameraInfo(){
        camIdx = -1;
        hidFd = -1;
        serialNo = "";
        deviceNode = "";
    };
    ~CameraInfo(){};
    void printAll();
    QString QcamIdx(){return QString::number(camIdx);};
    QString QdeviceNode(){return QString::fromStdString(deviceNode);};

    int camIdx;
    int hidFd;
    std::string serialNo;
    std::string deviceNode;
};

class MqttWorker : public QObject
{
    Q_OBJECT

public:
    explicit MqttWorker(ConfigStruct *conf, Cameraproperty *camProperty);
    ~MqttWorker(){};

public slots:
    void printSetting();
    void exitThisApp();

private slots:
    void updateLogStateChange(QMqttClient::ClientState state);
    void brokerDisconnected();
    void brokerConnected();
    void brokerHostnameChanged(QString hostname);
    void subscribeAfterSomeTime();
    void publishMsg(std::string msg);
    void fillMissingValuesInConfig();
    void processCommand(std::string topic, std::string message);
    void closeAndClearAll();

private:
    QMqttClient *m_client;
    ConfigStruct *m_conf;
    Cameraproperty *m_camProperty;
    std::vector<int> m_camIdx; // delete this
    std::vector<CameraInfo> m_cameraInfo;
};

#endif // MQTTWORKER_H
