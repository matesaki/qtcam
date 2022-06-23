//#include <QtMqtt/QMqttClient>
// https://doc.qt.io/QtMQTT/qmqttclient.html
#include <iostream>
#include <QtCore>
#include "mqttWorker.h"
#include "fscam_cu135.h"

using namespace std;

MqttWorker::MqttWorker(ConfigStruct *conf, Cameraproperty *camProperty) {
	m_client = new QMqttClient();
    m_conf = conf;
    m_camProperty = camProperty;
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

void MqttWorker::processCommand(string topic, string command) {
    cout << "MqttWorker | [" << topic << "] Process command: " << command << endl;

    int deviceIndex = 9999;
    m_camProperty->checkforDevice();
    QStringList availableCam = m_camProperty->modelCam.stringList();
    for (int i = 0; i < availableCam.size(); ++i) {
        if (availableCam.at(i).toStdString() == m_conf->cameraName) {
            deviceIndex = i;
            m_camIdx.push_back(i);
            cout << "|| index: " << i << endl;
        }
    }

    if (command == "init") {
        for(auto idx: m_camIdx) {
            cout << "|| ============ swtrigger ================== || " << endl;
            cout << "|| idx: " << idx << endl;
            emit m_camProperty->setCurrentDevice(QString::number(idx), QString::fromStdString(m_conf->cameraName));

            QString deviceNode = "";
            m_camProperty->gainDeviceNodeMap(idx, deviceNode);
            cout << "|| deviceNode: " << deviceNode.toStdString() << endl;
            m_camProperty->openEventNode(deviceNode);

            cout << "|| emit setFirstCamDevice(" << idx << "); " << endl;
            emit m_camProperty->setFirstCamDevice(idx);

            bool res = m_camProperty->uvccam.initExtensionUnit(QString::fromStdString(m_conf->cameraName));
            cout << "|| uvccam.initExtensionUnit() result = " << res << endl;

            FSCAM_CU135 cu135;
            //bool res = cu135.initFSCAM_CU135((char*) deviceNode.toStdString().c_str());
            //cout << "|| initFSCAM_CU135() result = " << res << endl;

            FSCAM_CU135::camStreamMode swtrigg = FSCAM_CU135::camStreamMode::STREAM_SOFTWARE_TRIGGER;
            //cout << "|| sw_trigg = " << swtrigg << endl;
            res = cu135.setStreamMode(swtrigg);
            cout << "|| cu135.setStreamMode() result = " << res << endl;
            //cout << "|| cu135.getStillResolution()" << endl;
            //cu135.getStillResolution();
        }
    } else if (command == "grab") {
        for(auto idx: m_camIdx) {
            cout << "|| ============= grab ================= || " << endl;
            cout << "|| idx: " << idx << endl;
            emit m_camProperty->setCurrentDevice(QString::number(idx), QString::fromStdString(m_conf->cameraName));

            QString deviceNode = "";
            m_camProperty->gainDeviceNodeMap(idx, deviceNode);
            cout << "|| deviceNode: " << deviceNode.toStdString() << endl;
            m_camProperty->openEventNode(deviceNode);

            cout << "|| emit setFirstCamDevice(" << idx << "); " << endl;
            emit m_camProperty->setFirstCamDevice(idx);

            FSCAM_CU135 cu135;
            bool res = cu135.initFSCAM_CU135((char*) deviceNode.toStdString().c_str());
            cout << "|| initFSCAM_CU135 result = " << res << endl;

            FSCAM_CU135::camStreamMode swtrigg = FSCAM_CU135::camStreamMode::STREAM_SOFTWARE_TRIGGER;
            cout << "|| sw_trigg = " << swtrigg << endl;
            cout << "|| cu135.setStreamMode()" << endl;
            cu135.setStreamMode(swtrigg);
            cout << "|| cu135.getStillResolution()" << endl;
            cu135.getStillResolution();
        }
    } else {
        cout << "|| Unknown command" << endl;

    }

    /*
    cout << "|| deviceIndex: " << deviceIndex << endl;
    emit m_camProperty->setCurrentDevice(QString::number(deviceIndex), QString::fromStdString(m_conf->cameraName));

    QString deviceNode = "";
    m_camProperty->gainDeviceNodeMap(deviceIndex, deviceNode);
    cout << "|| deviceNode: " << deviceNode.toStdString() << endl;
    m_camProperty->openEventNode(deviceNode);

    cout << "|| emit setFirstCamDevice(4); " << endl;
    emit m_camProperty->setFirstCamDevice(4);
    */

    // Find all cameras (get count)
    // Select right cameras (get indexes?)
    // Set format and output path/name
    // Switch to SW trigger (and back)
    // Store frame
    // Save image to location
}
