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

#include "imetadataextractor.h"
#include "logging.h"

#if defined HAS_GSTREAMER
#include "gstreamerextractor.h"
#endif

#if defined HAS_TAGLIB
#include "taglibextractor.h"
#endif

#include <cinttypes>
#include <fstream>
#include <iostream>


std::unique_ptr<IMetaDataExtractor> IMetaDataExtractor::extractor(
    MediaItem::Type type, std::string &ext) {
#if defined HAS_TAGLIB
    if (type == MediaItem::Type::Audio &&
        (ext.compare(TAGLIB_EXT_MP3) == 0 ||
         ext.compare(TAGLIB_EXT_OGG) == 0)) {
        std::unique_ptr<IMetaDataExtractor>
            extractor(static_cast<IMetaDataExtractor *>(new TaglibExtractor()));
        return extractor;
    }
#endif
#if defined HAS_GSTREAMER
    std::unique_ptr<IMetaDataExtractor>
        extractor(static_cast<IMetaDataExtractor *>(new GStreamerExtractor()));
    return extractor;
#endif

    return nullptr;
}

/// Get base filename from mediaItem
std::string IMetaDataExtractor::baseFilename(MediaItem &mediaItem, bool noExt, std::string delimeter) const
{
    std::string ret = "";
    std::string path = mediaItem.path();
    if (path.empty())
        return ret;
    ret = path.substr(path.find_last_of(delimeter) + 1);

    if (noExt)
    {
        std::string::size_type const p(ret.find_last_of('.'));
        std::string ret = ret.substr(0, p);
    }
    return ret;
}

/// Get random filename for attached image
std::string IMetaDataExtractor::randFilename() const
{
    size_t size = TAGLIB_FILE_NAME_SIZE;

    std::uint64_t val = 0;
    std::ifstream fs("/dev/urandom", std::ios::in|std::ios::binary);
    if (fs)
    {
        fs.read(reinterpret_cast<char*>(&val), size);
    }
    fs.close();
    return std::to_string(val).substr(0,size);
}

/// Get extension from mediaItem
std::string IMetaDataExtractor::extension(MediaItem &mediaItem) const
{
    std::string ret = "";
    std::string path = mediaItem.path();
    if (path.empty())
        return ret;
    ret = path.substr(path.find_last_of('.') + 1);
    return ret;
}

std::optional<IMetaDataExtractor::Date> IMetaDataExtractor::lastModifiedDate(MediaItem &mediaItem, bool formatted, bool localTime) const
{
    std::string path = mediaItem.path();
    if (path.empty())
    {
        LOG_ERROR(0, "Invalid media item path");
        return std::nullopt;
    }

    auto ftime = std::filesystem::last_write_time(path);
    std::stringstream ss;
    if (formatted)
    {
        std::time_t timeFormatted = decltype(ftime)::clock::to_time_t(ftime);
        if (localTime)
        {
            ss << std::put_time(std::localtime(&timeFormatted), "%c %Z");
            LOG_DEBUG("Return time with formatted value %s", ss.str().c_str());
            return ss.str();
        }
        else
        {
            ss << std::put_time(std::gmtime(&timeFormatted), "%c %Z");
            LOG_DEBUG("Return time with formatted value %s", ss.str().c_str());
            return ss.str();
        }
    }
    else
    {
        LOG_DEBUG("Return time with unformatted value %"PRId64"", ftime.time_since_epoch().count());
        return ftime.time_since_epoch().count();
    }
}

void IMetaDataExtractor::setMetaCommon(MediaItem &mediaItem,       MediaItem::Meta flag) const
{
    MediaItem::MetaData data;
    switch(flag)
    {
        case MediaItem::Meta::LastModifiedDate:
        {
            std::string modified = std::get<std::string>(lastModifiedDate(mediaItem).value());
            LOG_DEBUG("Last Modified Date : %s", modified.c_str());
            data = {modified.c_str()};
            break;
        }
        case MediaItem::Meta::LastModifiedDateRaw:
        {
            std::int64_t modified = std::get<std::int64_t>(lastModifiedDate(mediaItem, false).value());
            LOG_DEBUG("Last Modified Date : %"PRId64"", modified);
            data = {modified};
            break;
        }
    }
    LOG_DEBUG("Found tag for '%s'", MediaItem::metaToString(flag).c_str());
    mediaItem.setMeta(flag, data);
}
