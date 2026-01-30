#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QStringList>
#include <QFileInfo>

// 添加媒体对象信息结构体定义
typedef struct MediaObjectInfo {
    QString fileName;
    QString filePath;
} MediaObjectInfo;


class MusicPlayer : public QObject
{
    Q_OBJECT
public:
    explicit MusicPlayer(QObject *parent = nullptr);
    ~MusicPlayer();

    // 设置音乐文件列表
    void setMusicList(const QStringList &musicList);
    
    // 播放音乐
    void play();
    
    // 暂停音乐
    void pause();
    
    // 停止音乐
    void stop();
    
    // 下一首
    void next();
    
    // 上一首
    void previous();
    
    // 设置音量 (0-100)
    void setVolume(int volume);
    
    // 获取当前播放状态
    bool isPlaying() const;

    // 获取当前播放的音乐名称
    QString getCurrentMusicName() const;

    // 添加单个音乐文件到播放列表
    void addMusic(const QString &musicPath);

    // 扫描音乐文件 
    void scanSongs();
private:
    QMediaPlayer *m_player;// 音乐播放器
    QMediaPlaylist *m_playlist;// 音乐播放列表
    QList<MediaObjectInfo> m_mediaInfoList; // 音乐文件列表

};

#endif // MUSICPLAYER_H
