// Copyright (c) 2019-2020 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "dbconnector.h"

/// From main.cpp.
extern const char *lunaServiceId;

const char *DbConnector::dbUrl_ = "luna://com.webos.service.db";
LSHandle *DbConnector::lsHandle_ = nullptr;

void DbConnector::init(LSHandle * lsHandle)
{
    lsHandle_ = lsHandle;
}

DbConnector::DbConnector(const char *kindId) :
    kindId_(kindId)
{
    // nothing to be done here
}

DbConnector::DbConnector()
{
    // nothing to be done here
}

DbConnector::~DbConnector()
{
    // nothing to be done here
}

void DbConnector::ensureKind()
{
    if (!lsHandle_)
        LOG_CRITICAL(0, "Luna bus handle not set");

    LSError lsError;
    LSErrorInit(&lsError);
    LSMessageToken sessionToken;

    // ensure that kind exists
    std::string url = dbUrl_;
    url += "/putKind";

    auto kind = pbnjson::Object();
    kind.put("id", kindId_);
    kind.put("owner", lunaServiceId);
    kind.put("indexes", kindIndexes_);

    LOG_INFO(0, "Ensure kind '%s'", kindId_.c_str());
    
    if (!LSCall(lsHandle_, url.c_str(), kind.stringify().c_str(),
            DbConnector::onLunaResponse, this, &sessionToken,
            &lsError)) {
        LOG_ERROR(0, "Db service putKind error");
    } else {
        rememberSessionData(sessionToken, "putKind", nullptr);
    }
}

bool DbConnector::mergePut(const std::string &uri, bool precise,
    pbnjson::JValue &props, void *obj)
{
    if (!lsHandle_)
        LOG_CRITICAL(0, "Luna bus handle not set");

    LSError lsError;
    LSErrorInit(&lsError);
    LSMessageToken sessionToken;

    std::string url = dbUrl_;
    url += "/mergePut";

    // query for matching uri
    auto query = pbnjson::Object();
    query.put("from", kindId_);
    auto where = pbnjson::Array();
    auto cond = pbnjson::Object();
    cond.put("prop", "uri");
    cond.put("op", precise ? "=" : "%");
    cond.put("val", uri);
    where << cond;
    query.put("where", where);

    auto request = pbnjson::Object();
    // set the kind property in case the query fails
    props.put("_kind", kindId_);
    request.put("props", props);
    request.put("query", query);

    LOG_INFO(0, "Send mergePut for '%s'", uri.c_str());

    if (!LSCall(lsHandle_, url.c_str(), request.stringify().c_str(),
            DbConnector::onLunaResponse, this, &sessionToken,
            &lsError)) {
        LOG_ERROR(0, "Db service mergePut error");
        return false;
    } else {
        rememberSessionData(sessionToken, "mergePut", obj);
    }

    return true;
}

bool DbConnector::find(const std::string &uri, bool precise,
    void *obj)
{
    if (!lsHandle_)
        LOG_CRITICAL(0, "Luna bus handle not set");

    LSError lsError;
    LSErrorInit(&lsError);
    LSMessageToken sessionToken;

    std::string url = dbUrl_;
    url += "/find";

    // query for matching uri
    auto query = pbnjson::Object();
    query.put("from", kindId_);
    auto where = pbnjson::Array();
    auto cond = pbnjson::Object();
    cond.put("prop", "uri");
    cond.put("op", precise ? "=" : "%");
    cond.put("val", uri);
    where << cond;
    query.put("where", where);

    auto request = pbnjson::Object();
    request.put("query", query);

    LOG_INFO(0, "Send find for '%s'", uri.c_str());

    if (!LSCall(lsHandle_, url.c_str(), request.stringify().c_str(),
            DbConnector::onLunaResponse, this, &sessionToken,
            &lsError)) {
        LOG_ERROR(0, "Db service find error");
        return false;
    } else {
        rememberSessionData(sessionToken, "find", obj);
    }

    return true;
}

bool DbConnector::search(const std::string &uri, bool precise,
    void *obj)
{
    if (!lsHandle_)
        LOG_CRITICAL(0, "Luna bus handle not set");

    LSError lsError;
    LSErrorInit(&lsError);
    LSMessageToken sessionToken;

    std::string url = dbUrl_;
    url += "/search";

    // query for matching uri
    auto query = pbnjson::Object();
    query.put("from", kindId_);
    auto where = pbnjson::Array();
    auto cond = pbnjson::Object();
    cond.put("prop", "uri");
    cond.put("op", precise ? "=" : "%");
    cond.put("val", uri);
    where << cond;
    query.put("where", where);

    auto request = pbnjson::Object();
    request.put("query", query);

    LOG_INFO(0, "Send search for '%s'", uri.c_str());

    if (!LSCall(lsHandle_, url.c_str(), request.stringify().c_str(),
            DbConnector::onLunaResponse, this, &sessionToken,
            &lsError)) {
        LOG_ERROR(0, "Db service search error");
        return false;
    } else {
        rememberSessionData(sessionToken, "search", obj);
    }

    return true;
}

bool DbConnector::del(const std::string &uri, bool precise)
{
    if (!lsHandle_)
        LOG_CRITICAL(0, "Luna bus handle not set");

    LSError lsError;
    LSErrorInit(&lsError);
    LSMessageToken sessionToken;

    std::string url = dbUrl_;
    url += "/del";

    // query for matching uri
    auto query = pbnjson::Object();
    query.put("from", kindId_);
    auto where = pbnjson::Array();
    auto cond = pbnjson::Object();
    cond.put("prop", "uri");
    cond.put("op", precise ? "=" : "%");
    cond.put("val", uri);
    where << cond;
    query.put("where", where);

    auto request = pbnjson::Object();
    request.put("query", query);

    LOG_INFO(0, "Send delete for '%s'", uri.c_str());

    if (!LSCall(lsHandle_, url.c_str(), request.stringify().c_str(),
            DbConnector::onLunaResponse, this, &sessionToken,
            &lsError)) {
        LOG_ERROR(0, "Db service delete error");
        return false;
    } else {
        rememberSessionData(sessionToken, "del", nullptr);
    }

    return true;
}

bool DbConnector::roAccess(std::list<std::string> &services)
{
    if (!lsHandle_) {
        LOG_CRITICAL(0, "Luna bus handle not set");
        return false;
    }

    LSError lsError;
    LSErrorInit(&lsError);
    LSMessageToken sessionToken;

    std::string url = dbUrl_;
    url += "/putPermissions";

    auto permissions = pbnjson::Array();
    for (auto s : services) {
        auto perm = pbnjson::Object();
        auto oper = pbnjson::Object();
        oper.put("read", "allow");
        perm.put("operations", oper);
        perm.put("object", kindId_);
        perm.put("type", "db.kind");
        perm.put("caller", s);

        permissions << perm;
    }

    auto request = pbnjson::Object();
    request.put("permissions", permissions);

    LOG_INFO(0, "Send putPermissions");
    LOG_DEBUG("Request : %s", request.stringify().c_str());

    if (!LSCall(lsHandle_, url.c_str(), request.stringify().c_str(),
            DbConnector::onLunaResponse, this, &sessionToken,
            &lsError)) {
        LOG_ERROR(0, "Db service permissions error");
        return false;
    } else {
        rememberSessionData(sessionToken, "putPermissions", nullptr);
    }

    return true;
}

bool DbConnector::sessionDataFromToken(LSMessageToken token, SessionData *sd)
{
    std::lock_guard<std::mutex> lock(lock_);

    auto match = messageMap_.find(token);
    if (match == messageMap_.end())
        return false;

    *sd = match->second;
    messageMap_.erase(match);
    return true;
}

bool DbConnector::onLunaResponse(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    DbConnector *connector = static_cast<DbConnector *>(ctx);
    return connector->handleLunaResponse(msg);
}

void DbConnector::rememberSessionData(LSMessageToken token,
    const std::string &method, void *object)
{
    // remember token for response - we could do that after the
    // request has been issued because the response will happen
    // from the mainloop in the same thread context
    SessionData sd;
    sd.method = method;
    sd.object = object;
    auto p = std::make_pair(token, sd);

    std::lock_guard<std::mutex> lock(lock_);
    messageMap_.emplace(p);
}

bool DbConnector::addSubscriber(LSHandle *handle, LSMessage *msg)
{
    LSError lsError;
    LSErrorInit(&lsError);
    std::string method = LSMessageGetMethod(msg);
    if (method.empty())
    {
        LOG_ERROR(0, "Failed to get method name from ls message, it mandatory to get key for subscription");
        return false;
    }

    if (!LSSubscriptionAdd(handle, method.c_str(), msg, &lsError))
    {
        LOG_ERROR(0, "LSSubscriptionAdd failed: %s", lsError.message);
        return false;
    }
    LOG_DEBUG("Add subscription done, handle : %p, method : %s, sender : %s", handle, method.c_str(), LSMessageGetSender(msg));
    return true;
}

bool DbConnector::removeSubscriber(LSHandle *handle, LSMessage *msg, const std::string key)
{
    LSError lsError;
    LSErrorInit(&lsError);
    LSSubscriptionIter *iter = NULL;

    if (key.empty())
    {
        LOG_ERROR(0, "Invalid key");
        return false;
    }

    const char* sender = LSMessageGetSender(msg);
    if (sender != NULL)
    {
        if (LSSubscriptionAcquire(handle, key.c_str(), &iter, &lsError))
        {
            LSMessage *subscriberMessage;
            while (LSSubscriptionHasNext(iter))
            {
                subscriberMessage = LSSubscriptionNext(iter);
                const char *subscriberSender = LSMessageGetSender(subscriberMessage);
                if (sender == subscriberSender)
                {
                    LSSubscriptionRemove(iter);
                    break;
                }
            }
        }
        else
        {
            LOG_ERROR(0, "LSSubscriptionAcquire Failed");
            return false;
        }
    }
    else
    {
        LOG_ERROR(0, "Invalid sender");
        return false;
    }

    return true;
}

bool DbConnector::sendNotification(LSHandle *handle, std::string &message, const std::string &key)
{
    LSError lsError;
    LSErrorInit(&lsError);

    if (!handle)
    {
        LOG_ERROR(0, "Invalid handle for subscription reply");
        return false;
    }

    if (key.empty())
    {
        LOG_ERROR(0, "Invalid key for subscription reply");
        return false;
    }

    if (!LSSubscriptionReply(handle, key.c_str(), message.c_str(), &lsError)) {
        LOG_ERROR(0, "LSSubscriptionReply failed: %s", lsError.message);
        return false;
    }
    return true;
}

