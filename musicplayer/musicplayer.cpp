/******************************************************************
* @projectName   musicplayer
* @brief         musicplayer.cpp
* @author        Rong Yannan
* @email         ryn18247501992@163.com
* @date          2026-01-20
*******************************************************************/
#include "musicplayer.h"
#include <QDebug>
#include <QCoreApplication>//为qt控制应用程序生命周期提供了基本的功能
#include <QDir>//提供了对目录进行操作的功能
#include <QFileInfoList>//提供了对文件路径进行操作的功能


MusicPlayer::MusicPlayer(QObject *parent) : QObject(parent)
{
    m_player = new QMediaPlayer(this);//创建音乐播放器对象
    m_playlist = new QMediaPlaylist(this);//创建音乐播放列表对象
    /* 确保列表是空的 */
    m_playlist->clear();
    //将播放列表设置给音乐播放器
    m_player->setPlaylist(m_playlist);
    /* 设置播放模式，Loop是列循环 */
    m_playlist->setPlaybackMode(QMediaPlaylist::Loop);
    /* 扫描歌曲 */
    scanSongs();
    // 设置默认音量
    m_player->setVolume(15);
    
    qDebug() << "音乐播放器初始化完成";
    qDebug() << "音乐播放器初始化完成，播放列表歌曲数量：" << m_playlist->mediaCount();
}

MusicPlayer::~MusicPlayer()
{
    // 停止音乐播放
    m_player->stop();
    delete m_player;
    delete m_playlist;
    qDebug() << "音乐播放器已销毁";
}

// 设置音乐文件列表
void MusicPlayer::setMusicList(const QStringList &musicList)
{
    // 清空当前播放列表
    m_playlist->clear();
    // 添加新的音乐文件到播放列表
    foreach (const QString &musicPath, musicList) {
        addMusic(musicPath);
    }
    qDebug() << "音乐文件列表已设置";
}
    
// 播放音乐
void MusicPlayer::play()
{
    if (m_playlist->mediaCount() > 0) {
        m_player->play();
        qDebug() << "开始播放音乐";
    } else {
        qDebug() << "播放列表为空，无法播放";
    }
}
    
// 暂停音乐
void MusicPlayer::pause()
{
    if (m_player->state() == QMediaPlayer::PlayingState) {
        m_player->pause();
        qDebug() << "音乐已暂停";
    } else {
        qDebug() << "音乐未在播放状态，无法暂停";
    }
}

// 停止音乐
void MusicPlayer::stop()
{
    if (m_player->state() != QMediaPlayer::StoppedState) {
        m_player->stop();
        qDebug() << "音乐已停止";
    } else {
        qDebug() << "音乐未在播放状态";
    }
    }

// 下一首
void MusicPlayer::next()
{
    m_player->stop();// 先停止当前播放
    if (m_playlist->mediaCount() > 0) {
        m_playlist->next();
        m_player->play();// 播放下一首
        qDebug() << "播放下一首音乐";
    } else {
        qDebug() << "播放列表为空，无法播放下一首";
    }
}
    
// 上一首
void MusicPlayer::previous()
{
    m_player->stop();// 先停止当前播放
    if (m_playlist->mediaCount() > 0) {
        m_playlist->previous();
        m_player->play();// 播放上一首
        qDebug() << "播放上一首音乐";
    } else {
        qDebug() << "播放列表为空，无法播放上一首";
    }
}

// 设置音量 (0-100)
void MusicPlayer::setVolume(int volume)
{
    // 确保音量在0-100之间
    volume = qBound(0, volume, 100);
    m_player->setVolume(volume);
    qDebug() << "设置音量为:" << volume;
    
}

// 获取当前播放状态返回是否正在播放音乐
bool MusicPlayer::isPlaying() const
{
    return m_player->state() == QMediaPlayer::PlayingState;
}

// 获取当前播放的音乐名称
QString MusicPlayer::getCurrentMusicName() const
{
    int currentIndex = m_playlist->currentIndex();
    if (currentIndex >= 0 && currentIndex < m_mediaInfoList.count()) {
        return m_mediaInfoList.at(currentIndex).fileName;
    }
    return QString();
}

// 添加单个音乐文件到播放列表
void MusicPlayer::addMusic(const QString &musicPath)
{
    QUrl musicUrl;
    
    // 检查是否为资源文件路径
    if (musicPath.startsWith(":/")) {
        musicUrl = QUrl(musicPath);
    } else {
        // 绝对路径或相对路径
        musicUrl = QUrl::fromLocalFile(musicPath);
    }
    
    m_playlist->addMedia(musicUrl);
    qDebug() << "添加音乐文件:" << musicUrl;
}

void MusicPlayer::scanSongs()
{
    QDir dir(QCoreApplication::applicationDirPath() + "/myMusic");
    QDir dirbsolutePath(dir.absolutePath());
    /* 如果目录存在 */
    if (dirbsolutePath.exists())
    {
        /* 定义过滤器 */
        QStringList filter;
        /* 包含所有.wav后缀的文件 */
        filter << "*.wav";
        /* 获取该目录下的所有文件 */
        QFileInfoList files =
            dirbsolutePath.entryInfoList(filter, QDir::Files);
        /* 遍历 */
        for (int i = 0; i < files.count(); i++)
        {
            MediaObjectInfo info;
            /* 使用utf-8编码 */
            QString fileName = QString::fromUtf8(files.at(i)
                                                     .fileName()
                                                     .replace(".wav", "")
                                                     .toUtf8()
                                                     .data());
            info.fileName = fileName + "\n" + fileName.split("-").at(1);
            info.filePath = QString::fromUtf8(files.at(i)
                                                  .filePath()
                                                  .toUtf8()
                                                  .data());
            /* 媒体列表添加歌曲 */
            if (m_playlist->addMedia(
                    QUrl::fromLocalFile(info.filePath)))
            {
                /* 添加到容器数组里储存 */
                m_mediaInfoList.append(info);
            }
            else
            {
                qDebug() << m_playlist->errorString()
                                .toUtf8()
                                .data()
                         << endl;
                qDebug() << "  Error number:"
                         << m_playlist->error()
                         << endl;
            }
        }
    }
}
