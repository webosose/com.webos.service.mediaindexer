// Copyright (c) 2018-2020 LG Electronics, Inc.
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

/// Pure static libedit wrapper.
class Shell
{
 public:
    /**
     * \brief Run the shell.
     *
     * \param[in] argc From main().
     * \param[in] argv From main().
     */
    static void run(int  argc, char *argv[]);

    virtual ~Shell() {};

 private:
    /// Pure static.
    Shell() {};

    /// Strip leading/trailing whitespaces from string.
    static char *stripwhite (char *str);
    /// Print usage message.
    static void printHelp(void);
    /// Run plugin get().
    static void getPlugin(const char *uri);
    /// Run plugin put().
    static void putPlugin(const char *uri);
    /// Start general device detection.
    static void runDetect(void);
    /// Stop general device detection.
    static void stopDetect(void);
    /// Resolve media item uri to playback uri.
    static void getPlaybackUri(const char *uri);
    /// Evaluate entered command.
    static int executeLine(char *exp);
    /// Parse command and parameter.
    static void parseLine(const char *exp, char **cmd, char **arg);
};
