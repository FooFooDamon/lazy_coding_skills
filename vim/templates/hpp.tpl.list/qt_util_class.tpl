/* SPDX-License-Identifier: Apache-2.0 */

/*
 * TODO: Brief description of this file.
 *
 * Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
 * All rights reserved.
 */

#ifndef __${HEADER_GUARD}_HPP__
#define __${HEADER_GUARD}_HPP__

#include <QObject> // FIXME: Delete this if including QXxYy below.
//#include <QXxYy> // FIXME: Delete this if none.

class ${BASENAME} : public QObject // or QXxYy
{
    Q_OBJECT

public:
    ${BASENAME}() = delete;
    ${BASENAME}(QObject/* or QXxYy */ *parent = nullptr);
    Q_DISABLE_COPY_MOVE(${BASENAME});
    ~${BASENAME}();

Q_SIGNALS:
    // FIXME: Declare signal functions if any, and delete this comment line.

public Q_SLOTS:
    // FIXME: Declare public slot functions if any, and delete this comment line.

private Q_SLOTS:
    // FIXME: Declare private slot functions if any, and delete this comment line.

private:
    // FIXME: Declare member variables if any, and delete this comment line.
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
