/******************************************************************
Copyright © Deng Zhimao Co., Ltd. 1990-2021. All rights reserved.
* @projectName   commandexecutor
* @brief         commandexecutor.h
* @author        rong yannan
* @email         ryn18247501992@163.com
* @date          2026-02-05
* @version       1.0
*******************************************************************/

#ifndef COMMANDEXECUTOR_H
#define COMMANDEXECUTOR_H

#include <QObject>
#include <QJsonObject>
#include <QDebug>
#include <QMap> // 引入QMap类,用于存储键值对
#include <QVariant>// 引入QVariant类,用于存储任意类型的数据
#include "../led/led.h"// 引入LED类
#include "../beep/beep.h"// 引入蜂鸣器类
#include "../sensor/ap3216c.h"// 引入AP3216C传感器类
#include "../sensor/sensorthread.h"// 引入传感器线程类
#include "../musicplayer/musicplayer.h"// 引入音乐播放器类
#include "../mqttclient/mqttclient.h"// 引入MQTT客户端类
#include "../cloudagent/cloudagent.h"// 引入云端Agent类语义解析

class CommandExecutor : public QObject
{
    Q_OBJECT
public:
    explicit CommandExecutor(QObject *parent = nullptr);
    ~CommandExecutor();
    // 设置设备对象引用
    void setDevices(Led *led, Beep *beep, MusicPlayer *musicPlayer, 
                   SensorThread *sensorThread, MqttClient *mqttClient);
    // 执行设备控制命令
    void executeCommand(const QString &type, const QString &target, 
                       const QString &command, const QJsonObject &params);

private:
    // 设备对象引用
    Led *m_led; // LED设备引用
    Beep *m_beep; // 蜂鸣器设备引用
    MusicPlayer *m_musicPlayer; // 音乐播放器设备引用
    SensorThread *m_sensorThread; // 传感器线程设备引用
    MqttClient *m_mqttClient; // MQTT客户端设备引用

    //命令映射表
    QMap<QString, std::function<void()>> m_controlCommands;
    QMap<QString, std::function<void()>> m_queryCommands;

    // 初始化命令映射表
    void initCommandMaps();

    // LED控制命令
    void ledOn();
    void ledOff();

    // 蜂鸣器控制命令
    void beepOn();
    void beepOff();

    // 音乐播放器控制命令
    void musicPlay();
    void musicPause();
    void musicStop();
    void musicNext();
    void musicPrevious();

    // 传感器查询命令
    void querySensorData();
    
    // 其他设备控制/查询命令
    // ...可根据实际需求添加

signals:
    // 发送设备状态更新信号
    void statusUpdate(const QString &message);
};
#endif
