// Copyright (c) 2019 LG Electronics, Inc.
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

#include "gstreamerextractor.h"

#include <gst/gst.h>

GStreamerExtractor::GStreamerExtractor()
{
    // nothing to be done here
}

GStreamerExtractor::~GStreamerExtractor()
{
    // nothing to be done here
}

void GStreamerExtractor::extractMeta(MediaItem &mediaItem) const
{
    GstDiscoverer *discoverer = gst_discoverer_new(GST_SECOND, NULL);
    GError *error;

    if (!discoverer)
        return;;

    std::string uri = "file://";
    uri.append(mediaItem.path());

    LOG_DEBUG("Extract meta data from '%s' (%s) with GstDiscoverer",
        uri.c_str(), MediaItem::mediaTypeToString(mediaItem.type()).c_str());

    GstDiscovererInfo *discoverInfo =
        gst_discoverer_discover_uri(discoverer, uri.c_str(), &error);

    if (!discoverInfo) {
        LOG_ERROR(0, "GStreamer discoverer failed on '%s' with '%s'",
            uri.c_str(), error->message);
        goto out;
    }

    switch (mediaItem.type()) {
    case MediaItem::Type::Audio: {
        setMeta(mediaItem, discoverInfo, GST_TAG_TITLE);
        setMeta(mediaItem, discoverInfo, GST_TAG_DATE_TIME);
        setMeta(mediaItem, discoverInfo, GST_TAG_GENRE);
        setMeta(mediaItem, discoverInfo, GST_TAG_ALBUM);
        setMeta(mediaItem, discoverInfo, GST_TAG_ARTIST);
        setMeta(mediaItem, discoverInfo, GST_TAG_ALBUM_ARTIST);
        setMeta(mediaItem, discoverInfo, GST_TAG_DURATION);
        break;
    }
    case MediaItem::Type::Video: {
        setMeta(mediaItem, discoverInfo, GST_TAG_TITLE);
        setMeta(mediaItem, discoverInfo, GST_TAG_DATE_TIME);
        setMeta(mediaItem, discoverInfo, GST_TAG_GENRE);
        setMeta(mediaItem, discoverInfo, GST_TAG_DURATION);
        break;
    }
    case MediaItem::Type::Image: {
        setMeta(mediaItem, discoverInfo, GST_TAG_TITLE);
        setMeta(mediaItem, discoverInfo, GST_TAG_DATE_TIME);
        setMeta(mediaItem, discoverInfo, GST_TAG_GEO_LOCATION_LONGITUDE);
        setMeta(mediaItem, discoverInfo, GST_TAG_GEO_LOCATION_LATITUDE);
        setMeta(mediaItem, discoverInfo, GST_TAG_GEO_LOCATION_COUNTRY);
        setMeta(mediaItem, discoverInfo, GST_TAG_GEO_LOCATION_CITY);
        break;
    }
    case MediaItem::Type::EOL:
        std::abort();
    }

 out:
    if (error)
        g_error_free(error);
    if (discoverInfo)
        g_object_unref(discoverInfo);
    g_object_unref(discoverer);
}

MediaItem::Meta GStreamerExtractor::metaFromTag(const char *gstTag) const
{
    if (!strcmp(gstTag, GST_TAG_TITLE))
        return MediaItem::Meta::Title;
    if (!strcmp(gstTag, GST_TAG_GENRE))
        return MediaItem::Meta::Genre;
    if (!strcmp(gstTag, GST_TAG_ALBUM))
        return MediaItem::Meta::Album;
    if (!strcmp(gstTag, GST_TAG_ARTIST))
        return MediaItem::Meta::Artist;
    if (!strcmp(gstTag, GST_TAG_ALBUM_ARTIST))
        return MediaItem::Meta::AlbumArtist;
    if (!strcmp(gstTag, GST_TAG_TRACK_NUMBER))
        return MediaItem::Meta::Track;
    if (!strcmp(gstTag, GST_TAG_TRACK_COUNT))
        return MediaItem::Meta::TotalTracks;
    if (!strcmp(gstTag, GST_TAG_DATE_TIME))
        return MediaItem::Meta::DateOfCreation;
    if (!strcmp(gstTag, GST_TAG_DURATION))
        return MediaItem::Meta::Duration;
    if (!strcmp(gstTag, GST_TAG_GEO_LOCATION_LONGITUDE))
        return MediaItem::Meta::GeoLocLongitude;
    if (!strcmp(gstTag, GST_TAG_GEO_LOCATION_LATITUDE))
        return MediaItem::Meta::GeoLocLatitude;
    if (!strcmp(gstTag, GST_TAG_GEO_LOCATION_COUNTRY))
        return MediaItem::Meta::GeoLocCountry;
    if (!strcmp(gstTag, GST_TAG_GEO_LOCATION_CITY))
        return MediaItem::Meta::GeoLocCity;

    return MediaItem::Meta::EOL;
}

void GStreamerExtractor::setMeta(MediaItem &mediaItem, const GstDiscovererInfo *info,
    const char *tag) const
{
    GValue val = G_VALUE_INIT;
    const GstTagList *metaInfo = gst_discoverer_info_get_tags(info);

    MediaItem::MetaData data;

    // duration is special
    if (!strcmp(tag, GST_TAG_DURATION)) {
        GstClockTime t = gst_discoverer_info_get_duration(info);
        if (!GST_CLOCK_TIME_IS_VALID(t))
            return;
        // we can only do int64_t with libpbnjson :-(
        data = {std::int64_t(t / GST_SECOND)};
    } else if (!!metaInfo && gst_tag_list_copy_value (&val, metaInfo, tag)) {
        // these are the value types that are currently supported
        if (G_VALUE_HOLDS_STRING (&val)) {
            data = {g_value_get_string(&val)};
        } else if (G_VALUE_HOLDS_UINT64 (&val)) {
            // we can only do int64_t with libpbnjson :-(
            data = {std::int64_t(GST_TIME_AS_SECONDS(g_value_get_uint64(&val)))};
        } else if (G_VALUE_HOLDS_DOUBLE (&val)) {
            data = {g_value_get_double(&val)};
        } else if (GST_VALUE_HOLDS_DATE_TIME (&val)) {
            GstDateTime *dateTime = nullptr;
            auto ret = gst_tag_list_get_date_time(metaInfo, tag, &dateTime);
            if (ret) {
                data = {gst_date_time_to_iso8601_string(dateTime)};
                gst_date_time_unref(dateTime);
            }
        } else {
            std::abort(); // we should not request a tag which type is
                          // not supported
        }
    } else if (!strcmp(tag, GST_TAG_TITLE)) {
        auto p = std::filesystem::path(mediaItem.path());
        LOG_DEBUG("Generated title for '%s' is '%s'", mediaItem.uri().c_str(),
            p.stem().c_str());
        data = {p.stem()};
    } else {
        return;
    }

    auto meta = metaFromTag(tag);
    LOG_DEBUG("Found tag for '%s'", MediaItem::metaToString(meta).c_str());
    mediaItem.setMeta(meta, data);

    g_value_unset(&val);
}
