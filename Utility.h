/*
 * The MIT License
 *
 * Copyright 2017 william.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* 
 * File:   Utility.h
 * Author: william
 *
 * Created on December 4, 2017, 12:40 AM
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <vector>

namespace Util {

    std::string trim(const std::string& s);

    std::vector<std::string> split(const std::string& s, const std::string& delimiter);
    
    bool startsWith(const std::string& s, const std::string& substring);
    
    bool endsWith(const std::string& s, const std::string& substring);
    
    size_t regexFind(const std::string& s, const std::string& regex, int start = 0);
    
    std::string escapeRegex(const std::string& s);
    
    int getDepth(const std::string& s, int position);
    
    std::string replaceAtDepth(const std::string& s, const std::string& replacer,
            const std::string& replacee);
    
    std::vector<std::string> splitNotAtDepth(const std::string& s, const std::string& delimiter);
    
} //namespace Util

#endif /* UTILITY_H */
