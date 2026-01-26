/******************************************************************
Copyright © Deng Zhimao Co., Ltd. 1990-2021. All rights reserved.
* @projectName   02_asr_demo
* @brief         mainwindow.cpp
* @author        Deng Zhimao
* @email         1252699831@qq.com
* @net           www.openedv.com
* @date          2021-06-04
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
    sensorDataLabel->setStyleSheet("color: #4CAF50; font-size: 16px");
    sensorDataLabel->setText("传感器数据：\n光照强度：-\n接近距离：-\n红外数据：-");
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
    myMovie->setSpeed(50);
    movieLabel->setMovie(myMovie);
    movieLabel->show();
    /* 设置设置化时显示第一帧 */
    myMovie->jumpToNextFrame();

    movieLabel->setAlignment(Qt::AlignHCenter);//设置GIF动画标签水平剧中对齐

    timer1 = new QTimer(this);
    timer2 = new QTimer(this);
    timer3 = new QTimer(this);

    connect(timer1, SIGNAL(timeout()), this, SLOT(onTimer1TimeOut()));
    connect(timer2, SIGNAL(timeout()), this, SLOT(onTimer2TimeOut()));
    connect(timer3, SIGNAL(timeout()), this, SLOT(onTimer3TimeOut()));

    /* 自定义的录音类 */
    myAudioRecorder = new AudioRecorder(this);

    /* 自定义的百度云识别录音类 */
    myAsr = new Asr(this);

    /* LED */
    myLed = new Led(this);

    /* 新增：传感器采集线程 */
    m_sensorThread = new SensorThread(this);
    qDebug() << "传感器线程创建：" << m_sensorThread;
    connect(m_sensorThread, SIGNAL(sensorDataUpdated()), this, SLOT(onSensorDataUpdated()));

    // 应用启动时立即启动传感器线程
    m_sensorThread->startCapture();
    qDebug() << "传感器线程启动状态：" << m_sensorThread->isRunning();
    connect(myAsr, SIGNAL(asrReadyData(QString)), this, SLOT(onAsrReadyData(QString)));
}

MainWindow::~MainWindow()
{
     delete m_sensorThread; // 清理传感器线程
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event){

    if (watched == movieLabel && event->type() == QEvent::MouseButtonPress) {
        myBeep->setbeepState(false);//保证点击开始录音时 停止报警
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

void MainWindow::onTimer1TimeOut()
{
    /* 停止录音，8s钟的短语音 */
    myAudioRecorder->stopRecorder();
    textLabel->setText("正在识别，请稍候...");
    timer1->stop();
    myMovie->stop();
    QString fileName = QCoreApplication::applicationDirPath() + "/16k.wav";
    myAsr->getTheResult(fileName);
    timer3->start(30000);
}

void MainWindow::onTimer2TimeOut()
{
    timer1->start(8000);
    /* 开始录音 */
    myAudioRecorder->startRecorder();
    timer2->stop();
}

void MainWindow::onTimer3TimeOut()
{
    textLabel->setText("请点击，开始说话...");
    timer3->stop();
}

void MainWindow::onAsrReadyData(QString str)
{
    qDebug() << "传感器线程启动状态：" << m_sensorThread->isRunning();
    if (str.contains("开灯"))
        myLed->setLedState(true);
    else if (str.contains("关灯"))
        myLed->setLedState(false);
    else if (str.contains("报警")||str.contains("鸣笛"))
        myBeep->setbeepState(true);
    // 新增：处理传感器数据查询
    else if (str.contains("光照强度") || str.contains("传感器数据") || str.contains("环境数据")) {
        qDebug() << "执行传感器数据查询";
        // 确保线程已启动
        if (!m_sensorThread) {
            qDebug() << "传感器线程未初始化";
            return;
        }
        
        if (!m_sensorThread->isRunning()) {
            m_sensorThread->startCapture();
            qDebug() << "启动传感器线程";
        }
         // 立即获取并打印数据
        QString alsData = m_sensorThread->getAlsData();
        qDebug() << "获取的光照强度：" << alsData;
        // qDebug() <<"传感器数据"<<m_sensorThread->getAlsData();

        //更新UI显示传感器数据
        m_showingSensorData = true;
        sensorDataLabel->show(); // 显示传感器数据标签
        textLabel->setText("正在获取环境数据...");

        // 更新传感器数据显示
        QString als = m_sensorThread->getAlsData();
        QString ps = m_sensorThread->getPsData();
        QString ir = m_sensorThread->getIrData();

        QString sensorText = QString("传感器数据：\n光照强度：%1\n接近距离：%2\n红外数据：%3")
                        .arg(als)
                        .arg(ps)
                        .arg(ir);
        
        sensorDataLabel->setText(sensorText);
        textLabel->setText("环境数据已更新");


        // 5秒后自动停止显示传感器数据
        QTimer::singleShot(5000, [this]() {
        m_showingSensorData = false;
        sensorDataLabel->hide();
        textLabel->setText("请点击，开始说话...");});
       }
    else {
           // 普通语音识别结果
           qDebug() << "执行普通识别结果处理";
           textLabel->setText("识别结果是:\n" + str);
           textLabel->adjustSize();
           m_showingSensorData = false;
           sensorDataLabel->hide();
       }

//    textLabel->setText("识别结果是:\n" + str);
//    textLabel->adjustSize();
}


void MainWindow::onSensorDataUpdated()
{
    // 更新传感器数据显示
    QString als = m_sensorThread->getAlsData();
    QString ps = m_sensorThread->getPsData();
    QString ir = m_sensorThread->getIrData();

    QString sensorText = QString("传感器数据：\n光照强度：%1\n接近距离：%2\n红外数据：%3")
                        .arg(als)
                        .arg(ps)
                        .arg(ir);

    sensorDataLabel->setText(sensorText);

    // 如果正在显示传感器数据，更新textLabel
    if (m_showingSensorData) {
        textLabel->setText("环境数据已更新");
    }
}
