/******************************************************************
Copyright © Deng Zhimao Co., Ltd. 1990-2021. All rights reserved.
* @projectName   02_asr_demo
* @brief         mainwindow.cpp
* @author        Rong Yannan
* @email         ryn18247501992@163.com
* @date          2026-01-22
*******************************************************************/
#include "mainwindow.h"
#include <QDebug>
#include <QSound>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setGeometry(0, 0, 800, 480);
    this->setStyleSheet("background:#14161a");
    //ui组件的创建与布局
    mainWidget = new QWidget();
    movieWidget = new QWidget();
    movieLabel = new QLabel();
    textLabel = new QLabel();
    sensorDataLabel = new QLabel(); // 新增：用于显示传感器数据的标签

    movieWidget->setMinimumWidth(240);
    textLabel->setMinimumSize(240, 50);
    textLabel->setStyleSheet("color: white; font-size: 20px");
    textLabel->setText("请点击，开始说话...");
    textLabel->setAlignment(Qt::AlignCenter);

    // 新增：传感器数据标签设置
    sensorDataLabel->setMinimumSize(240, 100);
    sensorDataLabel->setStyleSheet("color: #f8f8f8ff; font-size: 18px");
    sensorDataLabel->setText("传感器数据：\n光照强度：- 接近距离：- 红外数据：-");
    sensorDataLabel->setAlignment(Qt::AlignCenter);
    sensorDataLabel->hide(); // 初始隐藏

    /* BEEP */
    myBeep = new Beep(this);
    /* 安装事件过滤器——使得点击GIF动画时候可以出发事件 */
    movieLabel->installEventFilter(this);
    movieLabel->setFixedSize(240, 240);

    vBoxLayout = new QVBoxLayout();//垂直布局
    hBoxLayout = new QHBoxLayout();//水平布局
    //构建布局层次结构
    vBoxLayout->addWidget(movieWidget);
    vBoxLayout->addWidget(textLabel);
    vBoxLayout->addWidget(sensorDataLabel); // 新增：添加传感器数据标签到布局
    vBoxLayout->setAlignment(Qt::AlignCenter);

    mainWidget->setLayout(vBoxLayout);

    hBoxLayout->addWidget(movieLabel);
    movieWidget->setLayout(hBoxLayout);

    setCentralWidget(mainWidget);//设置主窗口的中央部件

    myMovie = new QMovie(":/gif/voice_effect.gif");
    /* 设置播放速度，值越大越快 */
    myMovie->setSpeed(30);//降低播放速度
    movieLabel->setMovie(myMovie);
    movieLabel->show();
    /* 设置设置化时显示第一帧 */
    myMovie->jumpToNextFrame();

    movieLabel->setAlignment(Qt::AlignHCenter);//设置GIF动画标签水平剧中对齐

    timer1 = new QTimer(this);
    timer2 = new QTimer(this);
    timer3 = new QTimer(this);
    mqttPublishTimer = new QTimer(this);
    connect(timer1, SIGNAL(timeout()), this, SLOT(onTimer1TimeOut()));
    connect(timer2, SIGNAL(timeout()), this, SLOT(onTimer2TimeOut()));
    connect(timer3, SIGNAL(timeout()), this, SLOT(onTimer3TimeOut()));
    connect(mqttPublishTimer, SIGNAL(timeout()), this, SLOT(onMqttPublishTimerTimeout()));
    /* 自定义的录音类 */
    myAudioRecorder = new AudioRecorder(this);
    // 设置 MQTT 发布定时器，每 8秒发布一次传感器数据
    mqttPublishTimer->start(8000);

    /* MQTT客户端 */
    myMqttClient = new MqttClient(this);
    // 连接到 OneNET
    myMqttClient->connectToCloud();

    /* 自定义的百度云识别录音类 */
    myAsr = new Asr(this);

    /* LED */
    myLed = new Led(this);
    /* 音乐播放器 */
    myMusicPlayer = new MusicPlayer(this);

    /* 新增：传感器采集线程 */
    m_sensorThread = new SensorThread(this);
   // qDebug() << "传感器线程创建：" << m_sensorThread;
   // connect(m_sensorThread, SIGNAL(sensorDataUpdated()), this, SLOT(onSensorDataUpdated()));

    // 应用启动时立即启动传感器线程
    m_sensorThread->startCapture();
    qDebug() << "传感器线程启动状态：" << m_sensorThread->isRunning();
    connect(myAsr, SIGNAL(asrReadyData(QString)), this, SLOT(onAsrReadyData(QString)));


     
    // 连接MQTT远程控制信号
    connect(myMqttClient, &MqttClient::remoteSwitchReceived, this, &MainWindow::onRemoteSwitchReceived);

    // 初始发布设备状态
    myMqttClient->publishDeviceState("led", false);
    myMqttClient->publishDeviceState("beep", false);
    myMqttClient->publishDeviceState("music", false);

    /* 云端Agent */
    myCloudAgent = new CloudAgent(this);
    // 设置云端Agent的API地址（可以从环境变量中读取）
    myCloudAgent->setApiUrl("https://api.dify.ai/v1/chat-messages");
    // 设置云端Agent的API密钥（可以从环境变量中读取）
    myCloudAgent->setApiKey("app-WTOtVWnoOqBkgpccMs5KXqww");
    
    connect(myCloudAgent, &CloudAgent::commandReceived, this, &MainWindow::onCommandReceived);
    connect(myCloudAgent, &CloudAgent::networkError, this, &MainWindow::onCloudAgentError);

    /* 指令执行器 */
    myCommandExecutor = new CommandExecutor(this);
    myCommandExecutor->setDevices(myLed, myBeep, myMusicPlayer, m_sensorThread, myMqttClient);//设置指令执行器的设备
    connect(myCommandExecutor, &CommandExecutor::statusUpdate, this, &MainWindow::onStatusUpdate);
}
MainWindow::~MainWindow()
{
     delete m_sensorThread; // 清理传感器线程
     delete myMusicPlayer;  // 清理音乐播放器（如果已创建）
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event){

    if (watched == movieLabel && event->type() == QEvent::MouseButtonPress) {
        myBeep->setbeepState(false);//保证点击开始录音时 停止报警
        sensorDataLabel->hide();//保证点击开始录音时，隐藏传感器数据标签
        // 新增：无论是否在播放，都暂停音乐
        if (myMusicPlayer->isPlaying()) {
            myMusicPlayer->pause();
        }
        QSound::play(":/audio/sound.wav");
        if (myMovie->state() != QMovie::Running) {
            /* 等待QSound播放完,1.5s后再录音 */
            timer2->start(1500);
            textLabel->setText("正在听您说话，请继续...");
            myMovie->start();
        }
    }

    return QWidget::eventFilter(watched, event);
}

/*--------------------------定时器超时槽函数定义-------------------------------*/
void MainWindow::onTimer1TimeOut()
{
    /* 停止录音，8s钟的短语音 */
    myAudioRecorder->stopRecorder();
    textLabel->setText("正在识别，请稍候...");
    timer1->stop();
    myMovie->stop();
    QString fileName = QCoreApplication::applicationDirPath() + "/16k.wav";
    myAsr->getTheResult(fileName);//获取识别结果
    timer3->start(30000);
}

void MainWindow::onTimer2TimeOut()
{
    timer1->start(8000);
    /* 开始录音 */
    myAudioRecorder->startRecorder();
    m_sensorThread->stopCapture();//暂时挂起传感器采集线程，避免录音时的干扰
    timer2->stop();
}

void MainWindow::onTimer3TimeOut()
{
    textLabel->setText("请点击，开始说话...");
    timer3->stop();
}

// 添加新的槽函数用于MQTT数据发布
void MainWindow::onMqttPublishTimerTimeout()
{
    // 定时发布传感器数据到MQTT
    //qDebug() << "--- 达到8s发送间隔，数据上报云端 ---";
    int als = m_sensorThread->getAlsValue();
    int ir = m_sensorThread->getIrValue();
    myMqttClient->publishSensorData(als, ir);
    QString sensorText = QString("传感器数据：\n光照强度：%1 接近距离：%2 红外数据：%3")
                        .arg(m_sensorThread->getAlsData())
                        .arg(m_sensorThread->getPsData())
                        .arg(m_sensorThread->getIrData());
    sensorDataLabel->setText(sensorText);
}
/*------------------------------语音识别函数--------------------------------------*/
void MainWindow::onAsrReadyData(QString str)
{
    m_sensorThread->startCapture();//恢复传感器采集线程
    textLabel->setText("识别结果是:\n" + str);//显示识别结果
    textLabel->adjustSize();//根据文本内容调整标签大小
    // 发送语音识别结果到云端Agent获取控制指令
    myCloudAgent->requestCommand(str);


}


// 处理云端Agent返回的指令
void MainWindow::onCommandReceived(QString type, QString target, QString command, QJsonObject params)
{
    // 执行命令
    myCommandExecutor->executeCommand(type, target, command, params);
}

// 处理云端Agent错误
void MainWindow::onCloudAgentError(const QString &error)
{
    qDebug() << "云端Agent错误:" << error;
    textLabel->setText("云端服务错误: " + error);
    
    // 5秒后恢复默认提示
    QTimer::singleShot(5000, [this]() {
        textLabel->setText("请点击，开始说话...");
    });
}

// 处理指令执行器状态更新
void MainWindow::onStatusUpdate(const QString &message)
{
    // 更新UI显示状态
    textLabel->setText(message);
    
    // 如果是传感器数据查询，显示更长时间
    if (message.contains("传感器数据")) {
        QTimer::singleShot(7000, [this]() {
            textLabel->setText("请点击，开始说话...");
        });
    } else {
        // 其他状态显示5秒
        QTimer::singleShot(5000, [this]() {
            textLabel->setText("请点击，开始说话...");
        });
    }
}

void MainWindow::onSensorDataUpdated()
{
    static QTime lastActionTime = QTime::currentTime();
    // 获取当前时间与上次操作时间的间隔
    int elapsed = lastActionTime.msecsTo(QTime::currentTime());
    // 更新传感器数据显示
    // 限制：每 2 秒（2000ms）才允许处理一次数据（包括更新 UI 和发 MQTT）
    if (elapsed > 2000 || elapsed < 0) {
        QString als = m_sensorThread->getAlsData();
        QString ps = m_sensorThread->getPsData();
        QString ir = m_sensorThread->getIrData();
    

    QString sensorText = QString("传感器数据：\n光照强度：%1 接近距离：%2 红外数据：%3")
                        .arg(als)
                        .arg(ps)
                        .arg(ir);

    sensorDataLabel->setText(sensorText);
    // 更新上次操作时间
    lastActionTime = QTime::currentTime();
    }
  
}

//新增2：处理远程控制指令槽函数
void MainWindow::onRemoteSwitchReceived(QString key, bool value, QString msgId)
{
    qDebug() << "收到远程控制指令：" << key << value << msgId;

    if (key == "led") {
        myLed->setLedState(value);
        myMqttClient->publishDeviceState("led", value);
    } else if (key == "beer") {
        myBeep->setbeepState(value);
        myMqttClient->publishDeviceState("beer", value);
    } else if (key == "music") {
        if (value) {
            myMusicPlayer->play();
        } else {
            myMusicPlayer->pause();
        }
        myMqttClient->publishDeviceState("music", value);
    }

    // 发送回复回执
    myMqttClient->sendCmdReply(msgId);
}