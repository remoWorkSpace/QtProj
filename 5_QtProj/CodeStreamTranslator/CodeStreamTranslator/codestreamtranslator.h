#ifndef CODESTREAMTRANSLATOR_H
#define CODESTREAMTRANSLATOR_H

#include <QWidget>
#include <QMap>

#define IN
#define OUT

QT_BEGIN_NAMESPACE
namespace Ui { class CodeStreamTranslator; }
QT_END_NAMESPACE

class CodeStreamTranslator : public QWidget
{
    Q_OBJECT

public:
    CodeStreamTranslator(QWidget *parent = nullptr);
    ~CodeStreamTranslator();

private slots:
    void on_translateButton_clicked();

private:
    Ui::CodeStreamTranslator *ui;

    QMap<QString, QString> messTypeMap;//消息类型map，key：消息码流前4个字节或者前2个字节
                                       //           value：消息类型
    QMap<QString, QString> fieldTypeMap;//字段类型map，key：消息类型_字段位置
                                       //            value：字段类型(枚举/数字)
                                       //字段取值多含义的类型为Enum，字段取值一个含义的为Int
                                       //决定了此字段在fieldMeaningMap中的主键拼写方式

    QMap<QString, QString> fieldMeaningMap;//字段含义map，key：消息类型_字段位置/消息类型_字段位置_字段取值
                                           //           value:字段含义

    void initializeWidget();//初始化：清除消息类型和字段含义文本框显示的内容
    void loadConfigFile();//加载消息类型map、字段类型map、字段含义map
    void readConfigSection(QString strSection, QMap<QString,QString> &map);//读取messConfig.ini文件中指定[节]的内容
    void clearFieldMeaning();//清除字段含义文本框显示的内容

    void parseMess(QString strMessType, QString strMess);//解析消息码流

    bool getMessType(int iMessHeadLen, QString strMess, OUT QString &strMessType, OUT QString &strMessTypeDiscrip);
    QString getFieldType(QString strMessType, QString strFieldPos);//从fieldTypeMap中，获取字段类型
    QString getFieldMeaning(QString strMessType, QString strFieldPos, QString strFieldType, QString strFieldValue);//从fieldMeaningMap中获取字段含义
    void displayFieldMeaning(int iFieldPos, QString strMessMeaning);//控件显示字段含义
};
#endif // CODESTREAMTRANSLATOR_H
