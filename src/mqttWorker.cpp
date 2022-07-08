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

    publishMsg("Qtcam running");
}

void MqttWorker::publishMsg(string msg) {
    cout << "MqttWorker | Publish msg: " << msg << endl;
    string topicPath = m_conf->mqttTopicPub + '/' + m_conf->name;
    m_client->publish(QString::fromStdString(topicPath), QString::fromStdString(msg).toUtf8()); // topic, msg
}

void MqttWorker::exitThisApp() {
    cout << "MqttWorker::exitThisApp()" << endl;
    QCoreApplication::quit();
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

void MqttWorker::closeAndClearAll() {
    cout << "MqttWorker | Close and clear all" << endl;
    for (auto caminfo: m_cameraInfo) {
        m_camProperty->uvccam.exitDevice(caminfo.hidFd);
    }
    m_cameraInfo.clear();
}

void MqttWorker::processCommand(string topic, string command) {
    cout << "MqttWorker | [" << topic << "] Process command: " << command << endl;
    QString deviceNode = "";
    string msg = "";  // msg to publish on mqtt
    bool res = false;
    FSCAM_CU135 cu135;

    if (command == "init") {
        vector<int> camIdx;
        closeAndClearAll();

        // get camera indexes by name, needed only during init
        m_camProperty->checkforDevice();
        QStringList availableCam = m_camProperty->modelCam.stringList();
        for (int i = 0; i < availableCam.size(); ++i) {
            if (availableCam.at(i).toStdString() == m_conf->cameraName) {
                camIdx.push_back(i);
            }
        }

        cout << "|| INIT camIdx.size() == " << camIdx.size() << endl;
        publishMsg("INIT | Number of cameras: " + to_string(camIdx.size()));
        bool res2;
        for(auto idx: camIdx) {
            res2 = false;
            emit m_camProperty->setCurrentDevice(QString::number(idx), QString::fromStdString(m_conf->cameraName));
            m_camProperty->gainDeviceNodeMap(idx, deviceNode);
            m_camProperty->openEventNode(deviceNode);
            emit m_camProperty->setFirstCamDevice(idx);

            res = m_camProperty->uvccam.initExtensionUnit(QString::fromStdString(m_conf->cameraName));

            if (res) {
                res2 = cu135.setStillResolution(m_conf->stillformatId, m_conf->stillresolutionId);
            }
            //cout << "|| uvccam.initExtensionUnit() result = " << res << endl;
            msg = "INIT | " + deviceNode.toStdString() + " | " + m_camProperty->uvccam.getSerialNo() + " | ";
            if (res && res2) {
                publishMsg(msg + "OK | OK");
                CameraInfo caminfo;  // create object with info about camera
                caminfo.camIdx = idx;
                caminfo.hidFd = m_camProperty->uvccam.hid_fd;
                caminfo.serialNo = m_camProperty->uvccam.getSerialNo();
                caminfo.deviceNode = deviceNode.toStdString();
                m_cameraInfo.push_back(caminfo);
            } else if (res && !res2) {
                publishMsg(msg + "OK | FAIL");
            } else {
                publishMsg(msg + "FAIL | FAIL");
            }
        }
        if (camIdx.size() == 0) {
            publishMsg("INIT | No cameras!!");
        }
        /*for (auto caminfo: m_cameraInfo) {
            caminfo.printAll();
        }*/
    }
    else if (command == "swtrigger") {
        cout << "|| SW_TRIGGER m_cameraInfo.size() == " << m_cameraInfo.size() << endl;

        for (auto caminfo: m_cameraInfo) {
            //caminfo.printAll();
            m_camProperty->uvccam.hid_fd = caminfo.hidFd;
            /*cout << "  11111" << endl;
            emit m_camProperty->setCurrentDevice(caminfo.QcamIdx(), QString::fromStdString(m_conf->cameraName));
            cout << "  22222" << endl;
            //m_camProperty->gainDeviceNodeMap(caminfo.camIdx, caminfo.deviceNode);
            m_camProperty->openEventNode(caminfo.QdeviceNode());
            cout << "  33333" << endl;
            emit m_camProperty->setFirstCamDevice(caminfo.camIdx);
            cout << "  44444" << endl;*/

            res = cu135.setStreamMode(FSCAM_CU135::camStreamMode::STREAM_SOFTWARE_TRIGGER);
            msg = "SW_TRIGGER | " + caminfo.deviceNode + " | " + caminfo.serialNo + " | ";
            if (res) {
                publishMsg(msg + "OK");
            } else {
                publishMsg(msg + "FAIL");
            }
        }

        /*
        for(auto idx: m_camIdx) {
            emit m_camProperty->setCurrentDevice(QString::number(idx), QString::fromStdString(m_conf->cameraName));
            m_camProperty->gainDeviceNodeMap(idx, deviceNode);
            m_camProperty->openEventNode(deviceNode);
            emit m_camProperty->setFirstCamDevice(idx);

            bool res = m_camProperty->uvccam.initExtensionUnit(QString::fromStdString(m_conf->cameraName));
            if (res) {
                FSCAM_CU135 cu135;
                res = cu135.setStreamMode(FSCAM_CU135::camStreamMode::STREAM_SOFTWARE_TRIGGER);
                //cout << "|| cu135.setStreamMode() result = " << res << endl;
                //cout << "|| cu135.getStillResolution()" << endl;
                //cu135.getStillResolution();
                msg = "SW_TRIGGER | " + deviceNode.toStdString() + " | " + m_camProperty->uvccam.getSerialNo() + " | ";
                if (res) {
                    publishMsg(msg + "OK");
                } else {
                    publishMsg(msg + "FAIL");
                }
            }

        }*/
        if (m_cameraInfo.size() == 0) {
            publishMsg("SW_TRIGGER | No cameras (did you start with 'init'?)");
        }
    }
    else if (command == "notrigger") {
        cout << "|| NO_TRIGGER - not implemented yet :(" << endl;
    }
    else if (command == "grab") {
        cout << "|| GRAB ||" << endl;
        for (auto caminfo: m_cameraInfo) {
            m_camProperty->uvccam.hid_fd = caminfo.hidFd;
            res = cu135.storePreviewFrameFast();

            msg = "GRAB | " + caminfo.deviceNode + " | " + caminfo.serialNo + " | ";
            if (res) {
                publishMsg(msg + "OK");
            } else {
                publishMsg(msg + "FAIL");
            }
        }
    }
    else if (command == "exit") {
        cout << "|| EXIT ||" << endl;
        publishMsg("Qtcam stopped");
        // close all cameras hid_fd
        closeAndClearAll();

        // QTimer because publish() sending after function is done.
        QTimer::singleShot(500, this, SLOT(exitThisApp()));
    }
    else {
        cout << "|| Unknown command: " << command << endl;
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

//=============================================================================

void CameraInfo::printAll() {
    cout << "CameraInfo || " << endl;
    cout << " | camIdx: " << camIdx << endl;
    cout << " | hidFd: " << hidFd << endl;
    cout << " | serialNo: " << serialNo << endl;
    cout << " | deviceNode: " << deviceNode << endl;
    cout << flush;
}
