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

#include <pbnjson.hpp>
#include <luna-service2/lunaservice.h>

class MediaIndexer;

/**
 * \brief Indexer service class.
 *
 * LUNA API of media indexer service:\n
 * \n\b /getPlugin Enable a plugin (with uri) or get all available plugins (without uri).\n
 * Request schema:
 * \code{.json}
 * { "type": "object",
 *   "properties": {
 *       "uri": { "type": "string" }
 *   }
 * } \endcode
 * Response schema:
 * \code{.json}
 * { "type": "object",
 *   "properties": {
 *       "returnValue": { "type": "boolean" }
 *   }
 * } \endcode
 * \n\b /putPlugin Disable a plugin.\n
 * Request schema:
 * \code{.json}
 * { "type": "object",
 *   "properties": {
 *        "uri": { "type": "string" }
 *   },
 *   "required": [ "uri" ] } \endcode
 * Response schema:
 * \code{.json}
 * { "type": "object",
 *   "properties": {
 *       "returnValue": { "type": "boolean" }
 *   }
 * } \endcode
 * \n\b /getPluginList Returns list of available plugins.\n
 * Request is jut an empty object.\n
 * Response example:
 * \code{.json}
 * { "pluginList": [
 *       { "uri": "upnp" },
 *       { "uri": "storage" },
 *       { "uri": "msc" },
 *       ],
 *   "returnValue": true
 * } \endcode
 * \n\b /getDeviceList Returns list of enabled plugins and their devices with meta data.\n
 * Request schema:
 * \code{.json}
 * { "type": "object",
 *   "properties": {
 *       "subscribe": { "type": "boolean" }
 *   },
 *   "required": [ "subscribe" ]
 * } \endcode
 * Response example:
 * \code{.json}
 * { "pluginList": [
 *       { "active": true, "uri": "upnp", "deviceList": [
 *           { "available": true, "uri": "upnp://xxx",
 *             "audioCount": 132, "videoCount": 12, "imageCount": 120
 *           } ]
 *       } ],
 *   "returnValue": true
 * } \endcode
 * \n\b /getPlaybackUri Resolve media item uri to playback uri.\n
 * Request schema:
 * \code{.json}
 * { "type": "object",
 *   "properties": {
 *        "uri": { "type": "string" }
 *   },
 *   "required": [ "uri" ] } \endcode
 * Response schema:
 * \code{.json}
 * { "type": "object",
 *   "properties": {
 *       "playbackUri": { "type": "string" },
 *       "returnValue": { "type": "boolean" }
 *   }
 * } \endcode
 */
class IndexerService
{
public:
    /**
     * \brief Construct luna service for media indexer.
     *
     * \param[in] indexer The media indexer class.
     */
    IndexerService(MediaIndexer *indexer);
    virtual ~IndexerService();

    /**
     * \brief Generate device list object and push either as reply to
     * \p msg or to all registered subscribers.
     *
     * This is called from device observer method which in turn are
     * called with all plugins locked so we are save.
     *
     * \param[in] msg The Luna message.
     */
    bool pushDeviceList(LSMessage *msg = nullptr);

private:
    /// Get message id.
    LOG_MSGID;

    /// Needs indexer class.
    IndexerService();

    /// Schema for getPlugin.
    static pbnjson::JSchema pluginGetSchema_;
    /// Schema for putPlugin.
    static pbnjson::JSchema pluginPutSchema_;
    /// Schema for getDeviceList.
    static pbnjson::JSchema deviceListGetSchema_;
    /// Schema for runDetect and stopDetect.
    static pbnjson::JSchema detectRunStopSchema_;
    /// Schema for getPlaybackUri.
    static pbnjson::JSchema playbackUriGetSchema_;

    /**
     * \brief Callback for getPlugin() Luna method.
     *
     * \param[in] lsHandle Luna service handle.
     * \param[in] msg The Luna message.
     * \param[in] ctx Pointer to IndexerService class instance.
     */
    static bool onPluginGet(LSHandle *lsHandle, LSMessage *msg, void *ctx);

    /**
     * \brief Callback for putPlugin() Luna method.
     *
     * \param[in] lsHandle Luna service handle.
     * \param[in] msg The Luna message.
     * \param[in] ctx Pointer to IndexerService class instance.
     */
    static bool onPluginPut(LSHandle *lsHandle, LSMessage *msg, void *ctx);

    /**
     * \brief Callback for getPluginList() Luna method.
     *
     * \param[in] lsHandle Luna service handle.
     * \param[in] msg The Luna message.
     * \param[in] ctx Pointer to IndexerService class instance.
     */
    static bool onPluginListGet(LSHandle *lsHandle, LSMessage *msg, void *ctx);

    /**
     * \brief Callback for getDeviceList() Luna method.
     *
     * \param[in] lsHandle Luna service handle.
     * \param[in] msg The Luna message.
     * \param[in] ctx Pointer to IndexerService class instance.
     */
    static bool onDeviceListGet(LSHandle *lsHandle, LSMessage *msg, void *ctx);

    /**
     * \brief Callback for runDetect() Luna method.
     *
     * \param[in] lsHandle Luna service handle.
     * \param[in] msg The Luna message.
     * \param[in] ctx Pointer to IndexerService class instance.
     */
    static bool onRun(LSHandle *lsHandle, LSMessage *msg, void *ctx);

    /**
     * \brief Callback for stopDetect() Luna method.
     *
     * \param[in] lsHandle Luna service handle.
     * \param[in] msg The Luna message.
     * \param[in] ctx Pointer to IndexerService class instance.
     */
    static bool onStop(LSHandle *lsHandle, LSMessage *msg, void *ctx);

    /**
     * \brief Callback for getPlaybackUri() Luna method.
     *
     * \param[in] lsHandle Luna service handle.
     * \param[in] msg The Luna message.
     * \param[in] ctx Pointer to IndexerService class instance.
     */
    static bool onGetPlaybackUri(LSHandle *lsHandle, LSMessage *msg, void *ctx);

    /**
     * \brief Combines functionality for onPluginGet and onPluginPut.
     *
     * \param[in] msg The Luna message.
     * \param[in] get Whether to do get or put.
     */
    bool pluginPutGet(LSMessage *msg, bool get);

    /**
     * \brief Combines functionality for onRun and onStop.
     *
     * \param[in] msg The Luna message.
     * \param[in] run Whether to do run or stop.
     */
    bool detectRunStop(LSMessage *msg, bool run);

    /**
     * \brief Check message and add to subscribers if set.
     *
     * Every service that becomes a subscriber for the device list
     * also gets read-only access to the media database.
     *
     * \param[in] msg The received message.
     * \param[in] parser The message parser.
     */
    void checkForDeviceListSubscriber(LSMessage *msg,
        pbnjson::JDomParser &parser);

    /// Service method definitions.
    static LSMethod serviceMethods_[];

    /// Luna service handle.
    LSHandle *lsHandle_;
    /// Media indexer.
    MediaIndexer *indexer_;
};
