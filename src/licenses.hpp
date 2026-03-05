#pragma once
#include <string>
#include <unordered_map>

namespace Licenses {

    inline std::string getText(const std::string& type,
                               const std::string& author,
                               const std::string& year) {
        if (type == "mit") {
            return "MIT License\n\nCopyright (c) " + year + " " + author + R"(

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
)";
        }
        if (type == "apache2") {
            return "Apache License\nVersion 2.0, January 2004\n\nCopyright (c) " + year + " " + author + R"(

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
)";
        }
        if (type == "gpl3") {
            return "GNU GENERAL PUBLIC LICENSE\nVersion 3, 29 June 2007\n\nCopyright (c) " + year + " " + author + R"(

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
)";
        }
        return ""; // "none"
    }

    inline std::string headerComment(const std::string& type,
                                     const std::string& author,
                                     const std::string& year) {
        if (type == "none") return "";
        std::string spdx;
        if (type == "mit")     spdx = "MIT";
        else if (type == "apache2") spdx = "Apache-2.0";
        else if (type == "gpl3")    spdx = "GPL-3.0-or-later";
        return "// Copyright (c) " + year + " " + author + "\n"
             + "// SPDX-License-Identifier: " + spdx + "\n";
    }
}
