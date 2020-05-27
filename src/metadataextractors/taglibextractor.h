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

#if defined HAS_TAGLIB
#include <tag.h>
#endif

#include <string.h>

#define TAGLIB_EXT_JPG "jpg"
#define TAGLIB_EXT_JPEG "jpeg"
#define TAGLIB_EXT_PNG "png"
#define TAGLIB_EXT_MP3 "mp3"
#define TAGLIB_BASE_DIRECTORY "/tmp/"
#define TAGLIB_FILE_NAME_SIZE 16
namespace TagLib { namespace ID3v2 { class Tag; } }
namespace TagLib { class ByteVector; }


/**
 * \brief Media parser class for meta data extraction.
 *
 * This class extracts meta data using the Taglib library.
 */
class TaglibExtractor : public IMetaDataExtractor
{
 public:
    TaglibExtractor();
    virtual ~TaglibExtractor();

    /// From interface.
    void extractMeta(MediaItem &mediaItem) const;

 private:
    /// Get message id.
    LOG_MSGID;

    /// Get base filename from mediaItem
    std::string baseFilename(MediaItem &mediaItem, bool noExt = false, std::string delimeter = "//") const;

    /// Get random filename for attached image
    std::string randFilename() const;

    /// Get extension from mediaItem
    std::string extension(MediaItem &mediaItem) const;

    /// Get text frame from id3 tag key value
    std::string getTextFrame(TagLib::ID3v2::Tag &tag,      const TagLib::ByteVector &flag) const;

    /// Get attached image of mp3 from APIC key frame
    std::string saveAttachedImage(TagLib::ID3v2::Tag &tag, const std::string &fname) const;

    /// Set media item media per media type.
    void setMeta(MediaItem &mediaItem, TagLib::ID3v2::Tag &tag,
        MediaItem::Meta flag) const;
};
