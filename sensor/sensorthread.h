#ifndef SENSORTHREAD_H
#define SENSORTHREAD_H

#include <QThread>
#include <QMutex>
#include "ap3216c.h"
class SensorThread : public QThread
{
    Q_OBJECT
public:
    explicit SensorThread(QObject *parent = nullptr);
    ~SensorThread();

    void startCapture();//启动数据采集
    void stopCapture(); //停止数据采集

    // 获取传感器数据
    QString getAlsData();//光强数据
    QString getPsData(); //接近距离数据
    QString getIrData(); //红外强度数据

signals:
    // 数据更新信号
    void sensorDataUpdated();

protected:
    // 线程运行函数
    void run() override;//通过override显式声明重写run函数

private:
    Ap3216c *m_sensor;
    bool m_isRunning;
    QMutex m_mutex; // 用于线程安全的数据访问
    QString m_alsData;
    QString m_psData;
    QString m_irData;
    const int m_updateInterval = 1000; // 更新间隔(毫秒)

};

#endif // SENSORTHREAD_H
