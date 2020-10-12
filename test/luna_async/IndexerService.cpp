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

#include "IndexerService.h"
#include <ctime>
#include <cstdlib>
#include <fstream>

LSMethod IndexerServiceMock::serviceMethods_[] = {
    { "getDeviceList", IndexerServiceMock::onDeviceListGet, LUNA_METHOD_FLAGS_NONE },
    { "getAudioList", IndexerServiceMock::onGetAudioList, LUNA_METHOD_FLAGS_NONE },
    { "getAudioMetadata", IndexerServiceMock::onGetAudioMetaData, LUNA_METHOD_FLAGS_NONE },
    { "getVideoList", IndexerServiceMock::onGetVideoList, LUNA_METHOD_FLAGS_NONE },
    { "getVideoMetadata", IndexerServiceMock::onGetVideoMetaData, LUNA_METHOD_FLAGS_NONE },
    { "getImageList", IndexerServiceMock::onGetImageList, LUNA_METHOD_FLAGS_NONE },
    { "getImageMetadata", IndexerServiceMock::onGetImageMetaData, LUNA_METHOD_FLAGS_NONE },
    { "getMediaDbPermission", IndexerServiceMock::onMediaDbPermissionGet, LUNA_METHOD_FLAGS_NONE },
    {NULL, NULL}
};

IndexerServiceMock::IndexerServiceMock(std::string serviceName, GMainLoop *loop)
    : serviceName_(serviceName)
    , mainLoop_(loop)
{
    std::cout << std::string("IndexerServiceMock ctor!") << std::endl;
    LSError lsError;
    LSErrorInit(&lsError);

    if (!LSRegister(serviceName_.c_str(), &lsHandle_, &lsError)) {
        std::cout << std::string("Unable to register at luna-bus") << std::endl;
        return;
    }
    if (!LSRegisterCategory(lsHandle_, "/", serviceMethods_, NULL, NULL,
            &lsError)) {
        std::cout << std::string("Unable to register top level category") << std::endl;
        return;
    }
    if (!LSCategorySetData(lsHandle_, "/", this, &lsError)) {
        std::cout << std::string("Unable to set data on top level category") << std::endl;
        return;
    }
    if (!LSGmainAttach(lsHandle_, mainLoop_, &lsError)) {
        std::cout << std::string("Unable to attach service") << std::endl;
        return;
    }
}

IndexerServiceMock::~IndexerServiceMock()
{
    if (!lsHandle_)
        return;

    LSError lsError;
    LSErrorInit(&lsError);
    if (!LSUnregister(lsHandle_, &lsError)) {
        std::cout << std::string("Service unregister failed") << std::endl;
    }
}

bool IndexerServiceMock::onDeviceListGet(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    std::cout << std::string("onDeviceListGet!") << std::endl;

    // parse incoming message
    const char *payload = LSMessageGetPayload(msg);
    std::string method = LSMessageGetMethod(msg);
    pbnjson::JDomParser parser;

    // check request whether it's json or not.
    if (!parser.parse(payload, pbnjson::JSchema::AllSchema())) {
        std::cout << std::string("Invalid request: ") << method << std::string(" payload: ") << payload << std::endl;
        return false;
    }

    auto domTree(parser.getDom());

    auto reply = pbnjson::Object();
    reply.put("subscribed", LSMessageIsSubscription(msg));
    reply.put("returnValue", true);

    LSError lsError;
    LSErrorInit(&lsError);

    // initial response
    if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
        std::cout << std::string("Message reply error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        return false;
    }

    // process subscription
    if (LSMessageIsSubscription(msg)) {
        LSSubscriptionAdd(lsHandle, "getDeviceList", msg, nullptr);
        IndexerServiceMock *mock = static_cast<IndexerServiceMock*>(ctx);
        mock->queue_.push(std::string("getDeviceList"));
        std::uint32_t val = 0;
        std::ifstream fs("/dev/urandom", std::ios::in|std::ios::binary);
        if (fs)
        {
            fs.read(reinterpret_cast<char*>(&val), sizeof(val));
        }
        fs.close();
        int sec = (val % 5) * 1000; // get the 1~5 sec from rand func
        g_timeout_add(sec, &IndexerServiceMock::sendResponse, mock);
    }

    return true;
}

bool IndexerServiceMock::onGetAudioList(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    std::cout << std::string("onGetAudioList!") << std::endl;

    // parse incoming message
    const char *payload = LSMessageGetPayload(msg);
    std::string method = LSMessageGetMethod(msg);
    pbnjson::JDomParser parser;

    // check request whether it's json or not.
    if (!parser.parse(payload, pbnjson::JSchema::AllSchema())) {
        std::cout << std::string("Invalid request: ") << method << std::string(" payload: ") << payload << std::endl;
        return false;
    }

    auto domTree(parser.getDom());

    auto reply = pbnjson::Object();
    reply.put("subscribed", LSMessageIsSubscription(msg));
    reply.put("returnValue", true);

    LSError lsError;
    LSErrorInit(&lsError);

    // initial response
    if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
        std::cout << std::string("Message reply error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        return false;
    }

    // process subscription
    if (LSMessageIsSubscription(msg)) {
        LSSubscriptionAdd(lsHandle, "getAudioList", msg, nullptr);
        IndexerServiceMock *mock = static_cast<IndexerServiceMock*>(ctx);
        mock->queue_.push(std::string("getAudioList"));
        std::uint32_t val = 0;
        std::ifstream fs("/dev/urandom", std::ios::in|std::ios::binary);
        if (fs)
        {
            fs.read(reinterpret_cast<char*>(&val), sizeof(val));
        }
        fs.close();
        int sec = (val % 5) * 1000; // get the 1~5 sec from rand func
        g_timeout_add(sec, &IndexerServiceMock::sendResponse, mock);
    }

    return true;
}

bool IndexerServiceMock::onGetVideoList(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    std::cout << std::string("onGetVideoList!") << std::endl;

    // parse incoming message
    const char *payload = LSMessageGetPayload(msg);
    std::string method = LSMessageGetMethod(msg);
    pbnjson::JDomParser parser;

    // check request whether it's json or not.
    if (!parser.parse(payload, pbnjson::JSchema::AllSchema())) {
        std::cout << std::string("Invalid request: ") << method << std::string(" payload: ") << payload << std::endl;
        return false;
    }

    auto domTree(parser.getDom());

    auto reply = pbnjson::Object();
    reply.put("subscribed", LSMessageIsSubscription(msg));
    reply.put("returnValue", true);

    LSError lsError;
    LSErrorInit(&lsError);

    // initial response
    if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
        std::cout << std::string("Message reply error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        return false;
    }

    // process subscription
    if (LSMessageIsSubscription(msg)) {
        LSSubscriptionAdd(lsHandle, "getVideoList", msg, nullptr);
        IndexerServiceMock *mock = static_cast<IndexerServiceMock*>(ctx);
        mock->queue_.push(std::string("getVideoList"));
        std::uint32_t val = 0;
        std::ifstream fs("/dev/urandom", std::ios::in|std::ios::binary);
        if (fs)
        {
            fs.read(reinterpret_cast<char*>(&val), sizeof(val));
        }
        fs.close();
        int sec = (val % 5) * 1000; // get the 1~5 sec from rand func
        g_timeout_add(sec, &IndexerServiceMock::sendResponse, mock);
    }

    return true;
}

bool IndexerServiceMock::onGetImageList(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    std::cout << std::string("onGetImageList!") << std::endl;

    // parse incoming message
    const char *payload = LSMessageGetPayload(msg);
    std::string method = LSMessageGetMethod(msg);
    pbnjson::JDomParser parser;

    // check request whether it's json or not.
    if (!parser.parse(payload, pbnjson::JSchema::AllSchema())) {
        std::cout << std::string("Invalid request: ") << method << std::string(" payload: ") << payload << std::endl;
        return false;
    }

    auto domTree(parser.getDom());

    auto reply = pbnjson::Object();
    reply.put("subscribed", LSMessageIsSubscription(msg));
    reply.put("returnValue", true);

    LSError lsError;
    LSErrorInit(&lsError);

    // initial response
    if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
        std::cout << std::string("Message reply error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        return false;
    }

    // process subscription
    if (LSMessageIsSubscription(msg)) {
        LSSubscriptionAdd(lsHandle, "getImageList", msg, nullptr);
        IndexerServiceMock *mock = static_cast<IndexerServiceMock*>(ctx);
        mock->queue_.push(std::string("getImageList"));
        std::uint32_t val = 0;
        std::ifstream fs("/dev/urandom", std::ios::in|std::ios::binary);
        if (fs)
        {
            fs.read(reinterpret_cast<char*>(&val), sizeof(val));
        }
        fs.close();
        int sec = (val % 5) * 1000; // get the 1~5 sec from rand func
        g_timeout_add(sec, &IndexerServiceMock::sendResponse, mock);

    }

    return true;
}

bool IndexerServiceMock::onGetAudioMetaData(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    std::cout << std::string("onGetAudioMetaData!") << std::endl;

    // parse incoming message
    const char *payload = LSMessageGetPayload(msg);
    std::string method = LSMessageGetMethod(msg);
    pbnjson::JDomParser parser;

    // check request whether it's json or not.
    if (!parser.parse(payload, pbnjson::JSchema::AllSchema())) {
        std::cout << std::string("Invalid request: ") << method << std::string(" payload: ") << payload << std::endl;
        return false;
    }

    auto domTree(parser.getDom());

    auto reply = pbnjson::Object();
    reply.put("subscribed", LSMessageIsSubscription(msg));
    reply.put("returnValue", true);

    LSError lsError;
    LSErrorInit(&lsError);

    // initial response
    if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
        std::cout << std::string("Message reply error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        return false;
    }

    // process subscription
    if (LSMessageIsSubscription(msg)) {
        LSSubscriptionAdd(lsHandle, "getAudioMetaData", msg, nullptr);
        IndexerServiceMock *mock = static_cast<IndexerServiceMock*>(ctx);
        mock->queue_.push(std::string("getAudioMetaData"));
        std::uint32_t val = 0;
        std::ifstream fs("/dev/urandom", std::ios::in|std::ios::binary);
        if (fs)
        {
            fs.read(reinterpret_cast<char*>(&val), sizeof(val));
        }
        fs.close();
        int sec = (val % 5) * 1000; // get the 1~5 sec from rand func
        g_timeout_add(sec, &IndexerServiceMock::sendResponse, mock);
    }

    return true;
}

bool IndexerServiceMock::onGetVideoMetaData(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    std::cout << std::string("onGetVideoMetaData!") << std::endl;

    // parse incoming message
    const char *payload = LSMessageGetPayload(msg);
    std::string method = LSMessageGetMethod(msg);
    pbnjson::JDomParser parser;

    // check request whether it's json or not.
    if (!parser.parse(payload, pbnjson::JSchema::AllSchema())) {
        std::cout << std::string("Invalid request: ") << method << std::string(" payload: ") << payload << std::endl;
        return false;
    }

    auto domTree(parser.getDom());

    auto reply = pbnjson::Object();
    reply.put("subscribed", LSMessageIsSubscription(msg));
    reply.put("returnValue", true);

    LSError lsError;
    LSErrorInit(&lsError);

    // initial response
    if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
        std::cout << std::string("Message reply error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        return false;
    }

    // process subscription
    if (LSMessageIsSubscription(msg)) {
        LSSubscriptionAdd(lsHandle, "getVideoMetaData", msg, nullptr);
        IndexerServiceMock *mock = static_cast<IndexerServiceMock*>(ctx);
        mock->queue_.push(std::string("getVideoMetaData"));
        std::uint32_t val = 0;
        std::ifstream fs("/dev/urandom", std::ios::in|std::ios::binary);
        if (fs)
        {
            fs.read(reinterpret_cast<char*>(&val), sizeof(val));
        }
        fs.close();
        int sec = (val % 5) * 1000; // get the 1~5 sec from rand func
        g_timeout_add(sec, &IndexerServiceMock::sendResponse, mock);
    }

    return true;
}

bool IndexerServiceMock::onGetImageMetaData(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    std::cout << std::string("onGetImageMetaData!") << std::endl;

    // parse incoming message
    const char *payload = LSMessageGetPayload(msg);
    std::string method = LSMessageGetMethod(msg);
    pbnjson::JDomParser parser;

    // check request whether it's json or not.
    if (!parser.parse(payload, pbnjson::JSchema::AllSchema())) {
        std::cout << std::string("Invalid request: ") << method << std::string(" payload: ") << payload << std::endl;
        return false;
    }

    auto domTree(parser.getDom());

    auto reply = pbnjson::Object();
    reply.put("subscribed", LSMessageIsSubscription(msg));
    reply.put("returnValue", true);

    LSError lsError;
    LSErrorInit(&lsError);

    // initial response
    if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
        std::cout << std::string("Message reply error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        return false;
    }

    // process subscription
    if (LSMessageIsSubscription(msg)) {
        LSSubscriptionAdd(lsHandle, "getImageMetaData", msg, nullptr);
        IndexerServiceMock *mock = static_cast<IndexerServiceMock*>(ctx);
        mock->queue_.push(std::string("getImageMetaData"));
        std::uint32_t val = 0;
        std::ifstream fs("/dev/urandom", std::ios::in|std::ios::binary);
        if (fs)
        {
            fs.read(reinterpret_cast<char*>(&val), sizeof(val));
        }
        fs.close();
        int sec = (val % 5) * 1000; // get the 1~5 sec from rand func
        g_timeout_add(sec, &IndexerServiceMock::sendResponse, mock);
    }

    return true;
}

bool IndexerServiceMock::onMediaDbPermissionGet(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    std::cout << std::string("onMediaDbPermissionGet!") << std::endl;

    // parse incoming message
    const char *payload = LSMessageGetPayload(msg);
    std::string method = LSMessageGetMethod(msg);
    pbnjson::JDomParser parser;

    // check request whether it's json or not.
    if (!parser.parse(payload, pbnjson::JSchema::AllSchema())) {
        std::cout << std::string("Invalid request: ") << method << std::string(" payload: ") << payload << std::endl;
        return false;
    }

    auto domTree(parser.getDom());

    auto reply = pbnjson::Object();
    reply.put("subscribed", LSMessageIsSubscription(msg));
    reply.put("returnValue", true);

    LSError lsError;
    LSErrorInit(&lsError);

    // initial response
    if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
        std::cout << std::string("Message reply error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        return false;
    }

    // process subscription
    if (LSMessageIsSubscription(msg)) {
        LSSubscriptionAdd(lsHandle, "getMediaDbPermission", msg, nullptr);
        IndexerServiceMock *mock = static_cast<IndexerServiceMock*>(ctx);
        mock->queue_.push(std::string("getMediaDbPermission"));
        std::uint32_t val = 0;
        std::ifstream fs("/dev/urandom", std::ios::in|std::ios::binary);
        if (fs)
        {
            fs.read(reinterpret_cast<char*>(&val), sizeof(val));
        }
        fs.close();
        int sec = (val % 5) * 1000; // get the 1~5 sec from rand func
        g_timeout_add(sec, &IndexerServiceMock::sendResponse, mock);
    }

    return true;
}

void IndexerServiceMock::run()
{
    if (mainLoop_)
        g_main_loop_run(mainLoop_);
}

int IndexerServiceMock::sendResponse(void *ctx)
{
    std::cout << std::string("sendResponse!") << std::endl;
    IndexerServiceMock *mock = static_cast<IndexerServiceMock*>(ctx);
    LSError lsError;
    LSErrorInit(&lsError);
    std::string method = mock->queue_.front();
    mock->queue_.pop();
    auto reply = pbnjson::Object();
    reply.put("testString", method.c_str());
    reply.put("returnValue", true);
    if (!LSSubscriptionReply(mock->lsHandle_, method.c_str(), reply.stringify().c_str(), &lsError)) {
//    if (!LSSubscriptionReply(mock->lsHandle_, "getDeviceList", reply.stringify().c_str(), &lsError)) {
        std::cout << std::string("subscription reply error!") << std::endl;
        LSErrorPrint(&lsError, stdout);
        LSErrorFree(&lsError);
        return 0;
    }
    LSErrorFree(&lsError);
    return 0; // reply only one
}

int main(int argc, char* argv[])
{
    std::cout << std::string("IndexerService main start!") << std::endl;
    GMainLoop *loop = g_main_loop_new(NULL, false);

    IndexerServiceMock service(std::string("com.webos.service.mediaindexerMock"), loop);
    service.run();
    g_main_loop_unref(loop);

    std::cout << std::string("IndexerService Exit!") << std::endl;
    return 0;
}
