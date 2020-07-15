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

#include "gstreamerextractor.h"

#include <gst/gst.h>
#include <png.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <iostream>
#include <fstream>
#include <csetjmp>
#include <vector>

#define CAPS "video/x-raw,format=RGBA,width=160,height=160,pixel-aspect-ratio=1/1"

#define RETURN_IF_FAILED(object, state, message) \
do { \
    LOG_ERROR(0, message); \
    gst_element_set_state (object, state); \
    gst_object_unref (object); \
    return false; \
} while(0)

std::map<std::string, MediaItem::Meta> GStreamerExtractor::metaMap_ = {
    {GST_TAG_TITLE,                     MediaItem::Meta::Title},
    {GST_TAG_GENRE,                     MediaItem::Meta::Genre},
    {GST_TAG_ALBUM,                     MediaItem::Meta::Album},
    {GST_TAG_ARTIST,                    MediaItem::Meta::Artist},
    {GST_TAG_ALBUM_ARTIST,              MediaItem::Meta::AlbumArtist},
    {GST_TAG_TRACK_NUMBER,              MediaItem::Meta::Track},
    {GST_TAG_TRACK_COUNT,               MediaItem::Meta::TotalTracks},
    {GST_TAG_DATE_TIME,                 MediaItem::Meta::DateOfCreation},
    {GST_TAG_DURATION,                  MediaItem::Meta::Duration},
    {GST_TAG_GEO_LOCATION_LONGITUDE,    MediaItem::Meta::GeoLocLongitude},
    {GST_TAG_GEO_LOCATION_LATITUDE,     MediaItem::Meta::GeoLocLatitude},
    {GST_TAG_GEO_LOCATION_COUNTRY,      MediaItem::Meta::GeoLocCountry},
    {GST_TAG_GEO_LOCATION_CITY,         MediaItem::Meta::GeoLocCity},
    {GST_TAG_THUMBNAIL,                 MediaItem::Meta::Thumbnail}
};

GStreamerExtractor::StreamMeta &operator++(GStreamerExtractor::StreamMeta &meta)
{
    if (meta == GStreamerExtractor::StreamMeta::EOL)
        return meta;
    meta = static_cast<GStreamerExtractor::StreamMeta>(static_cast<int>(meta) + 1);
    return meta;
}

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
    bool force_sw_decoders = false;
    GstDiscoverer *discoverer = gst_discoverer_new(GST_SECOND, NULL);
    GError *error;

    if (!discoverer)
        return;

    std::string uri = "file://";
    uri.append(mediaItem.path());

    LOG_DEBUG("Extract meta data from '%s' (%s) with GstDiscoverer",
        uri.c_str(), MediaItem::mediaTypeToString(mediaItem.type()).c_str());

    pbnjson::JValue root = pbnjson::JDomParser::fromFile(JSON_CONFIGURATION_FILE);
    if (!root.isObject()) {
        LOG_DEBUG("configulation parsing error! set force-sw-decoders property!");
        force_sw_decoders = true;
    } else {
        JSonParser parser(root.stringify().c_str());
        force_sw_decoders = parser.get<bool>("force-sw-decoders");
    }

    g_object_set(discoverer, "force-sw-decoders", force_sw_decoders, NULL);

    GstDiscovererInfo *discoverInfo =
        gst_discoverer_discover_uri(discoverer, uri.c_str(), &error);

    if (!discoverInfo) {
        LOG_ERROR(0, "GStreamer discoverer failed on '%s' with '%s'",
            uri.c_str(), error->message);
        GstDiscovererResult result = gst_discoverer_info_get_result(discoverInfo);
        switch (result) {
        case GST_DISCOVERER_MISSING_PLUGINS: {
            int idx = 0;
            const gchar** installer_details =
                gst_discoverer_info_get_missing_elements_installer_details(discoverInfo);
            LOG_ERROR(0, "<Missing plugins for discoverer>");
            while (installer_details[idx]) {
                LOG_ERROR(0,"-> '%s'", installer_details[idx]);
                idx++;
            }
            break;
        }
        default:
            break;
        }

        if (error)
            g_error_free(error);
        g_object_unref(discoverer);
        return;
    }

    // get stream info from discover info.
    GstDiscovererStreamInfo *streamInfo = 
        gst_discoverer_info_get_stream_info(discoverInfo);

    if (!streamInfo)
        goto out;

    switch (mediaItem.type()) {
    case MediaItem::Type::Audio: {
        setMeta(mediaItem, discoverInfo, GST_TAG_TITLE);
        setMeta(mediaItem, discoverInfo, GST_TAG_DATE_TIME);
        setMeta(mediaItem, discoverInfo, GST_TAG_GENRE);
        setMeta(mediaItem, discoverInfo, GST_TAG_ALBUM);
        setMeta(mediaItem, discoverInfo, GST_TAG_ARTIST);
        setMeta(mediaItem, discoverInfo, GST_TAG_ALBUM_ARTIST);
        setMeta(mediaItem, discoverInfo, GST_TAG_DURATION);
        setStreamMeta(mediaItem, streamInfo);
        break;
    }
    case MediaItem::Type::Video: {
        setMeta(mediaItem, discoverInfo, GST_TAG_TITLE);
        setMeta(mediaItem, discoverInfo, GST_TAG_DATE_TIME);
        setMeta(mediaItem, discoverInfo, GST_TAG_GENRE);
        setMeta(mediaItem, discoverInfo, GST_TAG_DURATION);
        setMeta(mediaItem, discoverInfo, GST_TAG_THUMBNAIL);
        setStreamMeta(mediaItem, streamInfo);
        break;
    }
    case MediaItem::Type::Image: {
        setMeta(mediaItem, discoverInfo, GST_TAG_TITLE);
        setMeta(mediaItem, discoverInfo, GST_TAG_DATE_TIME);
        setMeta(mediaItem, discoverInfo, GST_TAG_GEO_LOCATION_LONGITUDE);
        setMeta(mediaItem, discoverInfo, GST_TAG_GEO_LOCATION_LATITUDE);
        setMeta(mediaItem, discoverInfo, GST_TAG_GEO_LOCATION_COUNTRY);
        setMeta(mediaItem, discoverInfo, GST_TAG_GEO_LOCATION_CITY);
        setStreamMeta(mediaItem, streamInfo);
        break;
    }
    case MediaItem::Type::EOL:
        std::abort();
    }

    setMetaCommon(mediaItem);

 out:
    if (error)
        g_error_free(error);
    if (discoverInfo)
        g_object_unref(discoverInfo);
    if (streamInfo)
        gst_discoverer_stream_info_unref(streamInfo);
    g_object_unref(discoverer);
}

bool GStreamerExtractor::saveBufferToImage(void *data, int32_t width, int32_t height,
                                  const std::string &filename, const std::string &ext) const
{
    auto writeData = [&](uint8_t *_data, uint32_t _dataSize) -> bool {
        LOG_DEBUG("Save Attached Image, fullpath : %s",filename.c_str());
        std::ofstream ofs(filename, std::ios_base::out | std::ios_base::binary);
        ofs.write(reinterpret_cast<char *>(_data), _dataSize);
        if (ofs.fail())
        {
            LOG_ERROR(0, "Failed to write attached image %s to device", filename.c_str());
            return false;
        }
        ofs.flush();
        ofs.close();
        return true;
    };
    if (ext == "jpg")
    {
        tjhandle tjInstance = NULL;
        uint8_t *outData = NULL;
        unsigned long outDataSize = 0;
        int32_t outSubSample = TJSAMP_420;
        int32_t flag = TJFLAG_FASTDCT;
        int32_t format = TJPF_RGBA;
        int32_t quality = 75;
        if ((tjInstance = tjInitCompress()) == NULL)
        {
            LOG_ERROR(0, "instance initialization failed");
            return false;
        }

        if (tjCompress2(tjInstance, static_cast<uint8_t *>(data), width, 0, height, format,
                    &outData, &outDataSize, outSubSample, quality, flag) < 0)
        {
            LOG_ERROR(0, "Image compression failed");
            return false;
        }
        tjDestroy(tjInstance);  tjInstance = NULL;
        return writeData(outData, outDataSize);
    }
    else if (ext == "png")
    {
        unsigned char *pdata = static_cast<unsigned char *>(data);
        std::ofstream ofile(filename, std::ios_base::out | std::ios_base::binary);
        png_structp p = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!p)
        {
            LOG_ERROR(0, "Failed to create png struct");
            return false;
        }
        png_infop pinfo = png_create_info_struct(p);
        if (!pinfo)
        {
            LOG_ERROR(0, "Failed to create png info struct");
            return false;
        }
        if (setjmp(png_jmpbuf(p)))
        {
            LOG_ERROR(0, "Error during write header");
            return false;
        }
        png_set_IHDR(p, pinfo, width, height, 8,
                    PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        std::vector<unsigned char*> prows(height);
        for (int32_t row = 0; row < height; ++row)
            prows[row] = pdata + row * (width * 4);
        png_set_rows(p, pinfo, &prows[0]);
        png_set_write_fn(p, &ofile,
            [](png_structp png_ptr, png_bytep png_data, size_t length)->void {
                std::ofstream *ofs = (std::ofstream *)png_get_io_ptr(png_ptr);
                ofs->write(reinterpret_cast<char *>(png_data), length);
                if (ofs->fail())
                {
                    LOG_ERROR(0, "Failed to write attached image to device");
                    return;
                }
            },
        NULL);
        png_write_png(p, pinfo, PNG_TRANSFORM_IDENTITY, NULL);
        ofile.flush();
        ofile.close();
    }
    else if (ext == "bmp")
    {
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data (static_cast<uint8_t *>(data),
            GDK_COLORSPACE_RGB, TRUE, 8, width, height,
            GST_ROUND_UP_4 (width * 4), NULL, NULL);
        GSList *sformats;
        GError *error = nullptr;

        if (!gdk_pixbuf_save (pixbuf, filename.c_str(), ext.c_str(), &error, NULL)) {
            LOG_ERROR(0, "Failed to save thumbnail image, Error Message : %s", error->message);
        }
    }
    else
    {
        LOG_ERROR(0, "Invalid format is requested, supported format : jpg, png, bmp");
        return false;
    }


    return true;
}

bool GStreamerExtractor::getThumbnail(MediaItem &mediaItem, std::string &filename, const std::string &ext) const
{
    LOG_DEBUG("Thumbnail Image creation start");

    auto begin = std::chrono::high_resolution_clock::now();;
    MediaItem::Type type = mediaItem.type();
    if (type != MediaItem::Type::Video && type != MediaItem::Type::Image)
    {
        LOG_ERROR(0, "Invalid Type of media(%s) for extracting thumbnail", \
            MediaItem::mediaTypeToString(mediaItem.type()).c_str());
        return false;
    }
    std::string uri = "file://";
    uri.append(mediaItem.path());
    //filename = THUMBNAIL_DIRECTORY + mediaItem.uuid() + "/" + std::filesystem::path(uri).stem().string() + "." + ext;
    filename = THUMBNAIL_DIRECTORY + mediaItem.uuid() + "/" + randFilename() + "." + ext;

    GstElement *pipeline = nullptr;
    GstElement *videoSink = nullptr;
    gint width, height;
    GstSample *sample;
    gchar *pipelineStr = nullptr;
    GError *error = nullptr;
    gint64 duration, position;
    GstStateChangeReturn ret;
    GstMapInfo map;

    gboolean res;
    LOG_DEBUG("uri : \"%s\"", uri.c_str());
    pipelineStr = g_strdup_printf("uridecodebin uri=\"%s\" ! queue ! qvconv ! "" appsink name=video-sink caps=\"" CAPS "\"", uri.c_str());    
    pipeline = gst_parse_launch(pipelineStr, &error);
    if (error != nullptr)
    {
        LOG_ERROR(0, "Failed to establish pipeline, Error Message : %s", error->message);
        g_error_free(error);
        return false;
    }

    videoSink = gst_bin_get_by_name (GST_BIN (pipeline), "video-sink");
    if (videoSink == nullptr)
        RETURN_IF_FAILED(pipeline, GST_STATE_NULL, "Failed to get video sink");    

    ret = gst_element_set_state (pipeline, GST_STATE_PAUSED);
    //ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    LOG_DEBUG("gst_element_set_state GST_STATE_PAUSED OK");
    switch (ret)
    {
        case GST_STATE_CHANGE_FAILURE:
            RETURN_IF_FAILED(pipeline, GST_STATE_NULL, "failed to play the file");
        case GST_STATE_CHANGE_NO_PREROLL:
            RETURN_IF_FAILED(pipeline, GST_STATE_NULL, "live sources not supported");
        default:
            break;
    }

    ret = gst_element_get_state (pipeline, NULL, NULL, 5 * GST_SECOND);
    if (ret == GST_STATE_CHANGE_FAILURE)
        RETURN_IF_FAILED(pipeline, GST_STATE_NULL, "failed to play the file");

    gst_element_query_duration (pipeline, GST_FORMAT_TIME, &duration);

    if (duration != -1)
        position = duration * 50 / 100;
    else
        position = 1 * GST_SECOND;

    gst_element_seek(pipeline, 1.0f, GST_FORMAT_TIME,
                   GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                   GST_SEEK_TYPE_SET, position,
                   GST_SEEK_TYPE_NONE, 0);

    g_signal_emit_by_name (videoSink, "pull-preroll", &sample, NULL);
    gst_object_unref (videoSink);

    if (sample)
    {
        GstBuffer *buffer;
        GstCaps *caps;
        GstStructure *s;

        caps = gst_sample_get_caps (sample);
        if (!caps)
            RETURN_IF_FAILED(pipeline, GST_STATE_NULL, "could not get snapshot format");
        s = gst_caps_get_structure (caps, 0);

        res = gst_structure_get_int (s, "width", &width);
        res |= gst_structure_get_int (s, "height", &height);
        if (!res)
            RETURN_IF_FAILED(pipeline, GST_STATE_NULL, "could not get resolution information");

        buffer = gst_sample_get_buffer (sample);
        gst_buffer_map (buffer, &map, GST_MAP_READ);

        if (!saveBufferToImage(map.data, width, height, filename, ext))
        {
            RETURN_IF_FAILED(pipeline, GST_STATE_NULL, "could not save thumbnail image");
        }

        gst_buffer_unmap (buffer, &map);

    }
    else
    {
        LOG_ERROR(0, "could not make snapshot");
    }
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);
    
    auto end = std::chrono::high_resolution_clock::now();;
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    LOG_DEBUG("Thumbnail Image creation done, elapsed time = %d [ms]", elapsedTime);
    return true;
}


MediaItem::Meta GStreamerExtractor::metaFromTag(const char *gstTag) const
{
    auto iter = metaMap_.find(gstTag);
    
    if (iter != metaMap_.end())
        return iter->second;

    LOG_ERROR(0, "Failed to find meta for tag %s",gstTag);
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
    } else if (!strcmp(tag, GST_TAG_THUMBNAIL)) {
        LOG_DEBUG("Generate Thumbnail image");
        std::string fname = "";
        if (!getThumbnail(mediaItem, fname)) {
            LOG_ERROR(0, "Failed to get thumbnail image from media item");
            return;
        } else {
            data = {fname};
        }
    } else {
        return;
    }

    auto meta = metaFromTag(tag);
    LOG_DEBUG("Found tag for '%s'", MediaItem::metaToString(meta).c_str());
    mediaItem.setMeta(meta, data);

    g_value_unset(&val);
}
/*
MediaItem::Meta GStreamerExtractor::metaFromStreamInfo(GStreamerExtractor::StreamMeta &meta) const
{
    for (auto type = GStreamerExtractor::StreamMeta::SampleRate;
            type < GStreamerExtractor::StreamMeta::EOL; ++type) {
    }
    return MediaItem::Meta::EOL;
}
*/


void GStreamerExtractor::setStreamMeta(MediaItem &mediaItem,
                                       GstDiscovererStreamInfo *streamInfo) const
{
    MediaItem::MetaData data;
    MediaItem::Meta meta;

    if (!streamInfo) {
        return;
    }

    switch (mediaItem.type()) {
    case MediaItem::Type::Audio: {
        if (GST_IS_DISCOVERER_AUDIO_INFO(streamInfo)) {
            /* let's get the audio meta data */
            LOG_INFO(0, "<Audio stream info>");
            GstDiscovererAudioInfo *audio_info =
                reinterpret_cast<GstDiscovererAudioInfo*>(streamInfo);
            // SampleRate
            data = {gst_discoverer_audio_info_get_sample_rate(audio_info)};
            LOG_INFO(0, " -> Audio SampleRate : %u", std::get<std::uint32_t>(data));
            meta = MediaItem::Meta::SampleRate;
            mediaItem.setMeta(meta, data);

            // Channels
            data = {gst_discoverer_audio_info_get_channels(audio_info)};
            LOG_INFO(0, " -> Audio Channels : %u", std::get<std::uint32_t>(data));
            meta = MediaItem::Meta::Channels;
            mediaItem.setMeta(meta, data);

            // BitRate
            data = {gst_discoverer_audio_info_get_bitrate(audio_info)};
            LOG_INFO(0, " -> Audio BitRate : %u", std::get<std::uint32_t>(data));
            meta = MediaItem::Meta::BitRate;
            mediaItem.setMeta(meta, data);

            // BitPerSample
            data = {gst_discoverer_audio_info_get_depth(audio_info)};
            LOG_INFO(0, " -> Audio BitPerSample : %u", std::get<std::uint32_t>(data));
            meta = MediaItem::Meta::BitPerSample;
        }
        break;
    }
    case MediaItem::Type::Video: {
        if (GST_IS_DISCOVERER_VIDEO_INFO(streamInfo)) {
            /* let's get the video meta data */
            LOG_INFO(0, "<Video stream info>");
            GstDiscovererVideoInfo *video_info =
                reinterpret_cast<GstDiscovererVideoInfo*>(streamInfo);
            // Width
            data = {gst_discoverer_video_info_get_width(video_info)};
            LOG_INFO(0, " -> Video Width : %u", std::get<std::uint32_t>(data));
            meta = MediaItem::Meta::Width;
            mediaItem.setMeta(meta, data);

            // Height
            data = {gst_discoverer_video_info_get_height(video_info)};
            LOG_INFO(0, " -> Video Height : %u", std::get<std::uint32_t>(data));
            meta = MediaItem::Meta::Height;
            mediaItem.setMeta(meta, data);

            // Frame rate
            uint32_t num = gst_discoverer_video_info_get_framerate_num(video_info);
            uint32_t denom = gst_discoverer_video_info_get_framerate_denom(video_info);
            std::string frame_rate = std::to_string(num) + std::string("/") + std::to_string(denom);
            data = {frame_rate};
            LOG_INFO(0, " -> Video Frame Rate : %s", std::get<std::string>(data));
            meta = MediaItem::Meta::FrameRate;
            mediaItem.setMeta(meta, data);
        }
        break;
    }
    case MediaItem::Type::Image: {
        if (GST_IS_DISCOVERER_VIDEO_INFO(streamInfo)) {
            /* let's get the image meta data */
            LOG_INFO(0, "<Image stream info>");
            GstDiscovererVideoInfo *video_info =
                reinterpret_cast<GstDiscovererVideoInfo*>(streamInfo);
            // Width
            data = {gst_discoverer_video_info_get_width(video_info)};
            LOG_INFO(0, " -> Image Width : %u", std::get<std::uint32_t>(data));
            meta = MediaItem::Meta::Width;
            mediaItem.setMeta(meta, data);

            // Height
            data = {gst_discoverer_video_info_get_height(video_info)};
            LOG_INFO(0, " -> Image Height : %u", std::get<std::uint32_t>(data));
            meta = MediaItem::Meta::Height;
            mediaItem.setMeta(meta, data);
        }
        break;
    }
    default:
        LOG_INFO(0, "Subtitle case. we don't need to get subtitle information yet");
        break;
    }

    GstDiscovererStreamInfo *nextStreamInfo =
        gst_discoverer_stream_info_get_next(streamInfo);

    if (nextStreamInfo) {
        setStreamMeta(mediaItem, nextStreamInfo);
        gst_discoverer_stream_info_unref(nextStreamInfo);
    } else if (GST_IS_DISCOVERER_CONTAINER_INFO(streamInfo)) {
        GList *streams =
            gst_discoverer_container_info_get_streams(GST_DISCOVERER_CONTAINER_INFO(streamInfo));
        for (GList *stream = streams; stream; stream = stream->next) {
            GstDiscovererStreamInfo *strInfo =
                reinterpret_cast<GstDiscovererStreamInfo*>(stream->data);
            setStreamMeta(mediaItem, strInfo);
        }
        gst_discoverer_stream_info_list_free(streams);
    } else {
        // do nothing
    }

}
