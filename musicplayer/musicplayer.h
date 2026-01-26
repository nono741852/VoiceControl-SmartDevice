#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QObject>

class musicplayer : public QObject
{
    Q_OBJECT
public:
    explicit musicplayer(QObject *parent = nullptr);

signals:

};

#endif // MUSICPLAYER_H
