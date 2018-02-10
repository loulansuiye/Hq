﻿#ifndef HQUTILS_H
#define HQUTILS_H

#include <QDate>
#include <QDebug>
#include <QDateTime>
enum SHARE_TYPE{
    SHARE_UNDEFINE = -1,
    SHARE_SH = 0,
    SHARE_SZ,
    SHARE_US,
    SHARE_HK,
    SHARE_FOUND,
};

class HqUtils
{

public:


    HqUtils();
    static bool isWeekend(const QDate& date);
    static SHARE_TYPE shareType(const QString& code);
    static QString prefixCode(const QString& code);
    static bool isCurrentActive();
    static bool isActiveTime(const QTime& time);
};

#endif // HQUTILS_H
