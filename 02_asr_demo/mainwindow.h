/******************************************************************
Copyright © Deng Zhimao Co., Ltd. 1990-2021. All rights reserved.
* @projectName   02_asr_demo
* @brief         mainwindow.h
* @author        Rong Yannan
* @email         ryn18247501992@163.com
* @date          2026-01-22
*******************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QMovie>
#include <QTimer>
#include <QHBoxLayout>
#include "../audiorecorder/audiorecorder.h"// 引入录音类
#include "../asr/asr.h"// 引入ASR类
#include "../led/led.h"// 引入LED类
#include "../beep/beep.h"// 引入蜂鸣器类
#include "../sensor/ap3216c.h"// 引入AP3216C传感器类
#include "../sensor/sensorthread.h"// 引入传感器线程类
#include "../musicplayer/musicplayer.h"// 引入音乐播放器类

class Asr;
class AudioRecorder;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    //UI组件：
    /* 主Widget */
    QWidget *mainWidget;

    /* gif底下的Widget */
    QWidget *movieWidget;


    /* gif动画画布 */
    QLabel *movieLabel;

    /* 用于显示识别结果 */
    QLabel *textLabel;

    QLabel *sensorDataLabel; // 新增：显示传感器数据的标签
    /* 垂直布局 */
    QVBoxLayout *vBoxLayout;

    /* 水平布局 */
    QHBoxLayout *hBoxLayout;



    /* 用于显示GIF动画 */
    QMovie *myMovie;

    /* 定时器 */
    QTimer *timer1;
    QTimer *timer2;
    QTimer *timer3;

    /* 事件过滤器 */
    bool eventFilter(QObject *watched, QEvent *event);

    /* 录音类 */
    AudioRecorder *myAudioRecorder;

    /* 主意识别类 */
    Asr *myAsr;

    /* 开发板LED */
    Led *myLed;
    SensorThread *m_sensorThread; // 新增：传感器采集线程
    /* 开发板蜂鸣器 */
    Beep *myBeep;
    // 传感器数据显示状态
    bool m_showingSensorData;
    /* 音乐播放器 */
    MusicPlayer *myMusicPlayer;


private slots:
    void onTimer1TimeOut();
    void onTimer2TimeOut();
    void onTimer3TimeOut();
    void onAsrReadyData(QString);
    void onSensorDataUpdated(); // 新增：传感器数据更新槽函数

};
#endif // MAINWINDOW_H
