#include "cloudagent.h"
#include <QJsonParseError>
#include <QDebug>
#include <QRegularExpression>
#include <QSettings>

CloudAgent::CloudAgent(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_apiUrl("https://api.dify.ai/v1/chat-messages") // Dify API默认地址
{
    // 尝试从环境变量加载API Key
    loadApiKeyFromEnv();
}

CloudAgent::~CloudAgent()
{
}

void CloudAgent::setApiUrl(const QString &url)
{
    m_apiUrl = url;
}

void CloudAgent::setApiKey(const QString &apiKey)
{
    if (apiKey.isEmpty()) {
        qWarning() << "API密钥为空";
        return;
    }
    
    // 验证API密钥格式
    if (!apiKey.startsWith("app-")) {
        qWarning() << "API密钥格式不正确，应该以'app-'开头";
    }
    m_apiKey = apiKey;
}

bool CloudAgent::loadApiKeyFromEnv(const QString &envVarName)
{
    QByteArray apiKey = qgetenv(envVarName.toLocal8Bit());
    if (!apiKey.isEmpty()) {
        m_apiKey = QString(apiKey);
        qDebug() << "从环境变量加载API Key成功:" << envVarName;
        return true;
    } else {
        qWarning() << "环境变量" << envVarName << "未设置!";
        return false;
    }
}

void CloudAgent::requestCommand(const QString &speechText)
{
    if (m_apiUrl.isEmpty()) {
        emit networkError("API URL未设置");
        return;
    }
    
    if (m_apiKey.isEmpty()) {
        emit networkError("API Key未设置，请通过setApiKey设置或设置DIFY_API_KEY环境变量");
        return;
    }

    // 构建请求JSON
    QJsonObject requestObj;
    requestObj["inputs"] = QJsonObject(); // 空的inputs对象
    requestObj["query"] = speechText;
    requestObj["response_mode"] = "blocking"; // 使用阻塞模式，而不是流式模式
    requestObj["conversation_id"] = QJsonValue::Null; // 可以保存会话ID以保持上下文
    requestObj["user"] = "imx6ull_001"; // 设备唯一标识
    
    QJsonDocument doc(requestObj);
    QByteArray requestData = doc.toJson();
    // 打印请求内容用于调试
    qDebug() << "发送API请求:" << QString(requestData);
    // 创建网络请求
    QNetworkRequest request;
    request.setUrl(m_apiUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    // 设置Authorization头
    QByteArray authHeader = "Bearer " + m_apiKey.toUtf8();
    request.setRawHeader("Authorization", authHeader);
    
    // 发送POST请求
    QNetworkReply *reply = m_networkManager->post(request, requestData);
    connect(reply, &QNetworkReply::finished, this, &CloudAgent::onNetworkReplyFinished);
}

void CloudAgent::onNetworkReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
     // 读取响应数据
    QByteArray responseData = reply->readAll();
    
    // 打印响应状态码和内容用于调试
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    // qDebug() << "HTTP状态码:" << statusCode;
    // qDebug() << "响应内容:" << QString(responseData);

    // 检查网络错误
    if (reply->error() != QNetworkReply::NoError) {
        emit networkError(QString("网络错误: %1").arg(reply->errorString()));

        if (statusCode >= 400) {
            QString errorMsg = QString(" (HTTP %1)").arg(statusCode);
            
            // 尝试解析错误响应
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
            if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject responseObj = doc.object();
                if (responseObj.contains("message")) {
                    errorMsg += QString(" - %1").arg(responseObj["message"].toString());
                }
            }
            emit networkError(errorMsg);
        }
        
        reply->deleteLater();
        return;
    }
    
    
    
    // 解析响应
    QString type, target, command;
    QJsonObject params;
    
    if (parseCommandResponse(responseData, type, target, command, params)) {
        emit commandReceived(type, target, command, params);
    } else {
        emit networkError("解析云端响应失败");
    }
    
    reply->deleteLater();
}

bool CloudAgent::parseCommandResponse(const QByteArray &response, QString &type, QString &target, QString &command, QJsonObject &params)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析错误:" << parseError.errorString();
        return false;
    }
    
    if (!doc.isObject()) {
        qDebug() << "响应不是有效的JSON对象";
        return false;
    }
    
    QJsonObject responseObj = doc.object();
    
    // Dify API响应格式检查
    if (!responseObj.contains("answer")) {
        qDebug() << "Dify响应缺少answer字段";
        return false;
    }
    
    QString answer = responseObj["answer"].toString();
    qDebug() << "Dify回答:" << answer;
    
    // 尝试从回答中提取JSON格式的控制指令
    QJsonObject commandObj;
    if (extractControlCommand(answer, commandObj)) {
        // 解析控制指令
        type = commandObj["type"].toString();
        target = commandObj["target"].toString();
        command = commandObj["command"].toString();
        
        if (commandObj.contains("params")) {
            params = commandObj["params"].toObject();
        }
        
        return true;
    }
    
    // 如果无法提取JSON指令，则尝试简单的文本解析
    answer = answer.toLower();
    
    // 简单的文本解析逻辑，作为后备方案
    if (answer.contains("开灯") || answer.contains("打开灯")) {
        type = "control";
        target = "led";
        command = "on";
        return true;
    } else if (answer.contains("关灯") || answer.contains("关闭灯")) {
        type = "control";
        target = "led";
        command = "off";
        return true;
    } else if (answer.contains("播放音乐") || answer.contains("开始播放")) {
        type = "control";
        target = "music";
        command = "play";
        return true;
    } else if (answer.contains("暂停音乐") || answer.contains("暂停播放")) {
        type = "control";
        target = "music";
        command = "pause";
        return true;
    } else if (answer.contains("停止音乐") || answer.contains("停止播放")) {
        type = "control";
        target = "music";
        command = "stop";
        return true;
    } else if (answer.contains("下一首") || answer.contains("下一曲")) {
        type = "control";
        target = "music";
        command = "next";
        return true;
    } else if (answer.contains("上一首") || answer.contains("上一曲")) {
        type = "control";
        target = "music";
        command = "prev";
        return true;
    } else if (answer.contains("报警") || answer.contains("警报")) {
        type = "control";
        target = "buzzer";
        command = "alarm";
        return true;
    } else if (answer.contains("传感器") || answer.contains("环境")) {
        type = "query";
        target = "sensor";
        command = "get_all";
        return true;
    }
    
    qDebug() << "无法解析Dify回答为控制指令:" << answer;
    return false;
}

bool CloudAgent::extractControlCommand(const QString &text, QJsonObject &commandObj)
{
    QJsonParseError parseError;
    // 尝试从文本中提取JSON格式的控制指令
    // 使用正则表达式查找JSON对象
    QRegularExpression jsonRegex(R"(\{[^{}]*"type"\s*:\s*"[^"]*"[^{}]*"target"\s*:\s*"[^"]*"[^{}]*"command"\s*:\s*"[^"]*"[^{}]*\})");
    QRegularExpressionMatch match = jsonRegex.match(text);
    
    if (match.hasMatch()) {
        QString jsonStr = match.captured(0);
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseError);
        
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            commandObj = doc.object();
            
            // 确保必需字段存在
            if (commandObj.contains("type") && commandObj.contains("target") && commandObj.contains("command")) {
                qDebug() << "成功提取控制指令:" << jsonStr;
                return true;
            }
        }
    }
    int startIndex = text.indexOf('{');
    int endIndex = text.lastIndexOf('}');
    
    if (startIndex != -1 && endIndex != -1 && endIndex > startIndex) {
        QString jsonString = text.mid(startIndex, endIndex - startIndex + 1);
        qDebug() << "提取到JSON子串:" << jsonString;
        
        QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);
        
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject tempObj = doc.object();
            
            // 检查是否包含必需字段
            if (tempObj.contains("type") && tempObj.contains("target") && tempObj.contains("command")) {
                commandObj = tempObj;
                qDebug() << "成功提取控制指令(JSON子串解析):" << jsonString;
                return true;
            }
        }
    }
    return false;
}