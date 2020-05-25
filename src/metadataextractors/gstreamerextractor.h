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

#include "imetadataextractor.h"

#if defined HAS_GSTREAMER
#include <gst/pbutils/gstdiscoverer.h>
#endif

/**
 * \brief Media parser class for meta data extraction.
 *
 * This class extracts meta data using the GStreamer framework.
 */
class GStreamerExtractor : public IMetaDataExtractor
{
 public:
    GStreamerExtractor();
    virtual ~GStreamerExtractor();

    /// From interface.
    void extractMeta(MediaItem &mediaItem) const;

 private:
    /// Get message id.
    LOG_MSGID;

    /// Get media item meta identifier from GStreamer tag.
    MediaItem::Meta metaFromTag(const char *gstTag) const;
    /// Set media item media per media type.
    void setMeta(MediaItem &mediaItem, const GstDiscovererInfo *metaInfo,
        const char *tag) const;
};
