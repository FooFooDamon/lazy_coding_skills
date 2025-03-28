/* SPDX-License-Identifier: Apache-2.0 */

/*
 * Class derived from QVideoWidget for playing video.
 *
 * Copyright (c) 2025 Man Hung-Coeng <udc577@126.com>
 * All rights reserved.
 */

#ifndef __QVIDEOCANVAS_HPP__
#define __QVIDEOCANVAS_HPP__

#include <QVideoWidget> // QT += multimediawidgets

class QMediaPlayer;
class QMenu;

class QVideoCanvas : public QVideoWidget
{
    Q_OBJECT

public:
    QVideoCanvas() = delete;
    QVideoCanvas(QWidget *parent = nullptr);
    Q_DISABLE_COPY_MOVE(QVideoCanvas);
    ~QVideoCanvas();

public:
    inline void setPlayer(QMediaPlayer *player)
    {
        this->player_ = player;
    }

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

public Q_SLOTS:
    void play(void);
    void pause(void);
    void stop(void);

private:
    QMediaPlayer *player_;
    std::shared_ptr<QMenu> context_menu_;
    std::shared_ptr<QAction> play_action_;
    std::shared_ptr<QAction> pause_action_;
    std::shared_ptr<QAction> stop_action_;
};

#endif /* #ifndef __QVIDEOCANVAS_HPP__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2025-03-27, Man Hung-Coeng <udc577@126.com>:
 *  01. Initial commit.
 *
 * >>> 2025-03-28, Man Hung-Coeng <udc577@126.com>:
 *  01. Change the type of member variable player_
 *      from std::shard_ptr to raw pointer.
 */

