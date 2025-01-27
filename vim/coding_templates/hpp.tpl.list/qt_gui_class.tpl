/* SPDX-License-Identifier: Apache-2.0 */

/*
 * Main GUI class of this project.
 *
 * Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
 * All rights reserved.
 */

#ifndef __${HEADER_GUARD}_HPP__
#define __${HEADER_GUARD}_HPP__

#include "ui_${BASENAME}.h"

class ${BASENAME} : public QDialog, public Ui_Dialog
{
    Q_OBJECT

public:
    ${BASENAME}() = delete;
    ${BASENAME}(QWidget *parent = nullptr);
    Q_DISABLE_COPY_MOVE(${BASENAME});
    ~${BASENAME}();

public:
    void infoBox(const QString &title, const QString &text);
    void warningBox(const QString &title, const QString &text);
    void errorBox(const QString &title, const QString &text);

protected:
    void closeEvent(QCloseEvent *event) override;

Q_SIGNALS:
    void delegatingClose(void);
    void delegatingResize(int width, int height);

private Q_SLOTS:
    void __delegatingClose(void);
    void __delegatingResize(int width, int height);

private:
    bool m_delegating_close_happens;
};

#endif /* #ifndef __${HEADER_GUARD}_HPP__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
 *  01. Initial commit.
 */
