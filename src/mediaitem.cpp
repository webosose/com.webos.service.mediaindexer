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

#include "mediaitem.h"
#include "device.h"
#include "plugins/pluginfactory.h"
#include "plugins/plugin.h"
#include <cinttypes>


// Not part of Device class, this is defined at the bottom of device.h
MediaItem::Type &operator++(MediaItem::Type &type)
{
    if (type == MediaItem::Type::EOL)
        return type;
    type = static_cast<MediaItem::Type>(static_cast<int>(type) + 1);
    return type;
}

// TODO: need to change as template
// Not part of Device class, this is defined at the bottom of device.h
MediaItem::Meta &operator++(MediaItem::Meta &meta)
{
    if (meta == MediaItem::Meta::EOL)
        return meta;
    meta = static_cast<MediaItem::Meta>(static_cast<int>(meta) + 1);
    return meta;
}

// Not part of Device class, this is defined at the bottom of device.h
MediaItem::AudioMeta &operator++(MediaItem::AudioMeta &meta)
{
    if (meta == MediaItem::AudioMeta::EOL)
        return meta;
    meta = static_cast<MediaItem::AudioMeta>(static_cast<int>(meta) + 1);
    return meta;
}

// Not part of Device class, this is defined at the bottom of device.h
MediaItem::VideoMeta &operator++(MediaItem::VideoMeta &meta)
{
    if (meta == MediaItem::VideoMeta::EOL)
        return meta;
    meta = static_cast<MediaItem::VideoMeta>(static_cast<int>(meta) + 1);
    return meta;
}

// Not part of Device class, this is defined at the bottom of device.h
MediaItem::ImageMeta &operator++(MediaItem::ImageMeta &meta)
{
    if (meta == MediaItem::ImageMeta::EOL)
        return meta;
    meta = static_cast<MediaItem::ImageMeta>(static_cast<int>(meta) + 1);
    return meta;
}

bool MediaItem::mimeTypeSupported(const std::string &mime)
{
    for (auto type = MediaItem::Type::Audio;
         type < MediaItem::Type::EOL; ++type) {
        auto typeString = MediaItem::mediaTypeToString(type);
        if (!mime.compare(0, typeString.size(), typeString))
            return true;
    }

    LOG_DEBUG("MIME type '%s' not supported", mime.c_str());

    return false;
}

std::string MediaItem::mediaTypeToString(MediaItem::Type type)
{
    switch (type) {
    case MediaItem::Type::Audio:
        return std::string("audio");
    case MediaItem::Type::Video:
        return std::string("video");
    case MediaItem::Type::Image:
        return std::string("image");
    case MediaItem::Type::EOL:
        return "";
    }

    return "";
}

std::string MediaItem::metaToString(MediaItem::Meta meta)
{
    switch (meta) {
    case MediaItem::Meta::Title:
        return std::string("title");
    case MediaItem::Meta::Genre:
        return std::string("genre");
    case MediaItem::Meta::Album:
        return std::string("album");
    case MediaItem::Meta::Artist:
        return std::string("artist");
    case MediaItem::Meta::AlbumArtist:
        return std::string("album_artist");
    case MediaItem::Meta::Track:
        return std::string("track");
    case MediaItem::Meta::TotalTracks:
        return std::string("total_tracks");
    case MediaItem::Meta::DateOfCreation:
        return std::string("date_of_creation");
    case MediaItem::Meta::Duration:
        return std::string("duration");
    case MediaItem::Meta::Year:
        return std::string("year");
    case MediaItem::Meta::Thumbnail:
        return std::string("thumbnail");
    case MediaItem::Meta::GeoLocLongitude:
        return std::string("geo_location_longitude");
    case MediaItem::Meta::GeoLocLatitude:
        return std::string("geo_location_latitude");
    case MediaItem::Meta::GeoLocCountry:
        return std::string("geo_location_country");
    case MediaItem::Meta::GeoLocCity:
        return std::string("geo_location_city");
    case MediaItem::Meta::LastModifiedDate:
        return std::string("last_modified_date");
    case MediaItem::Meta::FileSize:
        return std::string("file_size");
    case MediaItem::Meta::SampleRate:
        return std::string("sample_rate");
    case MediaItem::Meta::Channels:
        return std::string("channels");
    case MediaItem::Meta::BitRate:
        return std::string("bit_rate");
    case MediaItem::Meta::BitPerSample:
        return std::string("bit_per_sample");
    case MediaItem::Meta::Lyric:
        return std::string("lyric");
    case MediaItem::Meta::Width:
        return std::string("width");
    case MediaItem::Meta::Height:
        return std::string("height");
    case MediaItem::Meta::FrameRate:
        return std::string("frame_rate");
    case MediaItem::Meta::EOL:
        return "";
    default:
        return "";
    }

    return "";
}

MediaItem::MediaItem(std::shared_ptr<Device> device, const std::string &path,
    const std::string &mime, unsigned long hash, unsigned long filesize) :
    device_(device),
    type_(Type::EOL),
    hash_(hash),
    filesize_(filesize),
    parsed_(false),
    uri_(""),
    mime_(mime),
    path_(""),
    ext_("")
{
    // create uri
    uri_ = device->uri();
    if (uri_.back() != '/' && path.front() != '/')
        uri_.append("/");
    uri_.append(path);

    path_ = path;
    ext_ = path_.substr(path_.find_last_of('.') + 1);
    // set the type
    for (auto type = MediaItem::Type::Audio;
         type < MediaItem::Type::EOL; ++type) {
        auto typeString = MediaItem::mediaTypeToString(type);
        if (!!mime_.compare(0, typeString.size(), typeString))
            continue;

        type_ = type;
        break;
    }

    if (type_ != Type::EOL)
        device_->incrementMediaItemCount(type_);
}

unsigned long MediaItem::hash() const
{
    return hash_;
}

unsigned long MediaItem::fileSize() const
{
    return filesize_;
}

const std::string &MediaItem::path() const
{
    return path_;
}

const std::string &MediaItem::ext() const
{
    return ext_;
}

std::shared_ptr<Device> MediaItem::device() const
{
    return device_;
}

const std::string &MediaItem::uuid() const
{
    return device_->uuid();
}


std::optional<MediaItem::MetaData> MediaItem::meta(Meta meta) const
{
    auto m = meta_.find(meta);
    if (m != meta_.end())
        return m->second;
    else
        return std::nullopt;
}

void MediaItem::setMeta(Meta meta, MetaData value)
{
    // if meta data is set the media item is supposed to be parsed
    // parsed_ = true;

    switch (value.index()) {
    case 0:
        // valgrind complains about 'std::int64_t' here, for clean
        // valgrind output we need to set 'long'
        LOG_DEBUG("Setting '%s' on '%s' to '%" PRIu64 "'", metaToString(meta).c_str(),
            uri_.c_str(), std::get<std::int64_t>(value));
        break;
    case 1:
        LOG_DEBUG("Setting '%s' on '%s' to '%f'", metaToString(meta).c_str(),
            uri_.c_str(), std::get<double>(value));
        break;
    case 2:
        LOG_DEBUG("Setting '%s' on '%s' to '%d'", metaToString(meta).c_str(),
            uri_.c_str(), std::get<std::int32_t>(value));
        break;
    case 3:
        LOG_DEBUG("Setting '%s' on '%s' to '%s'", metaToString(meta).c_str(),
            uri_.c_str(), std::get<std::string>(value).c_str());
        break;
    case 4:
        LOG_DEBUG("Setting '%s' on '%s' to '%d'", metaToString(meta).c_str(),
            uri_.c_str(), std::get<std::uint32_t>(value));
        break;
    default:
        LOG_DEBUG("Invalid index for setting meta '%s'", uri_.c_str());
        break;
    }

    // make the arist the album artist of none has been set yet
    if (meta == MediaItem::Meta::Artist &&
        !this->meta(MediaItem::Meta::AlbumArtist))
        meta_.insert_or_assign(MediaItem::Meta::AlbumArtist, value);

    // save the meta data
    meta_.insert_or_assign(meta, value);
}

bool MediaItem::parsed() const
{
    return parsed_;
}

const std::string &MediaItem::uri() const
{
    return uri_;
}

const std::string &MediaItem::mime() const
{
    return mime_;
}

MediaItem::Type MediaItem::type() const
{
    return type_;
}

IMediaItemObserver *MediaItem::observer() const
{
    return device_->observer();
}

bool MediaItem::isMediaMeta(Meta meta){
    switch (meta) {
        case MediaItem::Meta::FileSize:
        case MediaItem::Meta::DateOfCreation:
        case MediaItem::Meta::LastModifiedDate:
            return true;
        default:
            return false;
    }
    return false;
}

bool MediaItem::isAudioMeta(Meta meta){
    switch (meta) {
        case MediaItem::Meta::Title:
        case MediaItem::Meta::Genre:
        case MediaItem::Meta::Album:
        case MediaItem::Meta::Artist:
        case MediaItem::Meta::AlbumArtist:
        case MediaItem::Meta::Track:
        case MediaItem::Meta::TotalTracks:
        case MediaItem::Meta::Duration:
        case MediaItem::Meta::Thumbnail:
        case MediaItem::Meta::SampleRate:
        case MediaItem::Meta::BitPerSample:
        case MediaItem::Meta::Channels:
        case MediaItem::Meta::BitRate:
        case MediaItem::Meta::Lyric:
        case MediaItem::Meta::FileSize:
            return true;
        default:
            return false;
    }
    return false;
}

bool MediaItem::isVideoMeta(Meta meta){
    switch (meta) {
        case MediaItem::Meta::Duration:
        case MediaItem::Meta::Width:
        case MediaItem::Meta::Height:
        case MediaItem::Meta::Thumbnail:
        case MediaItem::Meta::FrameRate:
        case MediaItem::Meta::FileSize:
            return true;
        default:
            return false;
    }
    return false;
}

bool MediaItem::isImageMeta(Meta meta){
    switch (meta) {
        case MediaItem::Meta::Width:
        case MediaItem::Meta::Height:
        case MediaItem::Meta::GeoLocLongitude:
        case MediaItem::Meta::GeoLocLatitude:
        case MediaItem::Meta::GeoLocCountry:
        case MediaItem::Meta::GeoLocCity:
        case MediaItem::Meta::FileSize:
            return true;
        default:
            return false;
    }
    return false;
}
