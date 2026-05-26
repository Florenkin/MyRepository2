#include "mainwidget.h"
#include <QApplication>

#include "simpleQtLogger.h"

void debugInfoInit()
{
    //使能qDebug 输出
    simpleqtlogger::ENABLE_LOG_SINK_QDEBUG              = true;
    //打开错误日志
    simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_ERROR    = true;
    //打开警告日志
    simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_WARNING  = true;
    //打开调试日志
    simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_DEBUG    = true;
    //打开信息输出日志
    simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_INFO     = true;
    simpleqtlogger::EnableLogLevels enableLogLevels_qDebug = simpleqtlogger::ENABLE_LOG_LEVELS;
    simpleqtlogger::SimpleQtLogger::createInstance(qApp);
    simpleqtlogger::SimpleQtLogger::getInstance()->setLogLevels_qDebug(enableLogLevels_qDebug);
    //设置日志格式
    simpleqtlogger::SimpleQtLogger::getInstance()->setLogFormat_qDebug("<TS> [<LL>] <TEXT>", "<TS> [<TID>] [<LL>] <TEXT>");

#if 0
    //写入文件
    simpleqtlogger::ENABLE_LOG_SINK_FILE = true;
    simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_ERROR    = true;
    simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_WARNING  = true;
    simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_DEBUG    = true;
    simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_INFO     = true;
    simpleqtlogger::EnableLogLevels enableLogLevels_file = simpleqtlogger::ENABLE_LOG_LEVELS;
    simpleqtlogger::SimpleQtLogger::createInstance(qApp);
    simpleqtlogger::SimpleQtLogger::getInstance()->setLogLevels_file(enableLogLevels_file);
    simpleqtlogger::SimpleQtLogger::getInstance()->setLogFormat_file("<TS> [<LL>] <TEXT>", "<TS> [<TID>] [<LL>] <TEXT>");
    simpleqtlogger::SimpleQtLogger::getInstance()->setLogFileName("testSimpleQtLogger.log", 10*1000, 10);
#endif
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    debugInfoInit();

    MainWidget w;
    w.show();

    return a.exec();
}
