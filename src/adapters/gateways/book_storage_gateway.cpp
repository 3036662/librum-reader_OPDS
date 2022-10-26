#include "book_storage_gateway.hpp"
#include "book.hpp"
#include "book_dto.hpp"
#include "i_book_storage_access.hpp"


using namespace adapters::dtos;
using namespace domain::models;


namespace adapters::gateways
{

BookStorageGateway::BookStorageGateway(IBookStorageAccess* bookStorageAccess)
    : m_bookStorageAccess(bookStorageAccess)
{
    connect(m_bookStorageAccess, &IBookStorageAccess::gettingBooksMetaDataFinished,
            this, &BookStorageGateway::proccessBooksMetadata);
}


void BookStorageGateway::createBook(const QString& authToken, const domain::models::Book& book)
{
    BookDto bookDto
    {
        .uuid = book.getUuid().toString(QUuid::WithoutBraces),
        .title = book.getTitle(),
        .author = book.getAuthor(),
        .filePath = book.getFilePath(),
        .creator = book.getCreator(),
        .creationDate = book.getCreationDate(),
        .format = book.getFormat(),
        .language = book.getLanguage(),
        .documentSize = book.getDocumentSize(),
        .pagesSize = book.getPagesSize(),
        .pageCount = book.getPageCount(),
        .currentPage = book.getCurrentPage(),
        .addedToLibrary = book.getAddedToLibrary(),
        .lastOpened = book.getLastOpened(),
        .cover = book.getCoverAsString()
    };
    
    m_bookStorageAccess->createBook(authToken, bookDto);
}

void BookStorageGateway::deleteBook(const QString& authToken, const QUuid& uuid)
{
    Q_UNUSED(authToken);
    Q_UNUSED(uuid);
}

void BookStorageGateway::updateBook(const QString& authToken, const domain::models::Book& book)
{
    Q_UNUSED(authToken);
    Q_UNUSED(book);
}

void BookStorageGateway::getBooksMetaData(const QString& authToken)
{
    m_bookStorageAccess->getBooksMetaData(authToken);
}

void BookStorageGateway::downloadBook(const QString& authToken, const QUuid& uuid)
{
    Q_UNUSED(authToken);
    Q_UNUSED(uuid);
}

void BookStorageGateway::proccessBooksMetadata(std::vector<QJsonObject>&
                                               jsonBooks)
{
    std::vector<Book> books;
    for(auto& jsonBook : jsonBooks)
    {
        // DB yields the uuid under the name "guid", but the core wants it 
        // as "uuid", here "guid" is renamed to "uuid"
        auto uuid = jsonBook["guid"].toString();
        jsonBook.remove("guid");
        jsonBook.insert("uuid", uuid);
        
        auto book = Book::fromJson(jsonBook);
        book.setDownloaded(false);
        
        books.emplace_back(std::move(book));
    }
    
    emit gettingBooksMetaDataFinished(books);
}

} // namespace adapters::gateways