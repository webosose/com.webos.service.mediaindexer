// @@@LICENSE
//
//      Copyright (c) 2017-2018 LG Electronics, Inc.
//
// Confidential computer software. Valid license from LG required for
// possession, use or copying. Consistent with FAR 12.211 and 12.212,
// Commercial Computer Software, Computer Software Documentation, and
// Technical Data for Commercial Items are licensed to the U.S. Government
// under vendor's standard commercial license.
//
// LICENSE@@@

#ifndef INDEXER_SERVICE_CLIENTS_MGR_IMPL_H
#define INDEXER_SERVICE_CLIENTS_MGR_IMPL_H

#include <map>
#include <utility>
#include "logging.h"
#include "indexerserviceclientsmgr.h"

class IndexerServiceClientsMgrImpl : public IndexerServiceClientsMgr
{
public:
    IndexerServiceClientsMgrImpl();
    ~IndexerServiceClientsMgrImpl();

    bool addClient(const std::string &sender,
                   const std::string &method,
                   const LSMessageToken& token);

    bool removeClient(const std::string &sender,
                      const std::string &method,
                      const LSMessageToken& token);

    bool isClientExist(const std::string &sender,
                       const std::string &method,
                       const LSMessageToken& token);
private:
    /// Get message id.
    LOG_MSGID;

    std::map<LSMessageToken, std::pair<std::string, std::string>> clients_;

};

#endif /* INDEXER_SERVICE_CLIENTS_MGR_IMPL_H */

