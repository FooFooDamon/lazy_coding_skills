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
{
    setupUi(this);

    //this->setFixedSize(this->geometry().size());
    //this->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    this->setWindowTitle(QString::asprintf("%s [%s]", this->windowTitle().toStdString().c_str(), FULL_VERSION()));
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
    QMessageBox::StandardButton button = QMessageBox::question(
        this, "", "Exit now ?", QMessageBox::Yes | QMessageBox::No);

    if (QMessageBox::Yes == button)
        event->accept();
    else
        event->ignore();
}

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
 *  01. Initial commit.
 */