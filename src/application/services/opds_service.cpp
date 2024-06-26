#include "opds_service.hpp"
#include <algorithm>
#include <functional>
#include "save_book_helper.hpp"
#include "zip_unpacker.hpp"

namespace application::services
{


OpdsService::OpdsService(IOpdsGateway* opdsGateway)
    : m_opdsGateway(opdsGateway)
{
    connect(m_opdsGateway, &IOpdsGateway::parsingXmlDomCompleted, this,
            &IOpdsService::processNodes);

    connect(m_opdsGateway, &IOpdsGateway::gettingOpdsImagedFinished, this,
            &OpdsService::setOpdsNodeCover);

    connect(m_opdsGateway, &IOpdsGateway::badNetworkResponse, this,
            &IOpdsService::badNetworkResponse);

    // call private method
    connect(m_opdsGateway, &IOpdsGateway::gettingBookMediaChunkReady, this,
            &OpdsService::saveDownloadedBookMediaChunkToFile);

    connect(m_opdsGateway, &IOpdsGateway::gettingBookMediaProgressChanged, this,
            &OpdsService::setMediaDownloadProgressForBook);
}

const std::vector<OpdsNode>& OpdsService::getOpdsNodes()
{
    historyStack.push_back(rootNode);
    loadRootNodesFromFile();
    return m_opdsNodes;
}

void OpdsService::loadRootNodesFromFile()
{
    m_opdsNodes.clear();
    QFile opdsFile = getOpdsJsonFile();
    if(!opdsFile.exists())
    {
        qDebug() << "Json file not found";
        m_opdsNodes.clear();
        return;
    }
    QJsonParseError jsonParseError;
    QJsonDocument jsonDoc;
    // open file
    if(opdsFile.open(QIODevice::ReadOnly))
    {
        QByteArray bytesFromFile = opdsFile.readAll();
        if(bytesFromFile.isEmpty())
        {
            qDebug() << "Opds libs json file is empty";
            return;
        }
        jsonDoc = QJsonDocument::fromJson(bytesFromFile, &jsonParseError);
        if(jsonDoc.isNull())
        {
            qDebug() << "Can't parse opds libs json file. "
                     << jsonParseError.errorString();
            return;
        }
    }
    else
    {
        qDebug() << "Can't open opds libs json file.";
        return;
    }
    opdsFile.close();
    const QJsonArray libsJsonArray = jsonDoc.array();
    if(libsJsonArray.isEmpty())
    {
        qDebug() << "libs array is empty";
        return;
    }
    // create nodes
    for(auto it = libsJsonArray.cbegin(); it != libsJsonArray.cend(); ++it)
    {
        QJsonObject lib(it->toObject());
        if(lib.isEmpty())
            continue;
        m_opdsNodes.emplace_back(lib.value("title").toString(),
                                 "",  // author
                                 lib.value("url").toString(),  // url
                                 lib.value("descr").toString(),  // descr
                                 "", "");
    }
}

// load  entries and links for url
void OpdsService::loadRootLib(const QString& url)
{
    // go to root -> load from json file
    if(url.isEmpty())
        return;
    if(url == "url_root")
    {
        emit nodesVecReplaceStarted();
        historyStack.clear();
        historyStack.push_back(rootNode);
        m_opdsNodes.clear();
        loadRootNodesFromFile();
        emit opdsNodesReady();
        return;
    }
    // go back remove current node from history
    if(historyStack.size() >= 2 &&
       historyStack[historyStack.size() - 2].url == url)
    {
        historyStack.pop_back();
    }
    //  push current to history
    else
    {
        OpdsNode curr_target = findNodeByUrl(url);
        if(curr_target.id == "id_next")
        {
            curr_target.title = "Back";
            curr_target.descr = "Back";
        }
        else
            curr_target.title = "..";
        historyStack.push_back(curr_target);
    }
    m_opdsGateway->loadRootlib(url);
}

// process nodes  recieved from the Gateway
void OpdsService::processNodes(const std::vector<OpdsNode>& nodes_vec)
{
    emit nodesVecReplaceStarted();
    m_opdsNodes.clear();
    if(historyStack.size() >= 2)
    {
        m_opdsNodes.push_back(rootNode);
        OpdsNode backNode = historyStack[historyStack.size() - 2];
        if(rootNode.url != backNode.url)
            m_opdsNodes.push_back(backNode);
    }
    std::copy(nodes_vec.cbegin(), nodes_vec.cend(),
              std::back_inserter(m_opdsNodes));
    emit opdsNodesReady();
}

const OpdsNode OpdsService::findNodeByUrl(const QString& url) const
{
    auto it = std::find_if(m_opdsNodes.cbegin(), m_opdsNodes.cend(),
                           [&url](const OpdsNode& node)
                           {
                               return node.url == url;
                           });
    return it == m_opdsNodes.end() ? OpdsNode() : *it;
}

void OpdsService::getNodeImage(const QString& id)
{
    auto itNode = std::find_if(m_opdsNodes.begin(), m_opdsNodes.end(),
                               [&id](const OpdsNode& node)
                               {
                                   return node.id == id;
                               });
    // if found and has imageUrl
    if(itNode != m_opdsNodes.end() && !(itNode->imageUrl).isEmpty())
    {
        // TODO Handle data uri
        // for data uri
        if(itNode->imageUrl.startsWith("data:"))
        {
            QByteArray data = itNode->imageUrl.toUtf8();
            qsizetype start_index = itNode->imageUrl.indexOf(',');
            if(start_index > 0)
            {
                data.remove(0, start_index + 1);
                data = data.fromBase64(data);
                if(!data.isEmpty())
                {
                    QImage image;
                    image.loadFromData(data);
                    if(!image.isNull())
                    {
                        m_opdsGateway->scaleImage(image);
                        itNode->imageObj = std::move(image);
                        itNode->imageUrl = std::to_string(std::hash<QString> {}(
                                                              itNode->imageUrl))
                                               .c_str();
                        itNode->imgDataReady = true;
                        emit dataChanged(
                            std::distance(m_opdsNodes.begin(), itNode));
                    }
                }
            }
        }
        else if(itNode->imgDataReady)
        {
            emit dataChanged(std::distance(m_opdsNodes.begin(), itNode));
        }
        else
        {
            m_opdsGateway->getOpdsImage(id, itNode->imageUrl);
        }
    }
}

/*
void OpdsService::deleteNodeImage(const QString &id){
    auto itNode= std::find_if(m_opdsNodes.begin(),m_opdsNodes.end(), [&id](const
OpdsNode& node){ return node.id == id;
    });
    // if found
    if (itNode != m_opdsNodes.end()){
        itNode->imageObj=QImage();
    };
}*/


void OpdsService::setOpdsNodeCover(const QString& id, const QImage& data)
{
    auto itNode = std::find_if(m_opdsNodes.begin(), m_opdsNodes.end(),
                               [&id](const OpdsNode& node)
                               {
                                   return node.id == id;
                               });

    // if node was not found
    if(itNode == m_opdsNodes.end())
        return;
    // if found - set image
    itNode->imageObj = std::move(data);
    itNode->imgDataReady = true;
    emit dataChanged(std::distance(m_opdsNodes.begin(), itNode));
}

const QImage* OpdsService::getImageDataByImgUrl(const QString& imgUrl) const
{
    auto itNode = std::find_if(m_opdsNodes.cbegin(), m_opdsNodes.cend(),
                               [&imgUrl](const OpdsNode& node)
                               {
                                   return node.imageUrl == imgUrl;
                               });
    return itNode != m_opdsNodes.cend() && itNode->imgDataReady
               ? &itNode->imageObj
               : nullptr;
}

void OpdsService::getBookMedia(const QString& id, const QString& url)
{
    auto uuid = QUuid::createUuid();
    m_opdsGateway->getBookMedia(id, uuid, url);
}

void OpdsService::saveDownloadedBookMediaChunkToFile(const QString& opdsId,
                                                     const QUuid& uuid,
                                                     const QByteArray& data,
                                                     const QString& format,
                                                     bool isLastChunk)
{
    auto destDir = getLibraryDir();
    QString fileName = uuid.toString(QUuid::WithoutBraces) + "." +
                       format;  //.epub.zip  or .epub
    auto destination = destDir.filePath(fileName);
    application::utility::saveDownloadedBookMediaChunkToFile(
        data, isLastChunk, fileName, destination);
    // finished downloading

    if(isLastChunk)
    {
        // unzipped if zipped
        QString unzipped_dest = application::utility::opds::unzipFile(
            destination, format, destDir, uuid.toString(QUuid::WithoutBraces));
        if(!unzipped_dest.isEmpty())
        {
            destination = unzipped_dest;
        }

        emit gettingBookFinished(QUrl::fromLocalFile(destination).toString(),
                                 opdsId);

    }  //    if(isLastChunk)
}  // OpdsService::saveDownloadedBookMediaChunkToFile

void OpdsService::setupUserData(const QString& token, const QString& email)
{
    Q_UNUSED(token);
    m_userEmail = email;
}

void OpdsService::clearUserData()
{
    m_userEmail.clear();
}

QDir OpdsService::getLibraryDir() const
{
    QDir destDir(
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if(!destDir.exists())
        destDir.mkpath(".");

    destDir.mkdir("local_libraries");
    destDir.cd("local_libraries");
    auto userLibraryName = getUserLibraryName(m_userEmail);
    destDir.mkdir(userLibraryName);
    destDir.cd(userLibraryName);

    return destDir.path();
}

QString OpdsService::getUserLibraryName(const QString& email) const
{
    // Hash the email to get a unique user library name
    auto hash = qHash(email);
    return QString::number(hash);
}

void OpdsService::setMediaDownloadProgressForBook(const QString& id,
                                                  qint64 bytesReceived,
                                                  qint64 bytesTotal)
{
    auto it_node = std::find_if(m_opdsNodes.begin(), m_opdsNodes.end(),
                                [&id](const OpdsNode& node)
                                {
                                    return node.id == id;
                                });
    if(it_node != m_opdsNodes.end())
    {
        it_node->mediaDownloadProgress =
            static_cast<double>(bytesReceived) / bytesTotal;
        emit downloadingBookMediaProgressChanged(
            std::distance(m_opdsNodes.begin(), it_node));
    }
}

void OpdsService::markBookAsDownloaded(const QString& id)
{
    if(id.isEmpty())
        return;
    auto it_node = std::find_if(m_opdsNodes.begin(), m_opdsNodes.end(),
                                [&id](const OpdsNode& node)
                                {
                                    return node.id == id;
                                });
    if(it_node != m_opdsNodes.end())
    {
        it_node->downloaded = true;
        emit bookIsDownloadedChanged(
            std::distance(m_opdsNodes.begin(), it_node));
    }
}

QFile OpdsService::getOpdsJsonFile() const
{
    const QString localAppFolder =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QDir destDir(localAppFolder);
    if(!destDir.exists())
        destDir.mkpath(".");
    const QString filepath = destDir.filePath("opds.json");
    return QFile(filepath);
}

bool OpdsService::saveOpdsLib(const QString& title, const QString& url,
                              const QString& descr)
{
    QFile opdsFile = getOpdsJsonFile();
    bool createNew { false };
    if(!opdsFile.exists())
        createNew = true;
    if(!opdsFile.open(QIODeviceBase::ReadWrite))
        return false;

    // read all from file
    QJsonParseError jsonParseError;
    QJsonDocument jsonDoc;
    // parse to json
    if(!createNew)
    {
        QByteArray bytesFromFile = opdsFile.readAll();
        if(!bytesFromFile.isEmpty())
        {
            jsonDoc = QJsonDocument::fromJson(bytesFromFile, &jsonParseError);
            if(jsonDoc.isNull())
            {
                qDebug() << "Can't parse opds libs json file. "
                         << jsonParseError.errorString();
            }
        }
    }
    QJsonArray libsJsonArray;
    if(jsonDoc.isArray())
        libsJsonArray = jsonDoc.array();

    //  add data to json
    QJsonObject lib { QPair<QString, QString>("title", title),
                      QPair<QString, QString>("url", url),
                      QPair<QString, QString>("descr", descr) };
    libsJsonArray.append(lib);
    jsonDoc.setArray(libsJsonArray);
    QByteArray result = jsonDoc.toJson();
    // save in file
    opdsFile.seek(0);
    if(opdsFile.write(result) == -1)
    {
        qDebug() << "error writing to file";
        opdsFile.close();
        return false;
    }
    opdsFile.resize(opdsFile.pos());
    // close file
    opdsFile.close();
    return true;
}

}  // namespace application::services
