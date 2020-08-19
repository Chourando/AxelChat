#ifndef YOUTUBEINTERCEPTOR_HPP
#define YOUTUBEINTERCEPTOR_HPP

#include <QObject>
#include <QWebEngineUrlRequestInterceptor>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "types.hpp"
#include <QWebEnginePage>
#include <QSettings>
#include "outputtofile.hpp"

class YouTubeInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
    Q_PROPERTY(QString userSpecifiedLink          READ userSpecifiedLink WRITE setLink NOTIFY linkChanged)
    Q_PROPERTY(QString broadcastId                READ broadcastId                     NOTIFY linkChanged)
    Q_PROPERTY(QUrl    broadcastShortUrl          READ broadcastShortUrl               NOTIFY linkChanged)
    Q_PROPERTY(QUrl    broadcastLongUrl           READ broadcastLongUrl                NOTIFY linkChanged)
    Q_PROPERTY(QUrl    chatUrl                    READ chatUrl                         NOTIFY linkChanged)
    Q_PROPERTY(QUrl    controlPanelUrl            READ controlPanelUrl                 NOTIFY linkChanged)
    Q_PROPERTY(bool    connected                  READ isConnected                     NOTIFY connectedChanged)
    Q_PROPERTY(bool    isBroadcastIdUserSpecified READ isBroadcastIdUserSpecified)

public:
    explicit YouTubeInterceptor(OutputToFile* outputToFile, QSettings* settings, const QString& settingsGroupPath = "youtube_interceptor", QObject *parent = nullptr);
    ~YouTubeInterceptor();
    void interceptRequest(QWebEngineUrlRequestInfo &info);
    QByteArray replyData() const;

    int messagesReceived() const;
    bool isConnected() const;
    bool isBroadcastIdUserSpecified() const;

    QString broadcastId() const;
    QString userSpecifiedLink() const;
    QUrl broadcastShortUrl() const;
    QUrl broadcastLongUrl() const;
    QUrl chatUrl() const;
    QUrl controlPanelUrl() const;

signals:
    void stateChanged();
    void connectedChanged();
    void readyRead(const QList<ChatMessage>& messages, const QList<MessageAuthor>& authors);
    void linkChanged();
    void connected(QString broadcastId);
    void disconnected(QString broadcastId);

public slots:
    void setLink(QString link);

private slots:
    void replyFinished(QNetworkReply* reply);

private:
    QString extractBroadcastId(const QString& link) const;

    OutputToFile* _outputToFile = nullptr;

    QSettings* _settings = nullptr;
    QString _settingsGroupPath;

    QNetworkAccessManager *_manager = new QNetworkAccessManager(this);
    QByteArray _replyData;

    int _messagesReceived = 0;

    YouTubeInfo _youtubeInfo;

    QWebEnginePage* _webPage = new QWebEnginePage(this);

    const QString _settingsKeyUserSpecifiedLink = "user_specified_link";
};

#endif // YOUTUBEINTERCEPTOR_HPP
