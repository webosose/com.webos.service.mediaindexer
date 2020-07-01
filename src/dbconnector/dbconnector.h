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

#pragma once

#include "logging.h"

#include <luna-service2/lunaservice.h>
#include <pbnjson.hpp>

#include <string>
#include <mutex>
#include <map>
#include <list>


/// Connector to com.webos.service.db.
class DbConnector
{
public:
    /**
     * \brief Configure luna service handle.
     *
     * This should be called before any object tries to request device
     * state change notifications.
     *
     * \param[in] lsHandle Luna service handle to use.
     */
    static void init(LSHandle *lsHandle);


    LSHandle *getHandle() { return lsHandle_; };

    /**
     * \brief Add subscriber for subscription messages
     *
     * \param[in] msg The Luna message.
     */
    bool addSubscriber(LSHandle *handle, LSMessage *msg);

    /**
     * \brief Remove subscriber for subscription messages
     *
     * \param[in] msg The Luna message.
     */
    bool removeSubscriber(LSHandle *handle, LSMessage *msg, const std::string key);

    bool sendNotification(LSHandle *handle, std::string &message, const std::string &key);
protected:
    /// Session data attached to each luna request
    struct SessionData {
        /// A method name to identify the action.
        std::string method;
        /// Some arbitrary object.
        void *object;
    };

    virtual ~DbConnector();

    /**
     * \brief Db service response handler.
     *
     * \return Result of message processing.
     */
    virtual bool handleLunaResponse(LSMessage *msg) = 0;

    /**
     * \brief Send mergePut request with uri.
     *
     * The _kind property will be added to the props from this
     * method. The method will also set the query to search for
     * matching uris.
     *
     * \param[in] uri The object uri.
     * \param[in] precise Make precise uri match or not.
     * \param[in] props JSON object with the properties to be updated.
     * \param[in] obj Some object to send with the luna request.
     * \return True on success, false on error.
     */
    virtual bool mergePut(const std::string &uri, bool precise,
        pbnjson::JValue &props, void *obj = nullptr);
    virtual bool mergePut(const std::string &kind_names, const std::string &uri, bool precise,
        pbnjson::JValue &props, void *obj = nullptr);

    /**
     * \brief Send find request with uri.
     *
     * \param[in] uri The object uri.
     * \param[in] precise Find precise matches or 'starts with'.
     * \param[in] obj Some object to send with the luna request.
     * \return True on success, false on error.
     */
    virtual bool find(const std::string &uri, bool precise = true,
        void *obj = nullptr);
    virtual bool find(const std::string &kind_names, const std::string &uri, bool precise = true,
        void *obj = nullptr);

    virtual bool search(const std::string &kind_names, pbnjson::JValue &selects,
        const std::string &prop, const std::string &val, bool precise = true, void *obj = nullptr);

    /**
     * \brief Delete all objects with the given uri.
     *
     * \param[in] uri The object uri.
     * \param[in] precise Find precise matches or 'starts with'.
     * \return True on success, false on error.
     */
    virtual bool del(const std::string &uri, bool precise = true);
    virtual bool del(const std::string &kind_names, const std::string &uri, bool precise = true);

    /**
     * \brief Give read only access to other services.
     *
     * \param[in] services List of service names.
     * \return True on success, false on error.
     */
    virtual bool roAccess(std::list<std::string> &services);

    /// Get message id.
    LOG_MSGID;

    /// Each specific database connection will be a singleton.
    DbConnector(const char *kindId);
    DbConnector();

    /// Ensure database kind.
    virtual void ensureKind();
    void ensureKind(const std::string &kind_names);

    /// Should be set from connector class constructor
    std::string kindId_;
    /// Should be set from connector class constructor, gives us the
    /// indexes for kind creation
    pbnjson::JArray kindIndexes_;
    pbnjson::JArray uriIndexes_;

    /// Get message token to classify response and get attached data.
    bool sessionDataFromToken(LSMessageToken token, SessionData *sd);

private:

    /// Do not use.
    //DbConnector();

    /// Db service url.
    static const char *dbUrl_;

    /// Luna service handle.
    static LSHandle *lsHandle_;

    /// Map of luna service message tokens and the method along with
    /// some call specific user data.
    std::map<LSMessageToken, DbConnector::SessionData> messageMap_;

    /// Callback for luna responses.
    static bool onLunaResponse(LSHandle *lsHandle, LSMessage *msg, void *ctx);

    /// Needed for the session data map.
    mutable std::mutex lock_;

    /// Remember session data.
    void rememberSessionData(LSMessageToken token, const std::string &method,
        void *object);
};
