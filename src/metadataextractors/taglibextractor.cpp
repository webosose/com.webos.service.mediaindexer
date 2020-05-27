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
//#include <filesystem>
#include <unistd.h>
#include <tfilestream.h>
#include <tstring.h>
#include <tfile.h>
#include <attachedpictureframe.h>
#include <textidentificationframe.h>
#include <fstream>

using namespace std;
using namespace TagLib;

TaglibExtractor::TaglibExtractor()
{
    // nothing to be done here
}

TaglibExtractor::~TaglibExtractor()
{
    // nothing to be done here
}

void TaglibExtractor::extractMeta(MediaItem &mediaItem) const
{
    std::string uri(mediaItem.path());

    FileRef f(uri.c_str());
    if (f.isNull()) {
        LOG_ERROR(0, "file uri(%s) is invalid to extract meta from taglibextractor",
            uri.c_str());
        return;
    }

    if (mediaItem.type() != MediaItem::Type::Audio) {
        LOG_ERROR(0, "mediaitem type is not audio");
        return;
    }

    if (uri.rfind(TAGLIB_EXT_MP3) == std::string::npos) {
        LOG_ERROR(0, "invalid file, file extension is not .mp3");
        return;
    }

    LOG_DEBUG("Extract meta data from '%s' (%s) with TagLib",
        uri.c_str(), MediaItem::mediaTypeToString(mediaItem.type()).c_str());

    ID3v2::Tag tag(f.file(), 0);

    setMeta(mediaItem, tag, MediaItem::Meta::Title);
    setMeta(mediaItem, tag, MediaItem::Meta::DateOfCreation);
    setMeta(mediaItem, tag, MediaItem::Meta::Genre);
    setMeta(mediaItem, tag, MediaItem::Meta::Album);
    setMeta(mediaItem, tag, MediaItem::Meta::Artist);
    setMeta(mediaItem, tag, MediaItem::Meta::AlbumArtist);
    setMeta(mediaItem, tag, MediaItem::Meta::Track);
    setMeta(mediaItem, tag, MediaItem::Meta::Year);
    setMeta(mediaItem, tag, MediaItem::Meta::Duration);
    setMeta(mediaItem, tag, MediaItem::Meta::Image);


}

/// Get base filename from mediaItem
std::string TaglibExtractor::baseFilename(MediaItem &mediaItem, bool noExt, std::string delimeter) const
{
    std::string ret = "";
    std::string path = mediaItem.path();
    if (path.empty())
        return ret;
    ret = path.substr(path.find_last_of(delimeter) + 1);

    if (noExt)
    {
        string::size_type const p(ret.find_last_of('.'));
        string ret = ret.substr(0, p);
    }
    return ret;
}

/// Get random filename for attached image
std::string TaglibExtractor::randFilename() const
{
    size_t size = TAGLIB_FILE_NAME_SIZE;

    std::uint64_t val = 0;
    ifstream fs("/dev/urandom", ios::in|ios::binary);
    if (fs)
    {
        fs.read(reinterpret_cast<char*>(&val), size);
    }
    fs.close();
    return std::to_string(val).substr(0,size);
}

/// Get extension from mediaItem
std::string TaglibExtractor::extension(MediaItem &mediaItem) const
{
    std::string ret = "";
    std::string path = mediaItem.path();
    if (path.empty())
        return ret;
    ret = path.substr(path.find_last_of('.') + 1);
    return ret;
}

std::string TaglibExtractor::getTextFrame(TagLib::ID3v2::Tag &tag,  const TagLib::ByteVector &flag) const
{
    std::string ret;
    if (!tag.frameListMap()[flag].isEmpty())
    {
        ID3v2::TextIdentificationFrame* frame
            = dynamic_cast<TagLib::ID3v2::TextIdentificationFrame*>(tag.frameListMap()[flag].front());
        if (frame)
        {
            bool isUnicode = (frame->textEncoding() != TagLib::String::Latin1);
            ret = (flag == "TCON") ? tag.genre().toCString(isUnicode) : frame->toString().toCString(isUnicode);
        }
    }
    return ret;
}

std::string TaglibExtractor::saveAttachedImage(TagLib::ID3v2::Tag &tag, const std::string &fname) const
{
    std::string of = "";
    if (tag.frameListMap().contains("APIC"))
    {
        std::string ext = "";
        ID3v2::AttachedPictureFrame *frame
            = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(tag.frameListMap()["APIC"].front());
        if (frame->mimeType().find(TAGLIB_EXT_JPG) || frame->mimeType().find(TAGLIB_EXT_JPG))
            ext = TAGLIB_EXT_JPG;
        else if (frame->mimeType().find(TAGLIB_EXT_PNG))
            ext = TAGLIB_EXT_PNG;
        of = TAGLIB_BASE_DIRECTORY + fname + "." + ext;

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


void TaglibExtractor::setMeta(MediaItem &mediaItem, TagLib::ID3v2::Tag &tag,
        MediaItem::Meta flag) const

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
            data = {static_cast<std::int32_t>(tag.year())};
            break;
        case MediaItem::Meta::Track:
            data = {static_cast<std::int32_t>(tag.track())};
            break;
        case MediaItem::Meta::Duration:
            data = {getTextFrame(tag, "TLEN")};
            break;
        case MediaItem::Meta::Image:
        {
            std::string baseName = randFilename();//baseFilename(mediaItem, true);//
            std::string outImagePath = saveAttachedImage(tag, baseName);
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
