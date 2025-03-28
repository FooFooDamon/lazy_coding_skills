/* SPDX-License-Identifier: Apache-2.0 */

/*
 * Class derived from QSlider for mouse reaction.
 *
 * Copyright (c) 2025 Man Hung-Coeng <udc577@126.com>
 * All rights reserved.
 */

#ifndef __QPLAYERSLIDER_HPP__
#define __QPLAYERSLIDER_HPP__

#include <QSlider> // QT += widgets

class QMediaPlayer;

class QPlayerSlider : public QSlider
{
    Q_OBJECT

public:
    QPlayerSlider() = delete;
    QPlayerSlider(QWidget *parent = nullptr);
    Q_DISABLE_COPY_MOVE(QPlayerSlider);
    ~QPlayerSlider();

public:
    enum Error
    {
        ERR_OK,
        ERR_NULL_PLAYER_POINTER,
        ERR_PLAYER_NOT_SET,
        ERR_PLAYER_ALREADY_SET,
    };

    Error setPlayer(QMediaPlayer *player);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void sliderMoved64(qint64 value);

public Q_SLOTS:
    void setProgress(qint64 progress);

private:
    QMediaPlayer *player_;
    uint8_t player_state_;
    bool mouse_released_;
};

#endif /* #ifndef __QPLAYERSLIDER_HPP__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2025-03-28, Man Hung-Coeng <udc577@126.com>:
 *  01. Initial commit.
 */

