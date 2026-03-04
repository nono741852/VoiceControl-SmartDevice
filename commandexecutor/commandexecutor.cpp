#include "commandexecutor.h"
#include <QTimer>
#include <QDebug>

CommandExecutor::CommandExecutor(QObject *parent)
    : QObject(parent),
      m_led(nullptr),
      m_beep(nullptr),
      m_musicPlayer(nullptr),
      m_sensorThread(nullptr),
      m_mqttClient(nullptr)
{
    initCommandMaps(); // 初始化命令映射表
}
// 设置设备引用
void CommandExecutor::setDevices(Led *led, Beep *beep, MusicPlayer *musicPlayer, 
                                  SensorThread *sensorThread, MqttClient *mqttClient)
{
    m_led = led;
    m_beep = beep;
    m_musicPlayer = musicPlayer;
    m_sensorThread = sensorThread;
    m_mqttClient = mqttClient;
}


CommandExecutor::~CommandExecutor()
{
    // 析构函数中不需要删除设备对象，因为它们由 MainWindow 创建并管理
}


//解析json控制命令
void CommandExecutor::executeCommand(const QString &type, const QString &target,
                       const QString &command, const QJsonObject &params)
{
    Q_UNUSED(params);
    
    if (type == "control") {
        // 处理控制命令
        QString commandKey = QString("%1_%2").arg(target, command);
        
        if (m_controlCommands.contains(commandKey)) {
            m_controlCommands[commandKey]();//调用对应的控制命令函数
        } else {
            qDebug() << "未知的控制命令:" << commandKey;
            emit statusUpdate("未知的控制命令");
        }}
    else if (type == "query") {
        // 处理查询命令
        QString commandKey = QString("%1_%2").arg(target, command);
        
        if (m_queryCommands.contains(commandKey)) {
            m_queryCommands[commandKey]();
        } else {
            qDebug() << "未知的查询命令:" << commandKey;
            emit statusUpdate("未知的查询命令");
        }
    } 
    else {
        qDebug() << "未知的命令:" << type;
        emit statusUpdate("未知的命令");
    }
}
// 初始化命令映射表（可根据实际需求添加更多命令）
void CommandExecutor::initCommandMaps()
{
    // 控制命令映射表
    m_controlCommands["led_on"] = [this]() { ledOn(); };
    m_controlCommands["led_off"] = [this]() { ledOff(); };
    m_controlCommands["beep_alarm"] = [this]() { beepOn(); };
    m_controlCommands["beep_off"] = [this]() { beepOff(); };
    m_controlCommands["music_play"] = [this]() { musicPlay(); };
    m_controlCommands["music_pause"] = [this]() { musicPause(); };
    m_controlCommands["music_stop"] = [this]() { musicStop(); };
    m_controlCommands["music_next"] = [this]() { musicNext(); };
    m_controlCommands["music_prev"] = [this]() { musicPrevious(); };

    // 查询命令映射表
    m_queryCommands["sensor_getdata"] = [this]() { querySensorData(); };
}

// LED控制命令
void CommandExecutor::ledOn()
{
    if (m_led) {
        m_led->setLedState(true);
        if (m_mqttClient) {
            m_mqttClient->publishDeviceState("led", true);
        }
        emit statusUpdate("LED已开启");
    }
}

void CommandExecutor::ledOff()
{
    if (m_led) {
        m_led->setLedState(false);
        if (m_mqttClient) {
            m_mqttClient->publishDeviceState("led", false);
        }
        emit statusUpdate("LED已关闭");
    }
}

// 蜂鸣器控制命令实现
void CommandExecutor::beepOn()
{
    if (m_beep) {
        m_beep->setbeepState(true);
        if (m_mqttClient) {
            m_mqttClient->publishDeviceState("beep", true);
        }
        emit statusUpdate("蜂鸣器已开启");
    }
}
void CommandExecutor::beepOff()
{
    if (m_beep) {
        m_beep->setbeepState(false);
        if (m_mqttClient) {
            m_mqttClient->publishDeviceState("beep", false);
        }
        emit statusUpdate("蜂鸣器已关闭");
    }
}
// 音乐播放器控制命令实现
void CommandExecutor::musicPlay()
{
    if (m_musicPlayer) {
        m_musicPlayer->play();
        if (m_mqttClient) {
            m_mqttClient->publishDeviceState("music", true);
        }
        QString currentMusic = m_musicPlayer->getCurrentMusicName();
        emit statusUpdate(QString("正在播放: %1").arg(currentMusic));
    }
}

void CommandExecutor::musicPause()
{
    if (m_musicPlayer) {
        m_musicPlayer->pause();
        if (m_mqttClient) {
            m_mqttClient->publishDeviceState("music", false);
        }
        emit statusUpdate("音乐已暂停");
    }
}

void CommandExecutor::musicStop()
{
    if (m_musicPlayer) {
        m_musicPlayer->stop();
        if (m_mqttClient) {
            m_mqttClient->publishDeviceState("music", false);
        }
        emit statusUpdate("音乐已停止");
    }
}

void CommandExecutor::musicNext()
{
    if (m_musicPlayer) {
        m_musicPlayer->next();
        if (m_mqttClient) {
            m_mqttClient->publishDeviceState("music", true);
        }
        QString currentMusic = m_musicPlayer->getCurrentMusicName();
        emit statusUpdate(QString("正在播放: %1").arg(currentMusic));
    }
}

void CommandExecutor::musicPrevious()
{
    if (m_musicPlayer) {
        m_musicPlayer->previous();
        if (m_mqttClient) {
            m_mqttClient->publishDeviceState("music", true);
        }
        QString currentMusic = m_musicPlayer->getCurrentMusicName();
        emit statusUpdate(QString("正在播放: %1").arg(currentMusic));
    }
}

// 传感器查询命令实现
void CommandExecutor::querySensorData()
{
    if (m_sensorThread) {
        if (!m_sensorThread->isRunning()) {
            m_sensorThread->startCapture();
        }
        
        // 获取传感器数据
        QString als = m_sensorThread->getAlsData();
        QString ps = m_sensorThread->getPsData();
        QString ir = m_sensorThread->getIrData();

        QString sensorText = QString("传感器数据：\n光照强度：%1  接近距离：%2  红外数据：%3")
                            .arg(als)
                            .arg(ps)
                            .arg(ir);
        
        emit statusUpdate(sensorText);
    }
}
