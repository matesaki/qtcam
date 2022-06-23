#ifndef MQTTWORKER_H
#define MQTTWORKER_H

#include <vector>
#include <QMqttClient>
#include "config.h"
#include "cameraproperty.h"

class MqttWorker : public QObject
{
	Q_OBJECT

public:
    explicit MqttWorker(ConfigStruct *conf, Cameraproperty *camProperty);
    ~MqttWorker(){};

public slots:
    void printSetting();

private slots:
    void updateLogStateChange(QMqttClient::ClientState state);
    void brokerDisconnected();
    void brokerConnected();
    void brokerHostnameChanged(QString hostname);
    void subscribeAfterSomeTime();
    void publishMsg(std::string msg);
    void fillMissingValuesInConfig();
    void processCommand(std::string topic, std::string message);

private:
    QMqttClient *m_client;
    ConfigStruct *m_conf;
    Cameraproperty *m_camProperty;
    std::vector<int> m_camIdx;
};

#endif // MQTTWORKER_H
