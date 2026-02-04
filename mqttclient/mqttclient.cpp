#include "mqttclient.h"
#include <QDebug>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMqttClient>
MqttClient::MqttClient(QObject *parent) : QObject(parent)
{
    m_client = new QMqttClient(this);// 初始化 MQTT 客户端
    // 拼接 OneJson 标准 Topic
    QString prefix = QString("$sys/%1/%2/thing/property").arg(m_productId, m_deviceName);// 设备属性 Topic
    m_topicSet = prefix + "/set";// 订阅：接收控制
    m_topicPost = prefix + "/post";// 发布：上报数据
    m_topicReply = prefix + "/set_reply";// 发布：回复回执
    m_topicGet = prefix + "/get";// 订阅：查询数据
    // 连接信号槽
    connect(m_client, &QMqttClient::connected, this, &MqttClient::onConnected);// 连接成功槽
    connect(m_client, &QMqttClient::disconnected, this, &MqttClient::onDisconnected);// 断开连接槽
    connect(m_client, &QMqttClient::messageReceived, this, &MqttClient::onMessageReceived);// 收到消息槽
    connect(m_client, &QMqttClient::errorChanged, this, &MqttClient::onErrorOccurred);// 错误发生槽
    // 错误打印，方便调试
    connect(m_client, &QMqttClient::stateChanged, [](QMqttClient::ClientState state){
        qDebug() << "MQTT State:" << state;
    });//返回0-3的状态值：0-未连接，1-连接中，2-已连接，3-已断开
}
// 析构函数
MqttClient::~MqttClient()
{
    if (m_client) {
        m_client->disconnectFromHost();
        delete m_client;
        m_client = nullptr;
    }
}

// 连接到 OneNET
void MqttClient::connectToCloud()
{
    // 设置连接参数
    m_client->setHostname("mqtts.heclouds.com");// 设置主机名
    m_client->setPort(1883);// 设置端口号
    m_client->setUsername(m_productId);// 设置用户名
    m_client->setClientId(m_deviceName);
    m_client->setPassword(m_token);// 设置密码
    qDebug() << "ClientId:" << m_deviceName;
    qDebug() << "Usernameid:" << m_productId;
    qDebug() << "Debug Token2:[" << m_token << "]";
    // 连接到 OneNET
    m_client->connectToHost();
}

// 断开连接
void MqttClient::disconnectFromServer()
{
    m_client->disconnectFromHost();
}

// 【上报】通用设备状态 (LED, Beep, 音乐播放状态)
void MqttClient::publishDeviceState(QString key, bool isOn)
{
    QJsonObject valObj;
    valObj.insert("value", isOn); // 布尔值或 0/1

    QJsonObject params;
    params.insert(key, valObj);

    QJsonObject root;
    root.insert("id", "10");
    root.insert("version", "1.0");
    root.insert("params", params);
    // 发布消息到 OneNET
    m_client->publish(m_topicPost, QJsonDocument(root).toJson());
    qDebug() << "上报状态:" << key << isOn;
}

// 【上报】传感器数据 (光照, 红外)
void MqttClient::publishSensorData(int als, int ir)
{
    // 构造嵌套格式 {"params": {"als": {"value": 123}}}
    QJsonObject p_als, p_ir;
    p_als.insert("value", als);// 确保云端标识符是 "als"
    p_ir.insert("value", ir);// 确保云端标识符是 "ir"

    QJsonObject params;
    params.insert("als", p_als); // 确保云端标识符是 "als"
    params.insert("ir", p_ir);   // 确保云端标识符是 "ir"

    QJsonObject root;
    root.insert("id", "12");
    root.insert("version", "1.0");
    root.insert("params", params);
    //qDebug() << "Publishing sensor data to MQTT:" << QJsonDocument(root).toJson();
    m_client->publish(m_topicPost, QJsonDocument(root).toJson());
}
// 【回复】指令回执 (防止云端超时)
void MqttClient::sendCmdReply(QString msgId)
{
    QJsonObject root;
    root.insert("id", msgId);
    root.insert("code", 200);
    root.insert("msg", "success");
    m_client->publish(m_topicReply, QJsonDocument(root).toJson());
}


/*----------------------槽函数定义----------------------------------*/
// 连接成功槽函数
void MqttClient::onConnected()
{
    qDebug() << ">>> 已连接 OneNET 云端!";
    // 必须订阅 set 主题才能接收手机指令
    m_client->subscribe(m_topicSet);
    QString topicPostReply = m_topicPost + "/reply";
    m_client->subscribe(topicPostReply);
    //m_client->subscribe(m_topicGet);
    qDebug() << "已订阅控制主题:" << m_topicSet;
    qDebug() << "已订阅响应主题:" << topicPostReply;
    //qDebug() << "已订阅查询主题:" << m_topicGet;
}

// 收到消息槽函数
void MqttClient::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    if (topic.name() == m_topicSet) {
        // 解析 OneJson 格式: {"id":"123", "params": {"led": true}}
        QJsonDocument doc = QJsonDocument::fromJson(message);
        QJsonObject root = doc.object();
        QString msgId = root.value("id").toString();
        QJsonObject params = root.value("params").toObject();

        // 遍历 params，支持同时控制多个设备
        for(auto key : params.keys()) {
            bool val = params.value(key).toBool();
            emit remoteSwitchReceived(key, val, msgId);
        }
    }
}

// 断开连接槽函数
void MqttClient::onDisconnected()
{
    qDebug() << ">>> 已断开 OneNET 云端连接!";
}

//连接失败槽函数
void MqttClient::onErrorOccurred(QMqttClient::ClientError error)
{
    qDebug() << "!!! MQTT 错误发生 !!! 代码 :" << (int)error;
    
    if (error == 1) qDebug() << "原因：底层 Socket 错误（检查网络/地址）";
    if (error == 5) qDebug() << "原因：鉴权失败（ProductID/Token/DeviceName 填错了）";
}
