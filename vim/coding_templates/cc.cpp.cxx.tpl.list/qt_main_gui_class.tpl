// SPDX-License-Identifier: Apache-2.0

/*
 * Main GUI class of this project.
 *
 * Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
 * All rights reserved.
 */

#include "${BASENAME}.hpp"

#include <QtCore/QTextCodec>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QMessageBox>

#include "versions.h" // For FULL_VERSION()

const QTextCodec *G_TEXT_CODEC = QTextCodec::codecForName("UTF8"/*"GB2312"*/);

${BASENAME}::${BASENAME}(QWidget *parent/* = nullptr*/)
    : QDialog(parent)
    , is_delegating_close_(false)
{
    setupUi(this);

    QObject::connect(this, SIGNAL(delegatingClose(void)), this, SLOT(doDelegatingClose(void)));
    QObject::connect(this, SIGNAL(delegatingResize(int,int)), this, SLOT(doDelegatingResize(int,int)));

    //this->delegatingResize(this->geometry().width(), this->geometry().height());
    //this->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    this->setWindowTitle(QString::asprintf("%s [%s]", this->windowTitle().toStdString().c_str(), FULL_VERSION()));
    //this->setAttribute(Qt::WA_DeleteOnClose, true); // Useful for instance alloated on heap in some cases.
}

${BASENAME}::~${BASENAME}()
{
    // FIXME: Implement this function, and delete this comment line.
}

#define SHOW_MSG_BOX(_type, _title, _text)                      do { \
    bool visible = this->isVisible(); \
    QMessageBox::_type(visible ? this : this->parentWidget(), visible ? _title : this->windowTitle(), _text); \
} while (0)

void ${BASENAME}::infoBox(const QString &title, const QString &text)
{
    SHOW_MSG_BOX(information, title, text);
}

void ${BASENAME}::warningBox(const QString &title, const QString &text)
{
    SHOW_MSG_BOX(warning, title, text);
}

void ${BASENAME}::errorBox(const QString &title, const QString &text)
{
    SHOW_MSG_BOX(critical, title, text);
}

void ${BASENAME}::closeEvent(QCloseEvent *event)/* override */
{
    if (this->is_delegating_close_)
        event->accept();
    else
    {
        QMessageBox::StandardButton button = QMessageBox::question(
            this, "", "Exit now?", QMessageBox::Yes | QMessageBox::No);

        if (QMessageBox::Yes == button)
            event->accept();
        else
            event->ignore();
    }
}

void ${BASENAME}::doDelegatingClose()
{
    this->is_delegating_close_ = true;
    this->close();
}

void ${BASENAME}::doDelegatingResize(int width, int height)
{
    this->setFixedSize(width, height); // FIXME: Or do it your way.
}

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
 *  01. Initial commit.
 */
