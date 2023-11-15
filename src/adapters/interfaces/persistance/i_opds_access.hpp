#pragma once
#include <QObject>
#include "adapters_export.hpp"

namespace  adapters {


class ADAPTERS_EXPORT IOpdsAccess: public QObject
{
   Q_OBJECT

public:
   virtual ~IOpdsAccess() noexcept = default;
    virtual void loadRootLib(const QString& url) = 0;

signals:
    void loadOpdsRootFinished(const QByteArray& data);

};



} // namespace  adapters