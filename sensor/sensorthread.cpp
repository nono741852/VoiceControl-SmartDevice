/******************************************************************
Copyright © Deng Zhimao Co., Ltd. 1990-2021. All rights reserved.
* @projectName   sensor
* @brief         sensorthread.cpp
* @author        rong yannan
* @email         ryn18247501992@163.com
* @date          2026-01-25
*******************************************************************/
#include "sensorthread.h"
#include <QDebug>
SensorThread::SensorThread(QObject *parent)
    : QThread(parent),          //调用父类QThread的构造函数
      m_sensor(new Ap3216c(this)),//创建ap3216c传感器对象，this作为父对象
      m_isRunning(false),        //初始化运行标志为false
      m_alsValue(0),            //初始化光强数值
      m_irValue(0)              //初始化红外数值
{}

//析构函数
SensorThread::~SensorThread()
{
    stopCapture();//停止数据采集
    wait(); // 等待线程结束
    delete m_sensor;//删除传感器对象
}

//开始采集数据函数
void SensorThread::startCapture()
{
    QMutexLocker locker(&m_mutex);//自动上锁，防止多线程竞争
    m_isRunning = true;           //设置运行标志
    qDebug() << "启动传感器数据采集线程";
    if (!isRunning()) {            //检查线程是否已经在运行
        start();                    //调用线程
    }
}
//停止数据采集函数
void SensorThread::stopCapture()
{
    QMutexLocker locker(&m_mutex);
    m_isRunning = false;
}



//获取数据函数
QString SensorThread::getAlsData()
{
    QMutexLocker locker(&m_mutex);//上锁保证读取数据时候不被修改
    return m_alsData;
}

QString SensorThread::getPsData()
{
    QMutexLocker locker(&m_mutex);
    return m_psData;
}

QString SensorThread::getIrData()
{
    QMutexLocker locker(&m_mutex);
    return m_irData;
}


//获取数据函数（数值形式）
int SensorThread::getAlsValue() 
{
    QMutexLocker locker(&m_mutex);
    return m_alsValue;
}

int SensorThread::getIrValue() 
{
    QMutexLocker locker(&m_mutex);
    return m_irValue;
}

void SensorThread::run()
{
    while (true) {
        {
            QMutexLocker locker(&m_mutex);
            if (!m_isRunning) {
                break;
            }//离开作用域自动解锁
        }

        // 采集传感器数据（无锁状态下，避免阻塞传感器）
        QString als = m_sensor->readAlsData();
        QString ps = m_sensor->readPsData();
        QString ir = m_sensor->readIrData();

        // 更新数据
        {
            QMutexLocker locker(&m_mutex);
            m_alsData = als;
            m_psData = ps;
            m_irData = ir;

            // 转换为数值形式——为了上传到MQTT
            m_alsValue = als.toInt();
            m_irValue = ir.toInt();
        }//离开作用域自动解锁

        // 发送数据更新信号
        emit sensorDataUpdated();

        // 休眠指定时间
        msleep(m_updateInterval);
    }
}
