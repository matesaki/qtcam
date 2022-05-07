//#include <QtMqtt/QMqttClient>
// https://doc.qt.io/QtMQTT/qmqttclient.html
#include <iostream>
#include <QtCore>
#include "mqttWorker.h"

using namespace std;

MqttWorker::MqttWorker(ConfigStruct *conf) {
	m_client = new QMqttClient();
    m_conf = conf;
    fillMissingValuesInConfig();

    //connect(m_client, &QMqttClient::connected, this, &MqttWorker::brokerConnected);
    //connect(m_client, &QMqttClient::disconnected, this, &MqttWorker::brokerDisconnected);
    //connect(m_client, &QMqttClient::hostnameChanged, this, &MqttWorker::brokerHostnameChanged);
    connect(m_client, &QMqttClient::stateChanged, this, &MqttWorker::updateLogStateChange);
    connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
        processCommand(topic.name().toStdString(), message.toStdString());
    });

    m_client->setHostname(QString::fromStdString(m_conf->mqttBroker));
    m_client->setPort(m_conf->mqttPort);
    m_client->connectToHost();

    QTimer::singleShot(2000, this, SLOT(subscribeAfterSomeTime())); // (msDelay, myClass, signalOrSlot);

    cout << "MqttWorker | Init DONE" << endl;
}

void MqttWorker::fillMissingValuesInConfig() {
    if(m_conf->mqttBroker == "") {
        m_conf->mqttBroker = "localhost";
    }
    if(m_conf->mqttPort == 0) {
        m_conf->mqttPort = 1883;
    }
    if(m_conf->mqttTopicPub == "") {
        m_conf->mqttTopicPub = "info";
    }
    if(m_conf->mqttTopicSub == "") {
        m_conf->mqttTopicSub = "cmd";
    }
}

void MqttWorker::printSetting() {
    cout << "MqttWorker | Setting:" << endl;
    cout << "  - Broker: " << m_conf->mqttBroker << endl;
    cout << "  - Port: " << m_conf->mqttPort << endl;
    cout << "  - Topic Publish: " << m_conf->mqttTopicPub << endl;
    cout << "  - Topic Subscribe: " << m_conf->mqttTopicSub << endl;
    cout << "  - Machine name: " << m_conf->name << endl;
}

void MqttWorker::subscribeAfterSomeTime() {
    // subscribe() works only after some time after connect to broker
    m_client->subscribe(QString::fromStdString(m_conf->mqttTopicSub));
    cout << "MqttWorker | Subscribed to topic '" << m_conf->mqttTopicSub << "'" << endl;

    publishMsg("ready");
}

void MqttWorker::publishMsg(string msg) {
    cout << "MqttWorker | Publish msg: " << msg << endl;
    string topicPath = m_conf->mqttTopicPub + '/' + m_conf->name;
    m_client->publish(QString::fromStdString(topicPath), QString::fromStdString(msg).toUtf8()); // topic, msg
}

void MqttWorker::updateLogStateChange(QMqttClient::ClientState state) {
    cout << "MqttWorker | State changed: ";
    switch(state) {
        case 0: cout << "Disconnected!" << endl; break;
        case 1: cout << "Connecting ..." << endl; break;
        case 2: cout << "Connected to '" << m_conf->mqttBroker << "'" << endl; break;
        default: cout << "???" << endl;
    }
}

void MqttWorker::brokerDisconnected() {
    cout << "MqttWorker | Broker disconnected!" << endl;
}

void MqttWorker::brokerConnected() {
    cout << "MqttWorker | Broker connected to '" << m_conf->mqttBroker << "'" << endl;

}

void MqttWorker::brokerHostnameChanged(QString hostname) {
    cout << "MqttWorker | Broker hostname changed to '" << hostname.toStdString() << "'" << endl;
}

void MqttWorker::processCommand(string topic, string message) {
    cout << "MqttWorker | Process command [" << topic << "] " << message << endl;
}
