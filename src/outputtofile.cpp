#include "outputtofile.hpp"
#include <QStandardPaths>
#include <QGuiApplication>
#include <QTextCodec>
#include <QDesktopServices>
#include <QTimeZone>
#include <QDir>

OutputToFile::OutputToFile(QSettings *settings, const QString &settingsGroupPath, QObject *parent) : QObject(parent)
{
    _settings = settings;
    _settingsGroupPath = settingsGroupPath;

    reinitIni();

    if (_settings)
    {
        setEnabled(_settings->value(_settingsGroupPath + "/" + _settingsKeyEnabled, false).toBool());

        setOutputFolder(_settings->value(_settingsGroupPath + "/" + _settingsKeyOutputFolder,
                standardOutputFolder()).toString());
    }
}

OutputToFile::~OutputToFile()
{
    if (_enabled)
    {
        if (_iniCurrent)
        {
            _iniCurrent->setValue("software/started", false);
        }
    }
}

bool OutputToFile::enabled() const
{
    return _enabled;
}

void OutputToFile::setEnabled(bool enabled)
{
    if (_enabled != enabled)
    {
        _enabled = enabled;
        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + _settingsKeyEnabled, enabled);
        }

        //qDebug(QString("OutputToFile: %1").arg(_enabled ? "enabled" : "disabled").toUtf8());

        if (_enabled)
        {
            reinitIni();
        }

        emit enabledChanged();
    }
}

QString OutputToFile::standardOutputFolder() const
{
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + QGuiApplication::applicationName() + "/output";
}

QString OutputToFile::outputFolder() const
{
    return _outputFolder;
}

void OutputToFile::setOutputFolder(const QString &outputFolder)
{
    if (_outputFolder != outputFolder)
    {
        _outputFolder = outputFolder;

        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + _settingsKeyOutputFolder, outputFolder);
        }

        reinitIni();

        //qDebug(QString("OutputToFile: outputFolder: \"%1\"").arg(_outputFolder).toUtf8());

        emit outputFolderChanged();
    }
}

void OutputToFile::resetSettings()
{
    setOutputFolder(standardOutputFolder());
}

void OutputToFile::onMessagesReceived(const ChatMessage &message, const MessageAuthor& author)
{
    //if (message.wherefrom.trimmed().toLower() == "software")
    if (message.type() == ChatMessage::Type::Unknown ||
        message.type() == ChatMessage::Type::SoftwareNotification ||
        message.type() == ChatMessage::Type::TestMessage)
    {
        return;
    }

    if (_enabled)
    {
        if (_iniMessages)
        {
            if (!_iniMessages->contains("statistic/count"))
            {
                _iniMessages->setValue("statistic/count", 0);
            }

            const QString& group = QString("%1").arg(_iniMessagesCount);

            _iniMessages->setValue(group + "/author",
                                   author.name());

            _iniMessages->setValue(group + "/message",
                                   message.text());

            _iniMessages->setValue(group + "/author_channel_id",
                                   message.author().channelId());

            _iniMessages->setValue(group + "/time", QString("%1:%2:%3.%4")
                                   .arg(message.publishedAt().time().hour(),   2, 10, QChar('0'))
                                   .arg(message.publishedAt().time().minute(), 2, 10, QChar('0'))
                                   .arg(message.publishedAt().time().second(), 2, 10, QChar('0'))
                                   .arg(message.publishedAt().time().msec(),   3, 10, QChar('0')));

            _iniMessagesCount++;

            _iniMessages->setValue("statistic/count", _iniMessagesCount);
        }
    }
}

void OutputToFile::showInExplorer()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(_outputFolder));
}

void OutputToFile::reinitIni()
{
    //Messages
    _iniMessagesCount = 0;

    if (_iniMessages)
    {
        _iniMessages->sync();
        _iniMessages->deleteLater();
        _iniMessages = nullptr;
    }

    _broadcastFolder = "";

    if (!_youTubeInfo.broadcastId.isEmpty())
    {
        _broadcastFolder = _outputFolder + "/broadcasts/" + _youTubeInfo.broadcastId;

        if (_enabled)
        {
            QDir dir = QDir(_broadcastFolder);
            if (!dir.exists())
            {
                dir.mkpath(_broadcastFolder);
            }
        }

        _iniMessages = new QSettings(_broadcastFolder + "/messages.ini", QSettings::IniFormat, this);
        _iniMessages->setIniCodec("UTF-8");

        _iniMessagesCount = _iniMessages->value("statistic/count", 0).toInt();
    }

    //Current
    if (_iniCurrent)
    {
        _iniCurrent->sync();
        _iniCurrent->deleteLater();
        _iniCurrent = nullptr;
    }

    _iniCurrent = new QSettings(_outputFolder + "/current.ini", QSettings::IniFormat, this);
    _iniCurrent->setIniCodec("UTF-8");

    if (_enabled)
    {
        _iniCurrent->setValue("software/started", true);
    }

    writeStartupInfo();
    writeYoutubeInfo();
}

void OutputToFile::writeStartupInfo()
{
    if (_enabled)
    {
        _iniCurrent->setValue("software/version",                  QCoreApplication::applicationVersion());

        _iniCurrent->setValue("software/startup_time_hour",        _startupDateTime.time().hour());
        _iniCurrent->setValue("software/startup_time_minute",      _startupDateTime.time().minute());
        _iniCurrent->setValue("software/startup_time_second",      _startupDateTime.time().second());
        _iniCurrent->setValue("software/startup_time_msec", _startupDateTime.time().msec());
        _iniCurrent->setValue("software/startup_timestamp_utc",    _startupDateTime.toMSecsSinceEpoch());

        _iniCurrent->setValue("software/startup_timezone_id",      QString::fromUtf8(_startupDateTime.timeZone().id()));
        _iniCurrent->setValue("software/startup_timezone_offset_from_utc", QString("%1")
                              .arg(double(_startupDateTime.timeZone().standardTimeOffset(_startupDateTime)) / float(60 * 60)));
    }
}

void OutputToFile::writeYoutubeInfo()
{
    if (_enabled)
    {
        _iniCurrent->setValue("youtube/broadcast_connected", _youTubeInfo.broadcastConnected);
        _iniCurrent->setValue("youtube/broadcast_id", _youTubeInfo.broadcastId);
        _iniCurrent->setValue("youtube/broadcast_user_specified", _youTubeInfo.userSpecified);
        _iniCurrent->setValue("youtube/broadcast_url", _youTubeInfo.broadcastLongUrl.toString());
        _iniCurrent->setValue("youtube/broadcast_chat_url", _youTubeInfo.broadcastChatUrl.toString());
    }
}

void OutputToFile::setYouTubeInfo(const YouTubeInfo &youTubeCurrent)
{
    _youTubeInfo = youTubeCurrent;

    reinitIni();
}
