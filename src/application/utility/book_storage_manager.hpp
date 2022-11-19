#pragma once
#include "i_book_storage_gateway.hpp"
#include "i_book_storage_manager.hpp"
#include "i_downloaded_books_tracker.hpp"

namespace application::utility
{

class BookStorageManager : public IBookStorageManager
{
public:
    BookStorageManager(IBookStorageGateway* bookStorageGateway,
                       IDownloadedBooksTracker* downloadedBooksTracker);

    void addBook(const domain::models::Book& bookToAdd) override;
    void deleteBook(const QUuid& uuid) override;
    void uninstallBook(const QUuid& uuid) override;
    void updateBook(const domain::models::Book& newBook) override;
    std::vector<domain::models::Book> loadBooks() override;

private:
    IBookStorageGateway* m_bookStorageGateway;
    IDownloadedBooksTracker* m_downloadedBooksTracker;
};

}  // namespace application::utility