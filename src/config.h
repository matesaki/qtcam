#ifndef CONFIG_H
#define CONFIG_H

struct ConfigStruct {
    bool exist;
    int option;
    int cameraMode;
    int stillformatId;
    int stillresolutionId;
    std::string mqttBroker;
    std::string mqttTopicPub;
    std::string mqttTopicSub;
    int mqttPort;
    std::string name;
};

#endif // MQTTWORKER_H
