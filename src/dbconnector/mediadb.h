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
#include <mutex>
#include <list>

class Device;

/// Connector to com.webos.mediadb.
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
     * \brief Check whether db data of media item should be updated or not.
     *
     * If the media item did not change it will be free'd. If it did
     * change, the observer will get notified.
     *
     * \param[in] mediaItem The media item to check.
     */
    bool needUpdate(MediaItem *mediaItem);

    /**
     * \brief Check whether db data of media item include enough information.
     *
     * If db data for media item have missing data, the observer will get notified.
     *
     * \param[in] mediaItem The media item to check.
     * \param[in] val db data for media item(previously stored).
     */
    bool isEnoughInfo(MediaItem *mediaItem, pbnjson::JValue &val);

    /**
     * \brief Update the media item meta in the database.
     *
     * \param[in] mediaItem The media item to update.
     */
    void updateMediaItem(MediaItemPtr mediaItem);

    /**
     * \brief Get the file path for the given media item uri.
     *
     * \param[in] uri Base uri for plugin identification.
     * \return The file path if available.
     */
    std::optional<std::string> getFilePath(const std::string &uri) const;

    /**
     * \brief Mark all media items of this device dirty.
     *
     * \param[in] device The device to mark dirty.
     */
    void markDirty(std::shared_ptr<Device> device, MediaItem::Type type = MediaItem::Type::EOL);

    /**
     * \brief Mark media item as not dirty.
     *
     * \param[in] uri Uri of the media item to unflag.
     */
    void unflagDirty(MediaItemPtr mediaItem);

    /**
     * \brief Remove all dirty flagged items for this device.
     *
     * \param[in] device The device to remove dirties.
     */
    void removeDirty(Device* device);

    /**
     * \brief Add service to db access list.
     *
     * \param[in] serviceName The service name.
     */
    void grantAccess(const std::string &serviceName);

    void grantAccessAll(const std::string &serviceName, bool atomic, pbnjson::JValue &resp);

    bool getAudioList(const std::string &uri, pbnjson::JValue &resp);

    bool getVideoList(const std::string &uri, pbnjson::JValue &resp);

    bool getImageList(const std::string &uri, pbnjson::JValue &resp);

    void makeUriIndex();


protected:
    /// Get message id.
    LOG_MSGID;

    /// Singleton.
    MediaDb();

private:
    /// Singleton object.
    static std::unique_ptr<MediaDb> instance_;
    static std::mutex ctorLock_;
    static std::mutex handlerLock_;
    mutable std::mutex lock_;
    std::map<MediaItem::Type, std::string> kindMap_ = {
        {MediaItem::Type::Audio, AUDIO_KIND},
        {MediaItem::Type::Video, VIDEO_KIND},
        {MediaItem::Type::Image, IMAGE_KIND}
    };
    /// List of services that should have read-only access to
    /// database.
    std::list<std::string> dbClients_;
    std::map<std::string, unsigned long> mediaItemMap_;

    //static constexpr char MEDIA_KIND[]  = "com.webos.service.mediaindexer.media:1";
    static constexpr char AUDIO_KIND[] = "com.webos.service.mediaindexer.audio:1";
    static constexpr char VIDEO_KIND[] = "com.webos.service.mediaindexer.video:1";
    static constexpr char IMAGE_KIND[] = "com.webos.service.mediaindexer.image:1";

    static constexpr char URI[] = "uri";
    static constexpr char HASH[] = "hash";
    static constexpr char DIRTY[] = "dirty";
    static constexpr char TYPE[] = "type";
    static constexpr char MIME[] = "mime";
    static constexpr char FILE_PATH[] = "file_path";

    pbnjson::JValue prepareWhere(const std::string &key,
                                 const std::string &value,
                                 bool precise,
                                 pbnjson::JValue whereClause = pbnjson::Array()) const;

    pbnjson::JValue prepareWhere(const std::string &key,
                                 bool value,
                                 bool precise,
                                 pbnjson::JValue whereClause = pbnjson::Array()) const;
};
