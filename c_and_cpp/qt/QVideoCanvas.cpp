// SPDX-License-Identifier: Apache-2.0

/*
 * Class derived from QVideoWidget for playing video.
 *
 * Copyright (c) 2025 Man Hung-Coeng <udc577@126.com>
 * All rights reserved.
 */

#include "QVideoCanvas.hpp"

#include <QKeyEvent> // QT += gui
#include <QMenu> // QT += widgets
#include <QMediaPlayer> // QT += multimedia

#include "qt_print.hpp"

QVideoCanvas::QVideoCanvas(QWidget *parent/* = nullptr*/)
    : QVideoWidget(parent)
{
    this->setAutoFillBackground(true);
    this->player_.reset();
    this->context_menu_ = std::make_shared<QMenu>(this);
    this->play_action_ = std::make_shared<QAction>(QIcon::fromTheme("media-playback-start"), "Play\tSpace", this);
    this->context_menu_->addAction(this->play_action_.get());
    this->pause_action_ = std::make_shared<QAction>(QIcon::fromTheme("media-playback-pause"), "Pause\tSpace", this);
    this->context_menu_->addAction(this->pause_action_.get());
    this->stop_action_ = std::make_shared<QAction>(QIcon::fromTheme("media-playback-stop"), "Stop\tEsc", this);
    this->context_menu_->addAction(this->stop_action_.get());

    QObject::connect(this->play_action_.get(), SIGNAL(triggered(bool)),
        this, SLOT(play(void)));
    QObject::connect(this->pause_action_.get(), SIGNAL(triggered(bool)),
        this, SLOT(pause(void)));
    QObject::connect(this->stop_action_.get(), SIGNAL(triggered(bool)),
        this, SLOT(stop(void)));

    this->setFocusPolicy(Qt::StrongFocus); // required by key event processing
}

QVideoCanvas::~QVideoCanvas()
{
    // Empty.
}

void QVideoCanvas::contextMenuEvent(QContextMenuEvent *event)/* override */
{
    if (this->player_.get())
    {
        auto state = this->player_->state();

        this->play_action_->setVisible(QMediaPlayer::PlayingState != state);
        this->pause_action_->setVisible(QMediaPlayer::PausedState != state);
    }
    this->context_menu_->exec(QCursor::pos());
}

void QVideoCanvas::keyPressEvent(QKeyEvent *event)/* override */
{
    if (event->key() == Qt::Key_Escape)
        event->ignore();
    else
        QWidget::keyPressEvent(event);
}

void QVideoCanvas::keyReleaseEvent(QKeyEvent *event)/* override */
{
    if (!this->player_.get())
    {
        QWidget::keyReleaseEvent(event);

        return;
    }

    switch (event->key())
    {
    case Qt::Key_Space:
        switch (this->player_->state())
        {
        case QMediaPlayer::PlayingState:
            this->player_->pause();
            break;

        case QMediaPlayer::PausedState:
            this->player_->play();
            break;

        default:
            break;
        }
        break;

    case Qt::Key_Escape:
        switch (this->player_->state())
        {
        case QMediaPlayer::PlayingState:
        case QMediaPlayer::PausedState:
            this->player_->stop();
            break;

        default:
            break;
        }
        break;

    default:
        QWidget::keyReleaseEvent(event);
        break;
    } // switch (event->key())
}

static inline void avoid_blank_screen(const std::shared_ptr<QMediaPlayer> &player)
{
    player->setPosition(player->position());
}

void QVideoCanvas::focusInEvent(QFocusEvent *event)/* override */
{
    avoid_blank_screen(this->player_);
}

void QVideoCanvas::focusOutEvent(QFocusEvent *event)/* override */
{
    avoid_blank_screen(this->player_);
}

void QVideoCanvas::play(void)
{
    if (this->player_.get())
        this->player_->play();
    else
        qtCErrV(::, "*** Player not set yet!\n");
}

void QVideoCanvas::pause(void)
{
    if (this->player_.get())
        this->player_->pause();
    else
        qtCErrV(::, "*** Player not set yet!\n");
}

void QVideoCanvas::stop(void)
{
    if (this->player_.get())
        this->player_->stop();
    else
        qtCErrV(::, "*** Player not set yet!\n");
}

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2025-03-27, Man Hung-Coeng <udc577@126.com>:
 *  01. Initial commit.
 */

