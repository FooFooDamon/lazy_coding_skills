/* SPDX-License-Identifier: Apache-2.0 */

/*
 * Formatted print enhancements based on Qt console debugging techniques.
 *
 * Copyright (c) 2024-2025 Man Hung-Coeng <udc577@126.com>
 * All rights reserved.
 */

#ifndef __QT_PRINT_HPP__
#define __QT_PRINT_HPP__

#include <typeinfo>

#include <QtGlobal>
#include <QThread>
#include <QLoggingCategory>

#if defined(__unix) || defined(__unix__) || defined(unix) \
    || defined(__linux) || defined(__linux__) || defined(linux) || defined(__gnu_linux__)

#ifndef QT_PRINT_WITH_COLOR
#define QT_PRINT_WITH_COLOR 1
#endif

#endif

#define QPRINT_FMT_ESCAPE_DEBUG(_format_)           _format_
#define QPRINT_FMT_ESCAPE_INFO(_format_)            _format_
#if defined(QT_PRINT_WITH_COLOR) && QT_PRINT_WITH_COLOR
#define QPRINT_FMT_ESCAPE_NOTICE(_format_)          "\033[0;32m[NOTICE] " _format_ "\033[0m"
#define QPRINT_FMT_ESCAPE_WARN(_format_)            "\033[0;33m" _format_ "\033[0m"
#define QPRINT_FMT_ESCAPE_ERR(_format_)             "\033[0;31m" _format_ "\033[0m"
#else
#define QPRINT_FMT_ESCAPE_NOTICE(_format_)          "[NOTICE] " _format_
#define QPRINT_FMT_ESCAPE_WARN(_format_)            _format_
#define QPRINT_FMT_ESCAPE_ERR(_format_)             _format_
#endif

#ifndef qtDebug
#define qtDebug(_format_, ...)                      qDebug(QPRINT_FMT_ESCAPE_DEBUG(_format_), ##__VA_ARGS__)
#endif

#ifndef qtInfo
#define qtInfo(_format_, ...)                       qInfo(QPRINT_FMT_ESCAPE_INFO(_format_), ##__VA_ARGS__)
#endif

#ifndef qtNotice
#define qtNotice(_format_, ...)                     qWarning(QPRINT_FMT_ESCAPE_NOTICE(_format_), ##__VA_ARGS__)
#endif

#ifndef qtWarn
#define qtWarn(_format_, ...)                       qWarning(QPRINT_FMT_ESCAPE_WARN(_format_), ##__VA_ARGS__)
#endif

#ifndef qtErr
#define qtErr(_format_, ...)                        qCritical(QPRINT_FMT_ESCAPE_ERR(_format_), ##__VA_ARGS__)
#endif

#define __QT_PRINT_VERBOSE(_level_, _qt_api_, _namespace_, _format_, ...)     \
    _qt_api_(QPRINT_FMT_ESCAPE_##_level_("(T:%s) " __FILE__ ":%d " #_namespace_ "%s(): " _format_), \
        QT_GET_THREAD_NAME(), __LINE__, __func__, ##__VA_ARGS__)

#ifndef qtDebugV
#define qtDebugV(_namespace_, _format_, ...)        __QT_PRINT_VERBOSE(DEBUG, qDebug, _namespace_, _format_, ##__VA_ARGS__)
#endif

#ifndef qtInfoV
#define qtInfoV(_namespace_, _format_, ...)         __QT_PRINT_VERBOSE(INFO, qInfo, _namespace_, _format_, ##__VA_ARGS__)
#endif

#ifndef qtNoticeV
#define qtNoticeV(_namespace_, _format_, ...)       __QT_PRINT_VERBOSE(NOTICE, qWarning, _namespace_, _format_, ##__VA_ARGS__)
#endif

#ifndef qtWarnV
#define qtWarnV(_namespace_, _format_, ...)         __QT_PRINT_VERBOSE(WARN, qWarning, _namespace_, _format_, ##__VA_ARGS__)
#endif

#ifndef qtErrV
#define qtErrV(_namespace_, _format_, ...)          __QT_PRINT_VERBOSE(ERR, qCritical, _namespace_, _format_, ##__VA_ARGS__)
#endif

#define __QT_CLASS_PRINT_VERBOSE(_level_, _qt_api_, _namespace_, _format_, ...)     \
    _qt_api_(QPRINT_FMT_ESCAPE_##_level_("(T:%s) " __FILE__ ":%d " #_namespace_ "%s::%s(): " _format_), \
        QT_GET_THREAD_NAME(), __LINE__, typeid(*this).name(), __func__, ##__VA_ARGS__)

#ifndef qtCDebugV
#define qtCDebugV(_namespace_, _format_, ...)       __QT_CLASS_PRINT_VERBOSE(DEBUG, qDebug, _namespace_, _format_, ##__VA_ARGS__)
#endif

#ifndef qtCInfoV
#define qtCInfoV(_namespace_, _format_, ...)        __QT_CLASS_PRINT_VERBOSE(INFO, qInfo, _namespace_, _format_, ##__VA_ARGS__)
#endif

#ifndef qtCNoticeV
#define qtCNoticeV(_namespace_, _format_, ...)      __QT_CLASS_PRINT_VERBOSE(NOTICE, qWarning, _namespace_, _format_, ##__VA_ARGS__)
#endif

#ifndef qtCWarnV
#define qtCWarnV(_namespace_, _format_, ...)        __QT_CLASS_PRINT_VERBOSE(WARN, qWarning, _namespace_, _format_, ##__VA_ARGS__)
#endif

#ifndef qtCErrV
#define qtCErrV(_namespace_, _format_, ...)         __QT_CLASS_PRINT_VERBOSE(ERR, qCritical, _namespace_, _format_, ##__VA_ARGS__)
#endif

#ifndef QT_GET_THREAD_NAME
#define QT_GET_THREAD_NAME()                        QThread::currentThread()->objectName().toStdString().c_str()
#endif

#ifndef QT_SET_THREAD_NAME
#define QT_SET_THREAD_NAME(_name_)                  do { \
    QThread *_thread_ = QThread::currentThread(); \
\
    _thread_->setObjectName(_name_); \
    pthread_setname_np(pthread_self(), _thread_->objectName().toStdString().c_str()); \
} while (0)
#endif

#ifndef QT_PRINT_INITIALIZE
#define QT_PRINT_INITIALIZE(_level_, _need_msg_preamble_)   do { \
    bool debug_msg_enabled = (0 == strcasecmp(_level_, "debug")); \
    bool info_msg_enabled = (debug_msg_enabled || 0 == strcasecmp(_level_, "info")); \
    bool notice_msg_enabled = (info_msg_enabled || 0 == strcasecmp(_level_, "notice")); \
    bool warn_msg_enabled = (notice_msg_enabled || 0 == strcasecmp(_level_, "warning")); \
    bool err_msg_enabled = (warn_msg_enabled || 0 == strcasecmp(_level_, "error")); \
\
    if (_need_msg_preamble_) { \
        qSetMessagePattern("%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}" \
            "%{if-critical}E%{endif}%{if-fatal}F%{endif} %{time yyyy-MM-dd hh:mm:ss.zzz} %{message}"); \
    } \
\
    QLoggingCategory::setFilterRules(QString::asprintf( \
        "default.debug=%s\ndefault.info=%s\n" \
        "default.warning=%s\ndefault.critical=%s\n", \
        (debug_msg_enabled ? "true" : "false"), (info_msg_enabled ? "true" : "false"), \
        (warn_msg_enabled ? "true" : "false"), (err_msg_enabled ? "true" : "false") \
    )); \
\
    /* To remove the auto-newline of each message. */ \
    /*qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg){ \
        FILE *stream = ((QtMsgType::QtDebugMsg == type) || (QtMsgType::QtInfoMsg == type)) ? stdout : stderr; \
\
        fprintf(stream, "%s", qFormatLogMessage(type, context, msg).toStdString().c_str()); \
    });*/ \
} while (0)
#endif

#ifndef QT_PRINT_FINALIZE
#define QT_PRINT_FINALIZE()                                 do {} while (0)
#endif

#endif /* #ifndef __QT_PRINT_HPP__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2024-09-18, Man Hung-Coeng <udc577@126.com>:
 *  01. Initial commit.
 *
 * >>> 2025-04-07, Man Hung-Coeng <udc577@126.com>:
 *  01. Enable auto-newline for each message again.
 */

