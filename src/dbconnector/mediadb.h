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

#include "dbconnector.h"
#include "mediaitem.h"

#include <memory>
#include <list>

class Device;

/// Connector to com.webos.service.db.
class MediaDb : public DbConnector
{
public:
    /**
     * \brief Get device database connector.
     *
     * \return Singleton object.
     */
    static MediaDb *instance();

    virtual ~MediaDb();

    /**
     * \brief Db service response handler.
     *
     * \return Result of message processing.
     */
    virtual bool handleLunaResponse(LSMessage *msg);

    /**
     * \brief Check media item hash fpr change.
     *
     * If the media item did not change it will be free'd. If it did
     * change, the observer will get notified.
     *
     * \param[in] mediaItem The media item to check.
     */
    void checkForChange(MediaItemPtr mediaItem);

    /**
     * \brief Update the media item meta in the database.
     *
     * \param[in] mediaItem The media item to update.
     */
    void updateMediaItem(MediaItemPtr mediaItem);

    /**
     * \brief Mark all media items of this device dirty.
     *
     * \param[in] device The device to mark dirty.
     */
    void markDirty(std::shared_ptr<Device> device);

    /**
     * \brief Mark media item as not dirty.
     *
     * \param[in] uri Uri of the media item to unflag.
     */
    void unflagDirty(const std::string &uri);

    /**
     * \brief Add service to db access list.
     *
     * \param[in] serviceName The service name.
     */
    void grantAccess(const std::string &serviceName);

protected:
    /// Get message id.
    LOG_MSGID;

    /// Singleton.
    MediaDb();

private:
    /// Singleton object.
    static std::unique_ptr<MediaDb> instance_;

    /// List of services that should have read-only access to
    /// database.
    std::list<std::string> dbClients_;
};
