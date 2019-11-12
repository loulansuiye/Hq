﻿#include "qsinastkinfothread.h"
#include <QDebug>
#include <QDateTime>
#include "qexchangedatamanage.h"
#include "dbservices/dbservices.h"
#include "data_structure/hqutils.h"
#include "data_structure/sharedata.h"


QSinaStkInfoThread::QSinaStkInfoThread(bool send, QObject *parent) : QObject(parent)
  , mHttp(0)
  , mSendResFlag(send)
{
    mStkList.clear();
    connect(this, SIGNAL(signalSetStkList(QStringList)), this, SLOT(setStkList(QStringList)));
//    mUpdateTimer = new QTimer;
//    mUpdateTimer->setInterval(2);
//    connect(mUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateInfo()));
    this->moveToThread(&mWorkThread);
    mWorkThread.start();
    //mActive = true;
}

QSinaStkInfoThread::~QSinaStkInfoThread()
{
//    if(mUpdateTimer)
//    {
//        mUpdateTimer->stop();
//        mUpdateTimer->deleteLater();
//    }
    if(mHttp)
    {
        mHttp->deleteLater();
    }

    mWorkThread.quit();
    mWorkThread.wait();

}


void QSinaStkInfoThread::setStkList(const QStringList &list)
{
    foreach (QString code, list) {
        if(code.length() == 6)
        {
            mStkList.append(ShareData::prefixCode(code)+code);
        } else if(code.length() == 8)
        {
            mStkList.append(code);
        } else
        {
            mStkList.append(code);
        }
    }
    //开始更新
    QString url("http://hq.sinajs.cn/?list=%1");
    if(mStkList.length() > 0)
    {
        QString wkURL = url.arg(mStkList.join(","));
        if(!mHttp)
        {
            mHttp = new QHttpGet(wkURL, true);
            connect(mHttp, SIGNAL(signalSendHttpConent(QByteArray)), this, SLOT(slotRecvHttpContent(QByteArray)));
            connect(mHttp, SIGNAL(signalErrorOccured(QNetworkReply::NetworkError)), this, SLOT(slotRecvError(QNetworkReply::NetworkError)));
            mHttp->start();
        } else
        {
            mHttp->setUrl(wkURL);
        }
    }
}


void QSinaStkInfoThread::slotUpdateInfo()
{
    QDateTime now = QDateTime::currentDateTime();
    if(now.date().dayOfWeek() == 6 || now.date().dayOfWeek() == 7 ||
            now.time().hour() < 9 || now.time().hour() >= 15)
    {
       return;
    }

    if(mHttp) mHttp->start();
}

void QSinaStkInfoThread::slotRecvHttpContent(const QByteArray &bytes)
{
    QTextCodec *codes = QTextCodec::codecForName("GBK");
    QString result = codes->toUnicode(bytes);
    //先换行
    QStringList resultlist = result.split(QRegExp("[\\n;]"), QString::SkipEmptyParts);
    //再分割具体的字段
    QStringList top10Keys = DATA_SERVICE->getHshtTop10List();
    ShareDataList datalist;
    foreach (QString detail, resultlist)
    {
        //var hq_str_sz399006="创业板指,1647.848,1692.416,1680.387,1718.384,1635.450,0.000,0.000,10414641478,126326323478.810
        detail.replace("var hq_str_", "");
        //qDebug()<<detail;
        QStringList detailList = detail.split(QRegExp("[\",=]"), QString::SkipEmptyParts);
        if(detailList.length() < 11) continue;
        QString code = detailList[0];
        ShareData * data = DATA_SERVICE->getShareData(code);
        if(!data) continue;
        //qDebug()<<data->mCode<<data->mName<<data->mShareType;
        data->mName = detailList[1];
        data->mCur = detailList[4].toDouble();
        data->mLastClose = detailList[3].toDouble();
        data->mChg = detailList[4].toDouble() - data->mLastClose;
        data->mChgPercent = data->mChg * 100 / detailList[3].toDouble() ;
        double high = detailList[5].toDouble();
        double low = detailList[6].toDouble();
        double buy = detailList[7].toDouble();
        double sell = detailList[8].toDouble();
        double buy1 = detailList[12].toDouble();
        double sell1 = detailList[22].toDouble();

        //竞价时段的特殊处理
        if(data->mCur == 0)
        {
            double temp = fmax(buy, buy1);
            if(temp == 0) temp = data->mLastClose;
            data->mCur = temp;
            data->mChg = detailList[8].toDouble() - data->mLastClose;
            data->mChgPercent = data->mChg * 100 / detailList[3].toDouble() ;
        }
        data->mVol = detailList[9].toInt();
        data->mMoney = detailList[10].toDouble();
        data->mHsl = 0.0;
        data->mMoneyRatio = 0.0;
        if(data->mHistory.mLastMoney> 0){
            data->mMoneyRatio = data->mMoney / data->mHistory.mLastMoney;
        }

        if(data->mCur != 0)
        {
            data->mGXL = data->mBonusData.mXJFH / data->mCur;
        }
        data->mTotalCap = data->mCur * data->mFinanceData.mTotalShare;
        data->mMutalbleCap = data->mCur * data->mFinanceData.mMutalShare;
        if(data->mFinanceData.mMutalShare > 0)
        {
            data->mHsl = data->mVol / (double)(data->mFinanceData.mMutalShare);
        }
        if(data->mProfit == 0)
        {
            data->mProfit = DATA_SERVICE->getProfit(code);
        }
        data->mForeignCap = data->mHsgtData.mVolTotal * data->mCur ;
        data->mHsgtData.mVolChange = data->mHsgtData.mVolCh1;
        data->mForeignCapChg = data->mHsgtData.mVolChange * data->mCur ;
        if(data->mHistory.mWeekDayPrice > 0)
        {
            data->mHistory.mChgPersFromWeek = (data->mCur - data->mHistory.mWeekDayPrice) * 100.0 / data->mHistory.mWeekDayPrice;
        } else
        {
//            data->mHistory.mChgPersFromWeek = data->mChgPercent;
        }
        if(data->mHistory.mMonthDayPrice> 0)
        {
            data->mHistory.mChgPersFromMonth= (data->mCur - data->mHistory.mMonthDayPrice) * 100.0 / data->mHistory.mMonthDayPrice;
        } else
        {
//            data->mHistory.mChgPersFromMonth= data->mChgPercent;
        }

        if(data->mHistory.mYearDayPrice > 0)
        {
            data->mHistory.mChgPersFromYear = (data->mCur - data->mHistory.mYearDayPrice) * 100.0 / data->mHistory.mYearDayPrice;
        } else
        {
//            data->mHistory.mChgPersFromYear = data->mChgPercent;
        }
//        data->mUpdateTime = QDateTime::currentMSecsSinceEpoch();
        if(top10Keys.contains(data->mCode.right(6)))
        {
            data->mHsgtData.mIsTop10 = true;
        } else
        {
            data->mHsgtData.mIsTop10 = false;
        }
        if(mSendResFlag) datalist.append(*data);
    }
    if(mSendResFlag)  emit sendStkDataList(datalist);
}

int QSinaStkInfoThread::getStkCount()
{
    return mStkList.count();
}

void QSinaStkInfoThread::slotRecvError(QNetworkReply::NetworkError e)
{
    qDebug()<<"error:"<<e;
}


