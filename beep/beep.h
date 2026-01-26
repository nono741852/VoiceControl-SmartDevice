#ifndef BEEP_H
#define BEEP_H

#include <QObject>
#include <QFile>
#include <QTime>

class Beep : public QObject
{
    Q_OBJECT
public:
    explicit Beep(QObject *parent = nullptr);
    ~Beep();
    /*设置蜂鸣器状态*/
    void setbeepState(bool on);

    /* 获取BEEP的状态 */
    bool getBeepState();
    /*触发报警模式*/
//    bool setAlarmModel(int mode = 1);

//    /*停止报警*/
//    void stopAlarm();
private:
    /* 文件 */
    QFile file;

};

#endif // BEEP_H
