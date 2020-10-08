// Copyright (c) 2020 LG Electronics, Inc.
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

#include "connector.h"
#include "GenerateUniqueID.h"

Connector::Connector(std::string&& serviceName)
    : serviceName_(serviceName)
{
    serviceName_ += mediaindexerclient::GenerateUniqueID()();
    connector_ = std::unique_ptr<LunaConnector>(new LunaConnector(serviceName_, true));
}

Connector::~Connector()
{
    connector_.reset();
}

std::string Connector::getServiceName() const
{
    return serviceName_;
}

#define INDENT_INCREMENT 4
void Connector::pretty_print(jvalue_ref object, int first_indent, int indent)
{
    if (!object) {
        printf("%*s<NULL>", first_indent, "");
    return;
    }

    if (!jis_array(object) && !jis_object(object))
    {
        printf("%*s%s", first_indent, "", jvalue_tostring_simple(object));
    }
    else if (jis_array(object))
    {
        int len = jarray_size(object);
        int i;
        printf("%*s[", first_indent, "");
        bool first = true;
        for (i=0;i<len;i++) {
            if (first) {
                printf("\n");
                first = false;
            } else {
                printf(",\n");
            }
            pretty_print(jarray_get(object, i), indent + INDENT_INCREMENT, indent + INDENT_INCREMENT);
        }
        printf("\n%*s]", indent, "");
    }
    else if (jis_object(object))
    {
        printf("%*s{", first_indent, "");
        bool first = true;
        jobject_iter it;
        (void)jobject_iter_init(&it, object);/* TODO: handle appropriately */
        jobject_key_value keyval;
        while (jobject_iter_next(&it, &keyval)) {
            if (first) {
                printf("\n");
                first = false;
            } else {
                printf(",\n");
            }
            // FIXME: contents of key are not being escaped
            raw_buffer key = jstring_get_fast(keyval.key);
            printf("%*s\"%.*s\": ", indent+INDENT_INCREMENT, "", (int)key.m_len, key.m_str);
            pretty_print(keyval.value, 0, indent + INDENT_INCREMENT);
        }
        printf("\n%*s}\n", indent, "");
    }
    else
    {
        printf("%*s<unknown json type>", first_indent, "");
    }
}
