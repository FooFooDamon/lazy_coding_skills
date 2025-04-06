// SPDX-License-Identifier: Apache-2.0

/*
 * Class derived from QSlider for mouse reaction.
 *
 * Copyright (c) 2025 Man Hung-Coeng <udc577@126.com>
 * All rights reserved.
 */

#include "QPlayerSlider.hpp"

#include <QMediaPlayer> // QT += multimedia

#include "qt_print.hpp"

QPlayerSlider::QPlayerSlider(QWidget *parent/* = nullptr*/)
    : QSlider(parent)
    , player_(nullptr)
    , player_state_(QMediaPlayer::StoppedState)
    , mouse_released_(true)
{
}

QPlayerSlider::~QPlayerSlider()
{
    // empty
}

QPlayerSlider::Error QPlayerSlider::setPlayer(QMediaPlayer *player)
{
    if (nullptr != this->player_)
        return ERR_PLAYER_ALREADY_SET;

    if (nullptr != player)
    {
        this->player_ = player;
        QObject::connect(this, SIGNAL(sliderMoved64(qint64)), this->player_, SLOT(setPosition(qint64)));
        QObject::connect(this->player_, SIGNAL(positionChanged(qint64)), this, SLOT(setProgress(qint64)));
    }

    return (nullptr == this->player_) ? ERR_NULL_PLAYER_POINTER : ERR_OK;
}

void QPlayerSlider::mousePressEvent(QMouseEvent *event)/* override*/
{
    QSlider::mousePressEvent(event);
    this->mouse_released_ = false;
    if (nullptr != this->player_)
    {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        this->player_state_ = this->player_->state();
#else
        this->player_state_ = this->player_->playbackState();
#endif
        if (QMediaPlayer::PlayingState == this->player_state_)
            this->player_->pause();
    }
}

void QPlayerSlider::mouseReleaseEvent(QMouseEvent *event)/* override*/
{
    QSlider::mouseReleaseEvent(event);
    this->mouse_released_ = true;
    emit this->sliderMoved64(this->value()/* this->sliderPosition() */);
    this->previousInFocusChain()->setFocus();
    if (nullptr == this->player_)
        qtCErrV(::, "*** Player not set yet!\n");
    else
    {
        if (QMediaPlayer::PlayingState == this->player_state_)
            this->player_->play();
    }
}

void QPlayerSlider::setProgress(qint64 progress)
{
    if (mouse_released_)
        this->setValue(progress);
}

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2025-03-28, Man Hung-Coeng <udc577@126.com>:
 *  01. Initial commit.
 *
 * >>> 2025-04-06, Man Hung-Coeng <udc577@126.com>:
 *  01. Add Qt 6 compatibility.
 */

