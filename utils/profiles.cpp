﻿#include "profiles.h"
#include "hqutils.h"
#include <QDir>
#include <QDebug>

Profiles* Profiles::minstance = 0;
Profiles::MGarbage Profiles::Garbage;
Profiles::Profiles(QObject *parent) :
    configSettings(NULL),
    QObject(parent)
{
    QDir dir(ANDROID_FILE_PATH);
    if(!dir.exists())
    {
        dir.mkdir(ANDROID_FILE_PATH);
         qDebug()<<"path not exist!!!!!!!!!!!"<<ANDROID_FILE_PATH;
    } else
    {
        qDebug()<<"path already exist!!!!!!!!!!!";
    }
    configSettings = new QSettings(QString("%1/%2").arg(ANDROID_FILE_PATH).arg("profiles.ini"), QSettings::IniFormat);
    configSettings->setIniCodec(QTextCodec::codecForName("GB18030"));
}

Profiles::~Profiles()
{
    if(configSettings)
    {
        configSettings->deleteLater();
    }
//    if ( minstance )
//    {
//        delete minstance;
//        minstance = 0;
//    }
}
/*-------------------------------------------
 *
 * 实例化
 *
---------------------------------------------*/
Profiles *Profiles::instance()
{
    if ( minstance == 0)
    {
        minstance = new Profiles();
    }
    return minstance;
}
/*-------------------------------------------
 *
 * 设置默认值
 *
---------------------------------------------*/
void Profiles::setDefault(const QString & prefix,const QString &key, const QVariant &value)
{
    configSettings->beginGroup(prefix);
    if(configSettings->value(key).toString().isEmpty())
    {
        configSettings->setValue(key, value);
    }
    configSettings->endGroup();
}
/*-------------------------------------------
 *
 * 设置配置文件值
 *
---------------------------------------------*/
void Profiles::setValue(const QString & prefix,const QString & key, const QVariant & value)
{
//    qDebug()<<"prefix:"<<prefix<<" key:"<<key<<" value:"<<value;
    configSettings->beginGroup(prefix);
    {
        configSettings->setValue(key, value);
    }
    configSettings->endGroup();
}
/*-------------------------------------------
 *
 * 返回值
 *
---------------------------------------------*/
QVariant Profiles::value(const QString & prefix,const QString &keys, const QVariant &defaultValue)
{
    QVariant values;
    configSettings->beginGroup(prefix);
    {
        values =  configSettings->value( keys,defaultValue);
    }
    configSettings->endGroup();
    return values;
}

QStringList Profiles::getAllSections()
{
    return configSettings->childGroups();
}
