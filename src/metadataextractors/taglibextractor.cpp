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
#include "taglibextractor.h"
#include <tag.h>
#include <fileref.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <mpegproperties.h>
//#include <filesystem>
#include <unistd.h>
#include <tfilestream.h>
#include <tstring.h>
#include <tfile.h>
#include <attachedpictureframe.h>
#include <textidentificationframe.h>
#include <xiphcomment.h>
#include <oggfile.h>
#include <vorbisfile.h>
#include <fstream>
#include <cinttypes>

using namespace std;
using namespace TagLib;
using namespace TagLib::ID3v2;
using namespace TagLib::Ogg;

TaglibExtractor::TaglibExtractor()
{
    // nothing to be done here
}

TaglibExtractor::~TaglibExtractor()
{
    // nothing to be done here
}

std::string TaglibExtractor::getTextFrame(TagLib::ID3v2::Tag *tag,  const TagLib::ByteVector &flag) const
{
    std::string ret;
    if (!tag) {
        LOG_ERROR(0, "tag is invalid");
        return "";
    }
    if (!tag->frameListMap()[flag].isEmpty())
    {
        ID3v2::TextIdentificationFrame* frame
            = dynamic_cast<TagLib::ID3v2::TextIdentificationFrame*>(tag->frameListMap()[flag].front());
        if (frame)
        {
            bool isUnicode = (frame->textEncoding() != TagLib::String::Latin1);
            ret = (flag == "TCON") ? tag->genre().toCString(isUnicode) : frame->toString().toCString(isUnicode);
        }
    }
    return ret;
}

std::string TaglibExtractor::saveAttachedImage(MediaItem &mediaItem, TagLib::ID3v2::Tag *tag, const std::string &fname) const
{
    std::string of = "";
    if (tag->frameListMap().contains("APIC"))
    {
        std::string ext = "";
        ID3v2::AttachedPictureFrame *frame
            = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(tag->frameListMap()["APIC"].front());
        if (frame->mimeType().find(TAGLIB_EXT_JPG) || frame->mimeType().find(TAGLIB_EXT_JPG))
            ext = TAGLIB_EXT_JPG;
        else if (frame->mimeType().find(TAGLIB_EXT_PNG))
            ext = TAGLIB_EXT_PNG;

        std::error_code err;
        std::string thumbnailDir = TAGLIB_BASE_DIRECTORY + mediaItem.uuid();
        if (!std::filesystem::is_directory(thumbnailDir))
        {
            if (!std::filesystem::create_directory(thumbnailDir, err))
            {
                LOG_ERROR(0, "Failed to create directory %s, error : %s",thumbnailDir.c_str(), err.message().c_str());
                LOG_DEBUG("Retry with create_directories");
                if (!std::filesystem::create_directories(thumbnailDir, err))
                    LOG_ERROR(0, "Retry Failed, error : %s", err.message().c_str());
            }
        }

        of = TAGLIB_BASE_DIRECTORY + mediaItem.uuid() + "/" + fname + "." + ext;

        LOG_DEBUG("Save Attached Image, fullpath : %s",of.c_str());
        std::ofstream ofs(of, ios_base::out | ios_base::binary);
        ofs.write(frame->picture().data(), frame->picture().size());
        if (ofs.fail())
        {
            LOG_ERROR(0, "Failed to write attached image %s to device", of.c_str());
            return std::string();
        }
        ofs.flush();
        ofs.close();
    }
    return of;
}


bool TaglibExtractor::extractMeta(MediaItem &mediaItem, bool expand) const
{
    std::string uri(mediaItem.path());

    if (mediaItem.type() != MediaItem::Type::Audio) {
        LOG_ERROR(0, "mediaitem type is not audio");
        return false;
    }

    LOG_DEBUG("Extract meta data from '%s' (%s) with TagLib",
        uri.c_str(), MediaItem::mediaTypeToString(mediaItem.type()).c_str());

    if (uri.rfind(TAGLIB_EXT_MP3) != std::string::npos)
    {
        TagLib::MPEG::File f(uri.c_str());
        ID3v2::Tag *tag = f.ID3v2Tag();
        if (!tag || tag->isEmpty())
        {
            LOG_DEBUG("tag for %s is empty", uri.c_str());
            return true;
        }
        LOG_DEBUG("Setting Meta data for Mp3");
        setMetaMp3(mediaItem, tag, f, MediaItem::Meta::Title);
        setMetaMp3(mediaItem, tag, f, MediaItem::Meta::Genre);
        setMetaMp3(mediaItem, tag, f, MediaItem::Meta::Album);
        setMetaMp3(mediaItem, tag, f, MediaItem::Meta::Artist);
        setMetaMp3(mediaItem, tag, f, MediaItem::Meta::Duration);
        setMetaMp3(mediaItem, tag, f, MediaItem::Meta::Thumbnail);
        if (expand) {
            setMetaMp3(mediaItem, tag, f, MediaItem::Meta::DateOfCreation);
            setMetaMp3(mediaItem, tag, f, MediaItem::Meta::AlbumArtist);
            setMetaMp3(mediaItem, tag, f, MediaItem::Meta::Track);
            setMetaMp3(mediaItem, tag, f, MediaItem::Meta::Year);
            setMetaMp3(mediaItem, tag, f, MediaItem::Meta::SampleRate);
            setMetaMp3(mediaItem, tag, f, MediaItem::Meta::BitRate);
            setMetaMp3(mediaItem, tag, f, MediaItem::Meta::Channels);
        }
        LOG_DEBUG("Setting Meta data for Mp3 Done");
    }
    else if (uri.rfind(TAGLIB_EXT_OGG) != std::string::npos)
    {
        TagLib::Vorbis::File oggf(uri.c_str());
        Ogg::XiphComment *tag = oggf.tag();
        if (!tag || tag->isEmpty()) {
            LOG_DEBUG("tag for %s is empty", uri.c_str());
            return true;
        }
        LOG_DEBUG("Setting Meta data for Ogg");
        setMetaOgg(mediaItem, tag, oggf, MediaItem::Meta::Title);
        setMetaOgg(mediaItem, tag, oggf, MediaItem::Meta::Genre);
        setMetaOgg(mediaItem, tag, oggf, MediaItem::Meta::Album);
        setMetaOgg(mediaItem, tag, oggf, MediaItem::Meta::Artist);
        setMetaOgg(mediaItem, tag, oggf, MediaItem::Meta::Duration);
        if (expand) {
            setMetaOgg(mediaItem, tag, oggf, MediaItem::Meta::DateOfCreation);
            setMetaOgg(mediaItem, tag, oggf, MediaItem::Meta::AlbumArtist);
            setMetaOgg(mediaItem, tag, oggf, MediaItem::Meta::Track);
            setMetaOgg(mediaItem, tag, oggf, MediaItem::Meta::Year);
            setMetaOgg(mediaItem, tag, oggf, MediaItem::Meta::SampleRate);
            setMetaOgg(mediaItem, tag, oggf, MediaItem::Meta::BitRate);
            setMetaOgg(mediaItem, tag, oggf, MediaItem::Meta::Channels);
        }
        LOG_DEBUG("Setting Meta data for Ogg Done");
    }
    else
    {
        LOG_ERROR(0, "invalid file, file extension is not .mp3");
        return false;
    }

    setMetaCommon(mediaItem);
    return true;
}

void TaglibExtractor::setMetaMp3(MediaItem &mediaItem, TagLib::ID3v2::Tag *tag,
         TagLib::MPEG::File &file, MediaItem::Meta flag) const

{
    MediaItem::MetaData data;

    switch(flag)
    {
        case MediaItem::Meta::Title:
            data = {getTextFrame(tag, "TIT2")};
            break;
        case MediaItem::Meta::DateOfCreation:
            data = {getTextFrame(tag, "TDRC")};
            break;
        case MediaItem::Meta::Genre:
            data = {getTextFrame(tag, "TCON")};
            break;
        case MediaItem::Meta::Album:
            data = {getTextFrame(tag, "TALB")};
            break;
        case MediaItem::Meta::Artist:
            data = {getTextFrame(tag, "TPE1")};
            break;
        case MediaItem::Meta::AlbumArtist:
            data = {getTextFrame(tag, "TPE2")};
            break;
        case MediaItem::Meta::Year:
            data = {static_cast<std::int32_t>(tag->year())};
            break;
        case MediaItem::Meta::Track:
            data = {getTextFrame(tag, "TPOS")};
            break;
        case MediaItem::Meta::Duration:
            data = {file.audioProperties()->lengthInSeconds()};
            break;
        case MediaItem::Meta::SampleRate:
            data = {file.audioProperties()->sampleRate()};
            break;
        case MediaItem::Meta::BitRate:
            data = {file.audioProperties()->bitrate()};
            break;
        case MediaItem::Meta::Channels:
            data = {file.audioProperties()->channels()};
            break;
        case MediaItem::Meta::Thumbnail:
        {
            std::string baseName = randFilename();//baseFilename(mediaItem, true);//
            std::string outImagePath = saveAttachedImage(mediaItem, tag, baseName);
            if (outImagePath.empty())
            {
                LOG_ERROR(0, "Extracting Image from %s is failed", baseName.c_str());
            }
            else
            {
                LOG_DEBUG("Extracted Image has been saved in %s", outImagePath.c_str());
                data = {outImagePath};
            }
            break;
        }
        default:
            break;
    }

    LOG_DEBUG("Found tag for '%s'", MediaItem::metaToString(flag).c_str());
    mediaItem.setMeta(flag, data);
}

void TaglibExtractor::setMetaOgg(MediaItem &mediaItem, TagLib::Ogg::XiphComment *tag,
    TagLib::Ogg::File &file, MediaItem::Meta flag) const
{
    MediaItem::MetaData data;
    Ogg::FieldListMap fieldListMap = tag->fieldListMap();

    switch(flag)
    {
        case MediaItem::Meta::Title:
            if (tag->contains("TITLE"))
                data = {fieldListMap["TITLE"].toString().toCString(true)};
            break;
        case MediaItem::Meta::DateOfCreation:
            if (tag->contains("DATE"))
                data = {fieldListMap["DATE"].toString().toCString(true)};
            break;
        case MediaItem::Meta::Genre:
            if (tag->contains("GENRE"))
                data = {fieldListMap["GENRE"].toString().toCString(true)};
            break;
        case MediaItem::Meta::Album:
            if (tag->contains("ALBUM"))
                data = {fieldListMap["ALBUM"].toString().toCString(true)};
            break;
        case MediaItem::Meta::Artist:
            if (tag->contains("ARTIST"))
                data = {fieldListMap["ARTIST"].toString().toCString(true)};
            break;
        case MediaItem::Meta::AlbumArtist:
            if (tag->contains("PERFORMER"))
                data = {fieldListMap["PERFORMER"].toString().toCString(true)};
            break;
        case MediaItem::Meta::Year:
            if (tag->contains("YEAR"))
                data = {fieldListMap["YEAR"].toString().toCString(true)};
            break;
        case MediaItem::Meta::Track:
            if (!fieldListMap["TRACKNUMBER"].isEmpty())
                data = {fieldListMap["TRACKNUMBER"].toString().toCString(true)};
            else if (!fieldListMap["TRACKNUM"].isEmpty())
                data = {fieldListMap["TRACKNUM"].toString().toCString(true)};
            break;
        case MediaItem::Meta::Duration:
            data = {file.audioProperties()->lengthInSeconds()};
            break;
        case MediaItem::Meta::SampleRate:
            data = {file.audioProperties()->sampleRate()};
            break;
        case MediaItem::Meta::BitRate:
            data = {file.audioProperties()->bitrate()};
            break;
        case MediaItem::Meta::Channels:
            data = {file.audioProperties()->channels()};
            break;
        case MediaItem::Meta::Thumbnail:
        default:
            break;
    }


    LOG_DEBUG("Found tag for '%s'", MediaItem::metaToString(flag).c_str());
    mediaItem.setMeta(flag, data);
}

