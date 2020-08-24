#include "youtubeinterceptor.hpp"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QWebEngineProfile>

YouTubeInterceptor::YouTubeInterceptor(OutputToFile* outputToFile, QSettings* settings, const QString& settingsGroupPath, QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent), _outputToFile(outputToFile), _settings(settings), _settingsGroupPath(settingsGroupPath)
{
    connect(_manager, &QNetworkAccessManager::finished, this, &YouTubeInterceptor::replyFinished);

    _webPage.setUrlRequestInterceptor(this);

    if (_settings)
    {
        setLink(_settings->value(_settingsGroupPath + "/" + _settingsKeyUserSpecifiedLink).toString());
    }
}

YouTubeInterceptor::~YouTubeInterceptor()
{
    _youtubeInfo.broadcastConnected = false;
    emit disconnected(_youtubeInfo.broadcastId);
    emit connectedChanged();

    if (_outputToFile)
    {
        _outputToFile->setYouTubeInfo(_youtubeInfo);
    }
}

void YouTubeInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    QUrl requestUrl = info.requestUrl();

    //qDebug() << "request:" << requestUrl;

    QString host = requestUrl.host().toLower();
    if (host.startsWith("www.", Qt::CaseInsensitive))
    {
        host = host.mid(4);
    }
    if (host.startsWith("studio.", Qt::CaseInsensitive))
    {
        host = host.mid(7);
    }

    QString path = requestUrl.path().toLower();
    if (path.startsWith('/'))
    {
        path = path.mid(1);
    }
    if (path.endsWith('/'))
    {
        path = path.left(path.lastIndexOf('/'));
    }

    //qDebug() << "host:" << host;
    //qDebug() << "path:" << path;

    //Chat update
    if (host.contains(QRegExp("^youtube\\..+$", Qt::CaseSensitivity::CaseInsensitive)) && (
                path.toLower() == "live_chat/get_live_chat" ||
                path.toLower() == "youtubei/v1/live_chat/get_live_chat"
                ))
    {
        if (!_youtubeInfo.broadcastConnected && !_youtubeInfo.broadcastId.isEmpty())
        {
            qDebug(QString("YouTube connected: %1")
                   .arg(_youtubeInfo.broadcastId).toUtf8());
            _youtubeInfo.broadcastConnected = true;
            if (_outputToFile)
            {
                _outputToFile->setYouTubeInfo(_youtubeInfo);
            }

            emit connected(_youtubeInfo.broadcastId);
            emit connectedChanged();
        }

        QString newUrl = requestUrl.toString().replace("youtubei/v1/live_chat/get_live_chat", "live_chat/get_live_chat", Qt::CaseSensitivity::CaseInsensitive);
        QNetworkRequest request = QNetworkRequest(newUrl);
        request.setRawHeader("User-Agent", QWebEngineProfile::defaultProfile()->httpUserAgent().toUtf8());
        _manager->get(request);
    }
}

void YouTubeInterceptor::replyFinished(QNetworkReply *reply)
{
    if (reply)
    {
        _replyData = reply->readAll();

        /*qDebug(QString("=================================================\n"
                       "%1\n"
                       "=================================================")
               .arg(QString::fromUtf8(_replyData))
               .toUtf8());*/

        reply->deleteLater();

        QList<ChatMessage> messages;
        QList<MessageAuthor> authors;


        QJsonDocument jsonDocument(QJsonDocument::fromJson(_replyData));
        QJsonObject responseJson = jsonDocument.object().value("response").toObject();

        QJsonObject liveChatContinuation = responseJson.value("continuationContents").toObject().value("liveChatContinuation").toObject();

        QJsonArray actionsJson = liveChatContinuation.value("actions").toArray();

        foreach (const QJsonValue& actionJson, actionsJson)
        {
            bool valid = false;

            QString messageText;
            QString messageId;
            const QDateTime& receivedAt = QDateTime::currentDateTime();
            QDateTime publishedAt;

            QString authorName;
            QString authorChannelId;
            QUrl authorBadgeUrl;
            QUrl authorAvatarUrl;
            bool authorIsVerified      = false;
            bool authorIsChatOwner     = false;
            bool authorIsChatSponsor   = false;
            bool authorIsChatModerator = false;

            const QJsonObject& actionObject = actionJson.toObject();

            if (actionObject.contains("addChatItemAction"))
            {
                //Message

                const QJsonObject& addChatItemAction = actionObject.value("addChatItemAction").toObject();

                const QJsonObject& liveChatTextMessageRenderer = addChatItemAction
                        .value("item").toObject()
                        .value("liveChatTextMessageRenderer").toObject();

                if (!liveChatTextMessageRenderer.isEmpty())
                {
                    messageId = liveChatTextMessageRenderer
                            .value("id").toString();

                    publishedAt = QDateTime::fromMSecsSinceEpoch(
                                liveChatTextMessageRenderer.value("timestampUsec").toString().toLongLong() / 1000,
                                Qt::TimeSpec::UTC).toLocalTime();

                    authorName = liveChatTextMessageRenderer
                            .value("authorName").toObject()
                            .value("simpleText").toString();

                    authorChannelId = liveChatTextMessageRenderer
                            .value("authorExternalChannelId").toString();

                    const QJsonArray& thumbnails = liveChatTextMessageRenderer
                            .value("authorPhoto").toObject()
                            .value("thumbnails").toArray();

                    int preWidth = -1;

                    foreach (const QJsonValue& element, thumbnails)
                    {
                        const QJsonObject thumbnail = element.toObject();

                        int width = thumbnail.value("width").toInt();

                        if (width > preWidth && authorAvatarUrl.isEmpty())
                        {
                            //Getting only max size thumbnail
                            authorAvatarUrl  = thumbnail.value("url").toString();

                            preWidth = width;
                        }
                    }

                    if (liveChatTextMessageRenderer.contains("authorBadges"))
                    {
                        const QJsonArray& authorBadges = liveChatTextMessageRenderer.value("authorBadges").toArray();

                        foreach (const QJsonValue& badge, authorBadges)
                        {
                            const QJsonObject& liveChatAuthorBadgeRenderer = badge.toObject().value("liveChatAuthorBadgeRenderer").toObject();

                            if (liveChatAuthorBadgeRenderer.contains("icon"))
                            {
                                const QString& iconType = liveChatAuthorBadgeRenderer.value("icon").toObject()
                                        .value("iconType").toString();

                                bool foundIconType = false;

                                if (iconType.toLower() == "owner")
                                {
                                    authorIsChatOwner = true;
                                    foundIconType = true;
                                }

                                if (iconType.toLower() == "moderator")
                                {
                                    authorIsChatModerator = true;
                                    foundIconType = true;
                                }

                                if (iconType.toLower() == "verified")
                                {
                                    authorIsVerified = true;
                                    foundIconType = true;
                                }

                                if (!foundIconType && !iconType.isEmpty())
                                {
                                    qDebug() << "Unknown iconType" << iconType;
                                }
                            }
                            else if (liveChatAuthorBadgeRenderer.contains("customThumbnail"))
                            {
                                authorIsChatSponsor = true;//ToDo: is not a fact

                                const QJsonArray& thumbnails = liveChatAuthorBadgeRenderer.value("customThumbnail").toObject()
                                        .value("thumbnails").toArray();

                                foreach (const QJsonValue& thumbnailJson, thumbnails)
                                {
                                    if (authorBadgeUrl.isEmpty())
                                    {
                                        authorBadgeUrl = QUrl(thumbnailJson.toObject().value("url").toString());
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                qDebug(QString("YouTubeInterceptor: Unknown json structure of object \"%1\". Data:\n"
                                               "==============================================================="
                                               "%1"
                                               "===============================================================")
                                       .arg("liveChatAuthorBadgeRenderer")
                                       .arg(QString::fromUtf8(_replyData))
                                       .toUtf8());
                            }
                        }
                    }

                    const QJsonArray& runs = liveChatTextMessageRenderer
                            .value("message").toObject()
                            .value("runs").toArray();
                    foreach (const QJsonValue& run, runs)
                    {

                        messageText += run.toObject().value("text").toString();
                    }

                    valid = true;
                }
            }
            else if (actionObject.contains("markChatItemAsDeletedAction"))
            {
                //Deleted message by unknown

                const QJsonObject& markChatItemAsDeletedAction = actionObject.value("markChatItemAsDeletedAction").toObject();
                const QJsonObject& deletedStateMessage = markChatItemAsDeletedAction.value("deletedStateMessage").toObject();

                const QJsonArray& runs = deletedStateMessage
                        .value("runs").toArray();
                foreach (const QJsonValue& run, runs)
                {
                    messageText += run.toObject().value("text").toString();
                }

                valid = true;
            }
            else if (actionObject.contains("markChatItemsByAuthorAsDeletedAction"))
            {
                //Deleted message by author

                const QJsonObject& markChatItemsByAuthorAsDeletedAction = actionObject.value("markChatItemsByAuthorAsDeletedAction").toObject();
                const QJsonObject& deletedStateMessage = markChatItemsByAuthorAsDeletedAction.value("deletedStateMessage").toObject();

                const QJsonArray& runs = deletedStateMessage
                        .value("runs").toArray();
                foreach (const QJsonValue& run, runs)
                {
                    messageText += run.toObject().value("text").toString();
                }

                valid = true;
            }
            else if (actionObject.contains("replaceChatItemAction"))
            {
                qDebug("YouTubeInterceptor: object \"replaceChatItemAction\" not support");
                qDebug(QString("Data:\n"
                               "==============================================================="
                               "%1"
                               "===============================================================")
                       .arg(QString::fromUtf8(_replyData))
                       .toUtf8());
            }
            else if (actionObject.contains("addLiveChatTickerItemAction"))
            {
                qDebug("YouTubeInterceptor: object \"addLiveChatTickerItemAction\" not support");
                qDebug(QString("Data:\n"
                               "==============================================================="
                               "%1"
                               "===============================================================")
                       .arg(QString::fromUtf8(_replyData))
                       .toUtf8());
            }
            else
            {
                qDebug(QString("YouTubeInterceptor: Unknown json structure of array \"%1\". Data:\n"
                               "==============================================================="
                               "%2"
                               "===============================================================")
                       .arg("actions")
                       .arg(QString::fromUtf8(_replyData))
                       .toUtf8());
            }

            if (valid)
            {
                const MessageAuthor& author = MessageAuthor::createFromYouTube(
                            authorName,
                            authorChannelId,
                            authorAvatarUrl,
                            authorBadgeUrl,
                            authorIsVerified,
                            authorIsChatOwner,
                            authorIsChatSponsor,
                            authorIsChatModerator);

                const ChatMessage& message = ChatMessage::createYouTube(
                            messageText,
                            messageId,
                            publishedAt,
                            receivedAt,
                            author);

                messages.append(message);
                authors.append(author);
            }
        }

        _messagesReceived += messages.count();

        emit readyRead(messages, authors);
    }
    else
    {
        qDebug() << "reply == nullptr";
    }

    emit stateChanged();
}

QString YouTubeInterceptor::extractBroadcastId(const QString &link) const
{
    QString withoutHttpsWWW = link;

    // \ ->/
    withoutHttpsWWW = withoutHttpsWWW.replace('\\', '/');

    //https:// http://
    if (withoutHttpsWWW.startsWith("https://"))
    {
        withoutHttpsWWW = withoutHttpsWWW.mid(8);
    }
    else if (withoutHttpsWWW.startsWith("http://"))
    {
        withoutHttpsWWW = withoutHttpsWWW.mid(7);
    }

    //www.
    if (withoutHttpsWWW.startsWith("www."))
    {
        withoutHttpsWWW = withoutHttpsWWW.mid(4);
    }

    // remove last '/'
    if (withoutHttpsWWW.endsWith("/"))
    {
        withoutHttpsWWW = withoutHttpsWWW.left(withoutHttpsWWW.lastIndexOf('/'));
    }

    //?
    QString withoutQuery = withoutHttpsWWW;
    if (withoutHttpsWWW.contains('?'))
    {
        withoutQuery = withoutHttpsWWW.left(withoutHttpsWWW.indexOf('?'));
    }

    const QUrlQuery& urlQuery = QUrlQuery(QUrl(link).query());

    const QString& vParameter = urlQuery.queryItemValue("v");

    QString broadcastId;
    QRegExp rx;

    //youtu.be/rSjMyeISW7w
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^youtu.be/([^/]*)$", Qt::CaseInsensitive);
        if (rx.indexIn(withoutQuery) != -1)
        {
            broadcastId = rx.cap(1);
        }
    }

    //studio.youtube.com/video/rSjMyeISW7w/livestreaming
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^(studio\\.)?youtube.com/video/([^/]*)/livestreaming$", Qt::CaseInsensitive);
        if (rx.indexIn(withoutQuery) != -1)
        {
            broadcastId = rx.cap(2);
        }
    }

    //youtube.com/video/rSjMyeISW7w
    //studio.youtube.com/video/rSjMyeISW7w
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^(studio\\.)?youtube.com/video/([^/]*)$", Qt::CaseInsensitive);
        if (rx.indexIn(withoutQuery) != -1)
        {
            broadcastId = rx.cap(2);
        }
    }

    //youtube.com/watch/rSjMyeISW7w
    //studio.youtube.com/watch/rSjMyeISW7w
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^(studio\\.)?youtube.com/watch/([^/]*)$", Qt::CaseInsensitive);
        if (rx.indexIn(withoutQuery) != -1)
        {
            broadcastId = rx.cap(2);
        }
    }

    //youtube.com/live_chat?is_popout=1&v=rSjMyeISW7w
    //studio.youtube.com/live_chat?v=rSjMyeISW7w&is_popout=1
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^(studio\\.)?youtube.com/live_chat$", Qt::CaseInsensitive);
        if (rx.indexIn(withoutQuery) != -1 && !vParameter.isEmpty())
        {
            broadcastId = vParameter;
        }
    }

    //https://www.youtube.com/watch?v=rSjMyeISW7w&feature=youtu.be
    //https://www.youtube.com/watch?v=rSjMyeISW7w
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^(studio\\.)?youtube.com/watch$", Qt::CaseInsensitive);
        if (rx.indexIn(withoutQuery) != -1 && !vParameter.isEmpty())
        {
            broadcastId = vParameter;
        }
    }

    //Broadcast id only
    //rSjMyeISW7w
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^[a-zA-Z0-9_\\-]+$", Qt::CaseInsensitive);
        if (rx.indexIn(withoutHttpsWWW) != -1)
        {
            broadcastId = withoutHttpsWWW;
        }
    }

    return broadcastId;
}

QUrl YouTubeInterceptor::chatUrl() const
{
    return _youtubeInfo.broadcastChatUrl;
}

QUrl YouTubeInterceptor::controlPanelUrl() const
{
    return _youtubeInfo.controlPanelUrl;
}

QUrl YouTubeInterceptor::broadcastLongUrl() const
{
    return _youtubeInfo.broadcastLongUrl;
}

QString YouTubeInterceptor::userSpecifiedLink() const
{
    return _youtubeInfo.userSpecified;
}

QUrl YouTubeInterceptor::broadcastShortUrl() const
{
    return _youtubeInfo.broadcastShortUrl;
}

QString YouTubeInterceptor::broadcastId() const
{
    return _youtubeInfo.broadcastId;
}

bool YouTubeInterceptor::isConnected() const
{
    return _youtubeInfo.broadcastConnected;
}

bool YouTubeInterceptor::isBroadcastIdUserSpecified() const
{
    return _youtubeInfo.userSpecified.trimmed() == _youtubeInfo.broadcastId.trimmed() && !_youtubeInfo.userSpecified.isEmpty();
}

int YouTubeInterceptor::messagesReceived() const
{
    return _messagesReceived;
}

QByteArray YouTubeInterceptor::replyData() const
{
    return _replyData;
}

void YouTubeInterceptor::setLink(QString link)
{
    link = link.trimmed();

    if (_youtubeInfo.userSpecified != link)
    {
        bool preConnected = _youtubeInfo.broadcastConnected;
        QString preBroadcastId = _youtubeInfo.broadcastId;

        _youtubeInfo = YouTubeInfo();

        if (_youtubeInfo.broadcastConnected)
        {
            _youtubeInfo.broadcastConnected = false;
            emit connectedChanged();
        }

        _youtubeInfo.userSpecified = link;
        _youtubeInfo.broadcastId = extractBroadcastId(link);

        if (!_youtubeInfo.broadcastId.isEmpty())
        {
            _youtubeInfo.broadcastChatUrl = QUrl(QString("https://www.youtube.com/live_chat?v=%1")
                    .arg(_youtubeInfo.broadcastId));

            _youtubeInfo.broadcastShortUrl = QUrl(QString("https://youtu.be/%1")
                                 .arg(_youtubeInfo.broadcastId));

            _youtubeInfo.broadcastLongUrl = QUrl(QString("https://www.youtube.com/watch?v=%1")
                                 .arg(_youtubeInfo.broadcastId));

            _youtubeInfo.controlPanelUrl = QUrl(QString("https://studio.youtube.com/video/%1/livestreaming")
                                                         .arg(_youtubeInfo.broadcastId));
        }

        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + _settingsKeyUserSpecifiedLink, _youtubeInfo.userSpecified);
        }

        if (preConnected && !preBroadcastId.isEmpty())
        {
            emit disconnected(preBroadcastId);
        }

        if (_outputToFile)
        {
            _outputToFile->setYouTubeInfo(_youtubeInfo);
        }

        /*qDebug() << "User Specified Link" << _youtubeInfo.userSpecified;
        qDebug() << "Broadcast ID:" << _youtubeInfo.broadcastId;
        qDebug() << "Broadcast URL:" << _youtubeInfo.broadcastURL.toString();
        qDebug() << "Chat URL:" << _youtubeInfo.broadcastChatURL.toString();*/

        _webPage.load(QUrl(_youtubeInfo.broadcastChatUrl));

        emit linkChanged();
    }
}
