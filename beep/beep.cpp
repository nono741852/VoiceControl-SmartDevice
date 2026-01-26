/******************************************************************
* @projectName   beep
* @brief         beep.cpp
* @author        Rong Yannan
* @email         ryn18247501992@qq.com
* @date          2026-01-22
*******************************************************************/
#include "beep.h"
#include <QDebug>
Beep::Beep(QObject *parent)
{
    this->setParent(parent);
    /* 开发板的蜂鸣器接口 */
    file.setFileName("/sys/devices/platform/leds/leds/beep/brightness");
    if(!file.exists()){
        qDebug()<<"未检测到蜂鸣器设备";
    }
}

Beep::~Beep()
{
    setbeepState(false);//析构时关闭蜂鸣器
}

void Beep::setbeepState(bool on)
{
    qDebug()<<on<<endl;

    /* 如果文件不存在，则返回 */
    if (!file.exists())
        return;
    if(!file.open(QIODevice::ReadWrite))
        qDebug()<<file.errorString();

    QByteArray buf[2] = {"0", "1"};

    /* 根据状态控制蜂鸣器报警 */
    if (on)
        file.write(buf[1]);
    else
        file.write(buf[0]);

    /* 关闭文件 */
    file.close();
    getBeepState();
}

bool Beep::getBeepState()
{
    /* 如果文件不存在，则返回 */
    if (!file.exists())
        return false;

    if(!file.open(QIODevice::ReadWrite))
        qDebug()<<file.errorString();

    QTextStream in(&file);

    /* 读取文件所有数据 */
    QString buf = in.readLine();

    /* 打印出读出的值 */
    qDebug()<<"buf: "<<buf<<endl;
    file.close();
    if (buf == "1") {
        qDebug()<<"开启报警模型";
        return true;
    } else {
        qDebug()<<"已经关闭报警模式22";
        return false;
    }
}
/*触发报警模式*/
//bool setAlarmModel(int mode = 1);

/*停止报警*/
//void stopAlarm();
