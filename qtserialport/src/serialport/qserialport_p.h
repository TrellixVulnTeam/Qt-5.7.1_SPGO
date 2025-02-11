/****************************************************************************
**
** Copyright (C) 2011-2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2011 Sergey Belyashov <Sergey.Belyashov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSERIALPORT_P_H
#define QSERIALPORT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qserialport.h"

#include <private/qringbuffer_p.h>
#include <private/qiodevice_p.h>

#if defined(Q_OS_WIN32)
#  include <qt_windows.h>
#elif defined(Q_OS_UNIX)
#  include <QtCore/qlockfile.h>
#  include <QtCore/qscopedpointer.h>
#  include <QtCore/qfileinfo.h>
#  include <QtCore/qstringlist.h>
#  include <limits.h>
#  include <termios.h>
#  ifdef Q_OS_ANDROID
struct serial_struct {
    int     type;
    int     line;
    unsigned int    port;
    int     irq;
    int     flags;
    int     xmit_fifo_size;
    int     custom_divisor;
    int     baud_base;
    unsigned short  close_delay;
    char    io_type;
    char    reserved_char[1];
    int     hub6;
    unsigned short  closing_wait;
    unsigned short  closing_wait2;
    unsigned char   *iomem_base;
    unsigned short  iomem_reg_shift;
    unsigned int    port_high;
    unsigned long   iomap_base;
};
#    define ASYNC_SPD_CUST  0x0030
#    define ASYNC_SPD_MASK  0x1030
#    define PORT_UNKNOWN    0
#  elif defined(Q_OS_LINUX)
#    include <linux/serial.h>
#  endif
#else
#  error Unsupported OS
#endif

QT_BEGIN_NAMESPACE

class QThread;
class QWinOverlappedIoNotifier;
class QTimer;
class QSocketNotifier;

#if defined(Q_OS_UNIX)
QString serialPortLockFilePath(const QString &portName);
#endif

class QSerialPortErrorInfo
{
public:
    explicit QSerialPortErrorInfo(QSerialPort::SerialPortError newErrorCode = QSerialPort::UnknownError,
                                  const QString &newErrorString = QString());
    QSerialPort::SerialPortError errorCode;
    QString errorString;
};

class QSerialPortPrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QSerialPort)
public:
    enum IoConstants {
        ReadChunkSize = 512,
        InitialBufferSize = 16384
    };

    QSerialPortPrivate();

    bool open(QIODevice::OpenMode mode);
    void close();

    QSerialPort::PinoutSignals pinoutSignals();

    bool setDataTerminalReady(bool set);
    bool setRequestToSend(bool set);

    bool flush();
    bool clear(QSerialPort::Directions directions);

    bool sendBreak(int duration);
    bool setBreakEnabled(bool set);

    bool waitForReadyRead(int msec);
    bool waitForBytesWritten(int msec);

    bool setBaudRate();
    bool setBaudRate(qint32 baudRate, QSerialPort::Directions directions);
    bool setDataBits(QSerialPort::DataBits dataBits);
    bool setParity(QSerialPort::Parity parity);
    bool setStopBits(QSerialPort::StopBits stopBits);
    bool setFlowControl(QSerialPort::FlowControl flowControl);

    QSerialPortErrorInfo getSystemError(int systemErrorCode = -1) const;

    void setError(const QSerialPortErrorInfo &errorInfo);

    qint64 writeData(const char *data, qint64 maxSize);

    static QString portNameToSystemLocation(const QString &port);
    static QString portNameFromSystemLocation(const QString &location);

#if defined(Q_OS_UNIX)
    static qint32 settingFromBaudRate(qint32 baudRate);
#endif

    static QList<qint32> standardBaudRates();

    qint64 readBufferMaxSize;
    QRingBuffer writeBuffer;
    QSerialPort::SerialPortError error;
    QString systemLocation;
    qint32 inputBaudRate;
    qint32 outputBaudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
    QSerialPort::FlowControl flowControl;
    bool settingsRestoredOnClose;
    bool isBreakEnabled;

#if defined(Q_OS_WIN32)

    bool initialize();
    bool setDcb(DCB *dcb);
    bool getDcb(DCB *dcb);
    OVERLAPPED *waitForNotified(int msecs);

    qint64 queuedBytesCount(QSerialPort::Direction direction) const;

    bool completeAsyncCommunication(qint64 bytesTransferred);
    bool completeAsyncRead(qint64 bytesTransferred);
    bool completeAsyncWrite(qint64 bytesTransferred);

    bool startAsyncCommunication();
    bool startAsyncRead();
    bool _q_startAsyncWrite();
    void _q_notified(DWORD numberOfBytes, DWORD errorCode, OVERLAPPED *overlapped);

    void emitReadyRead();

    DCB restoredDcb;
    COMMTIMEOUTS currentCommTimeouts;
    COMMTIMEOUTS restoredCommTimeouts;
    HANDLE handle;
    QByteArray readChunkBuffer;
    QByteArray writeChunkBuffer;
    bool communicationStarted;
    bool writeStarted;
    bool readStarted;
    QWinOverlappedIoNotifier *notifier;
    QTimer *startAsyncWriteTimer;
    OVERLAPPED communicationOverlapped;
    OVERLAPPED readCompletionOverlapped;
    OVERLAPPED writeCompletionOverlapped;
    DWORD originalEventMask;
    DWORD triggeredEventMask;

#elif defined(Q_OS_UNIX)

    bool initialize(QIODevice::OpenMode mode);
    bool setTermios(const termios *tio);
    bool getTermios(termios *tio);

    bool setCustomBaudRate(qint32 baudRate, QSerialPort::Directions directions);
    bool setStandardBaudRate(qint32 baudRate, QSerialPort::Directions directions);

    bool isReadNotificationEnabled() const;
    void setReadNotificationEnabled(bool enable);
    bool isWriteNotificationEnabled() const;
    void setWriteNotificationEnabled(bool enable);

    bool waitForReadOrWrite(bool *selectForRead, bool *selectForWrite,
                            bool checkRead, bool checkWrite,
                            int msecs);

    qint64 readFromPort(char *data, qint64 maxSize);
    qint64 writeToPort(const char *data, qint64 maxSize);

#ifndef CMSPAR
    qint64 writePerChar(const char *data, qint64 maxSize);
#endif

    bool readNotification();
    bool startAsyncWrite();
    bool completeAsyncWrite();

    struct termios restoredTermios;
    int descriptor;

    QSocketNotifier *readNotifier;
    QSocketNotifier *writeNotifier;

    bool readPortNotifierCalled;
    bool readPortNotifierState;
    bool readPortNotifierStateSet;

    bool emittedReadyRead;
    bool emittedBytesWritten;

    qint64 pendingBytesWritten;
    bool writeSequenceStarted;

    QScopedPointer<QLockFile> lockFileScopedPointer;

#endif
};

QT_END_NAMESPACE

#endif // QSERIALPORT_P_H
