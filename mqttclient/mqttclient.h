#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <QObject>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttTopicName>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

class MqttClient : public QObject
{
    Q_OBJECT
public:
    explicit MqttClient(QObject *parent = nullptr);
    virtual ~MqttClient(); // 修正：添加析构函数声明，并设为virtual

    // 连接到 OneNET
    void connectToCloud();
    
    // 断开连接
    void disconnectFromServer();
    
    // 【上报】通用设备状态 (LED, Beep, 音乐播放状态)
    void publishDeviceState(QString key, bool isOn);

    // 【上报】传感器数据 (光照, 红外)
    void publishSensorData(int als, int ir);

    // 【回复】指令回执 (防止云端超时)
    void sendCmdReply(QString msgId);
    
    // // 检查连接状态
    // bool isConnected() const;
signals:
    // 信号：收到远程控制指令，发给 MainWindow 执行
    void remoteSwitchReceived(QString key, bool value, QString msgId);

private slots:
    // 连接状态变化槽
    void onConnected();
    void onMessageReceived(const QByteArray &message, const QMqttTopicName &topic);


    void onDisconnected();
    void onErrorOccurred(QMqttClient::ClientError error);

private:
    QMqttClient *m_client;

    // --- 请在此处填入你的 OneNET 设备信息 ---
    const QString m_productId = "your_productid";      // 产品ID
    const QString m_deviceName = "imx6ull";        // 设备名称
    
    // 【重要】请使用 Token 工具生成的完整 Token 字符串
    const QString m_token = "your_mqtt_token";

    // Topic 定义
    QString m_topicSet;       // 订阅：接收控制
    QString m_topicPost;      // 发布：上报数据
    QString m_topicReply;     // 发布：回复回执
    QString m_topicGet;       // 订阅：查询数据

};

#endif // MQTTCLIENT_H
