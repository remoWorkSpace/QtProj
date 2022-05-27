#include "codestreamtranslator.h"
#include "ui_codestreamtranslator.h"

#include <QSettings>
#include <QDir>

CodeStreamTranslator::CodeStreamTranslator(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CodeStreamTranslator)
{
    ui->setupUi(this);

    initializeWidget();//展示控件，文本框清除内容
    loadConfigFile();//加载消息类型
                     //加载字段类型
                     //加载字段对应含义
}

CodeStreamTranslator::~CodeStreamTranslator()
{
    delete ui;
}

void CodeStreamTranslator::initializeWidget()
{
    //清除消息类型文本框显示的文本
    QString strName = "messTypeContent_label";
    QLabel* pLable = QWidget::findChild<QLabel*>(strName);
    pLable->clear();

    clearFieldMeaning();
    return;
}

void CodeStreamTranslator::clearFieldMeaning()
{
    QString strName;
    QLabel* pLable;
    QString strPos;
    for(int i=1; i<=32; i++)
    {
        strPos = QString::number(i, 10);
        strName = "byteContent_" + strPos + "_Label";
        pLable = QWidget::findChild<QLabel*>(strName);
        pLable->clear();//清除已有内容
    }
    return;
}

void CodeStreamTranslator::loadConfigFile()
{
    //从.ini文件读取数据加入map
    readConfigSection("messType", messTypeMap);
    readConfigSection("fieldType", fieldTypeMap);
    readConfigSection("fieldMeaning", fieldMeaningMap);
    return;
}

void CodeStreamTranslator::readConfigSection(QString strSection, QMap<QString,QString> &map)
{
    //QString strPath = QDir::currentPath()+"/messCoinfig.ini";//获取当前运行目录？
    QString strPath = QCoreApplication::applicationDirPath()+"/messCoinfig.ini";//获取app可执行文件所在位置
    QSettings settings(strPath, QSettings::IniFormat);

    /* //当文件不存在时，settings关联文件失败，无法获取到失败的错误码
    QSettings::Status enumStatus = settings.status();
    if(QSettings::NoError != enumStatus)
    {
        ui->messTypeContent_label->setText("打开messConfig.ini文件失败，或文件格式错误。");
        return;
    }*/

    settings.beginGroup(strSection);//定位到[strSection]组
    QStringList listKeys = settings.allKeys();//读取[strSection]组下的内容
    QString strKey;
    QString strValue;

    for(int i=0; i<listKeys.size(); i++)
    {
        strKey = listKeys.at(i);
        strValue = settings.value(strKey).toString();
        map.insert(strKey, strValue);
    }
    settings.endGroup();
    return;
}


bool CodeStreamTranslator::getMessType(int iMessHeadLen, QString strMess, OUT QString &strMessType, OUT QString &strMessTypeDiscrip)
{
    QString strMessHead = strMess.left(iMessHeadLen);//取消息头

    QMap<QString,QString>::Iterator iter = messTypeMap.find(strMessHead+"_type");
    if(iter == messTypeMap.end())
    {
        //消息头错误，没有此类型
        strMessType = "noType";
        strMessTypeDiscrip = "消息头不正确，无法解析";
        return false;
    }
    else
    {
        strMessType = messTypeMap.find(strMessHead+"_type").value();
        strMessTypeDiscrip = messTypeMap.find(strMessHead+"_meaning").value();
        return true;
    }
}

void CodeStreamTranslator::on_translateButton_clicked()
{
    //从消息框获取码流
    QString strCodeStream = ui->messbox_TextEdit->toPlainText().trimmed();

    //获取消息类型 及 含义描述
    //如果前4位是FF FF FF FF,为上位机发出的消息，共28位
    //如果前两位是EE EE，最后2位是AA AA，为上位机发出的消息，控制下位机关闭蜂鸣器，共5位
    //如果前两位是BB BB，最后2位是DD DD，为上位机发出的消息，控制下位机自检，共5位
    //如果前两位是FF FF，最后2位是AA AA，为下位机发出的消息，共32位
    QString strMessType;
    QString strMessTypeDiscrip;
    //取前4个字节
    bool bRet = getMessType(11, strCodeStream, strMessType, strMessTypeDiscrip);
    if(bRet!=true)
    {
        //取前2个字节
        getMessType(5, strCodeStream, strMessType, strMessTypeDiscrip);
    }

    //设置消息类型 Label 控件
    ui->messTypeContent_label->setText(strMessTypeDiscrip);
    ui->messTypeContent_label->setStyleSheet("QLabel{background-color:rgb(255,250,232);}");

    //解析码流
    parseMess(strMessType, strCodeStream);

    return;
}

void CodeStreamTranslator::parseMess(QString strMessType, QString strMess)
{
    //如果码流为空，直接退出
    if(0 == strMess.size())
    {
        return;
    }
    //如果码流最后添加一位空格，便于解析码流时提取单个字段
    QString strMessMod = strMess+" ";

    QString::iterator iter = strMessMod.begin();
    QString strFieldValue;//字段取值
    QString strFieldType;//字段类型
    QString strFieldMeaning;//字段含义
    int iFieldPos = 1;//字段在码流中位置
    QString strFieldPos;//字段在码流中的位置，string类型

    //清理控件中已有的文本，为下次解析做准备
    clearFieldMeaning();

    for(; iter != strMessMod.end(); iter++)
    {
        if(iter->isSpace())
        {
            //遇到空格，对 积累的字符和字节位置 进行处理
            //1、获取字段类型
            strFieldPos = QString::number(iFieldPos, 10);
            strFieldType = getFieldType(strMessType, strFieldPos);
            //2、获取字段含义
            strFieldMeaning = getFieldMeaning(strMessType, strFieldPos, strFieldType, strFieldValue);
            //3、找到对应位置控件，展示含义
            displayFieldMeaning(iFieldPos, strFieldMeaning);
            //积累字节和字节位置清理备用
            strFieldValue.clear();
            iFieldPos++;
        }
        else
        {
            strFieldValue.append(*iter);
        }
    }

    return;
}

QString CodeStreamTranslator::getFieldType(QString strMessType, QString strFieldPos)
{
    QString strKey = strMessType + "_" + strFieldPos;

    QMap<QString, QString>::Iterator iter = fieldTypeMap.find(strKey);
    if(fieldTypeMap.end() == iter)
    {
        QString strFieldType = "Can't find type";
        return strFieldType;
    }
    return iter.value();
}
//QString strCodeType, QString strValue
QString CodeStreamTranslator::getFieldMeaning(QString strMessType, QString strFieldPos, QString strFieldType, QString strFieldValue)
{
    //拼接map主键
    QString strKey = strMessType + "_" + strFieldPos;
    if(strFieldType == "Enum")
    {
        strKey.append("_");
        strKey.append(strFieldValue);
    }
    //获取字段取值对应进制数据，开始拼接字段含义
    QString strFieldMeaning  = strFieldValue;
    if(strFieldType == "Int")
    {
        //十六进制转为十进制数字
        bool bOk;
        QString strDecFieldValue = QString::number(strFieldValue.toInt(&bOk, 16), 10);
        strFieldMeaning = strDecFieldValue;
    }
    //查map
    QMap<QString, QString>::iterator iter = fieldMeaningMap.find(strKey);
    if(fieldMeaningMap.end() == iter)
    {
        strFieldMeaning.append(" |Can't find meaning");
    }
    else
    {
        strFieldMeaning.append(" ");
        strFieldMeaning.append(iter.value());
    }
    return strFieldMeaning;
}

void CodeStreamTranslator::displayFieldMeaning(int iFieldPos, QString strMessMeaning)
{
    QString strFieldPos = QString::number(iFieldPos, 10);
    QString strWidgetName = "byteContent_" + strFieldPos + "_Label";

    QLabel* pLabel = QWidget::findChild<QLabel*>(strWidgetName);
    pLabel->setText(strMessMeaning);
    //pLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);//.ui文件，控件里文本的默认布局
    pLabel->setStyleSheet("QLabel{background-color:rgb(255,250,232);}");//设置背景色协助查看label文本在label中的位置
    return;
}
//rgb(200,101,102)红棕色
//rgb(191,253,248)浅蓝色
//rgb(255,250,232)浅黄色















