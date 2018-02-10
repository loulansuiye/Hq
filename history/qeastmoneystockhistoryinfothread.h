﻿#ifndef QEASTMONEYSTOCKHISTORYINFOTHREAD_H
#define QEASTMONEYSTOCKHISTORYINFOTHREAD_H

#include <QStringList>
#include <QDate>
#include <QRunnable>
#include <stockdata.h>
#include <QObject>

class QEastmoneyStockHistoryInfoThread : public QRunnable
{
public:
    explicit QEastmoneyStockHistoryInfoThread(const QString& code, const StockDataList& list, bool deldb, QObject* parent = 0,  const QDate& date = QDate() );
    ~QEastmoneyStockHistoryInfoThread();
    QString getCode();
public:
    void run();
private:
    QString         mCode;
    QDate           mStartDate;
    StockDataList   mForeignVolList;
    QObject         *mParent;
    bool            mDelDB;
};

#endif // QEASTMONEYSTOCKHISTORYINFOTHREAD_H


//http://nufm.dfcfw.com/EM_Finance2014NumericApplication/JS.aspx?type=CT&cmd=P.[(x)]|3000592&sty=MPICT&st=z&sr=&p=&ps=&cb=callback&js=&token=aaf32a88de888ea7b0ea63e017ecb049&_=1481245370248
