/******************************************************************
Copyright © Deng Zhimao Co., Ltd. 1990-2021. All rights reserved.
* @projectName   sensor
* @brief         sensorthread.cpp
* @author        rong yannan
* @email         ryn18247501992@163.com
* @date          2026-02-05
*******************************************************************/
#ifndef CLOUDAGENT_H
#define CLOUDAGENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class CloudAgent : public QObject
{
    Q_OBJECT

public:
    explicit CloudAgent(QObject *parent = nullptr);
    ~CloudAgent();

    // 设置云端Agent的API地址
    void setApiUrl(const QString &url);
    void setApiKey(const QString &apiKey);
    // 从环境变量加载API Key
    bool loadApiKeyFromEnv(const QString &envVarName = "DIFY_API_KEY");
    // 发送语音识别文本到云端Agent获取控制指令
    void requestCommand(const QString &speechText);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_apiUrl;
    QString m_apiKey;
    // 解析云端返回的JSON指令
    bool parseCommandResponse(const QByteArray &response, QString &type, QString &target, QString &command, QJsonObject &params);
    // 从Dify响应中提取JSON格式的控制指令
    bool extractControlCommand(const QString &text, QJsonObject &commandObj);
private slots:
    // 处理网络响应
    void onNetworkReplyFinished();

signals:
    // 发送解析后的设备控制指令
    void commandReceived(QString type, QString target, QString command, QJsonObject params);
    
    // 网络错误信号
    void networkError(const QString &error);
};

#endif // CLOUDAGENT_H