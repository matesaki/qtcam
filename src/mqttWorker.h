#ifndef MQTTWORKER_H
#define MQTTWORKER_H

#include <QMqttClient>
#include "config.h"

class MqttWorker : public QObject
{
	Q_OBJECT

public:
    explicit MqttWorker(ConfigStruct *conf);
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
};

#endif // MQTTWORKER_H
