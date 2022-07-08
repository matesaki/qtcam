#ifndef CONFIG_H
#define CONFIG_H

struct ConfigStruct {
    bool exist;
    int cameraMode;
    int stillformatId;
    int stillresolutionId;
    std::string mqttBroker;
    std::string mqttTopicPub;
    std::string mqttTopicSub;
    int mqttPort;
    std::string name;
    std::string cameraName;
    std::string storePlace;
};

#endif // MQTTWORKER_H
