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
 * File:   Utility.cpp
 * Author: william
 * 
 * Created on December 4, 2017, 12:40 AM
 */

#include "Utility.h"
#include <regex>

#include <iostream>

namespace Util {

std::string trim(const std::string& s) {
    //Find the end of white space
    int subStart = s.find_first_not_of(" \t");
    int subEnd = s.find_last_not_of(" \t") + 1;
    //If there is no end to the white space return empty string
    if (subStart == std::string::npos)
        return std::string();
    //Otherwise return the substring
    else
        return s.substr(subStart, subEnd - subStart);
}

std::vector<std::string> split(const std::string& s, const std::string& delimiter) {
    //Create a vector for the strings and copy s
    std::vector<std::string> strings;
    std::string copyS = s;
    //While there are still delimiters left, split copyS and add it to strings
    int next;
    while ((next = copyS.find(delimiter)) != std::string::npos) {
        strings.push_back(copyS.substr(0,next));
        copyS = copyS.substr(next + delimiter.length());
    }
    //Add the rest of the string before returning
    strings.push_back(copyS);
    return strings;
}

bool startsWith(const std::string& s, const std::string& substring) {
    return (s.rfind(substring,0) == 0);
}
    
bool endsWith(const std::string& s, const std::string& substring) {
    int endStart = s.length() - substring.length();
    return (s.find(substring, endStart) == endStart);
}

size_t regexFind(const std::string& s, const std::string& regex, int start) {
    //Create regex and match variables
    std::smatch match;
    std::regex rgx;
    //Bad regexes through std::regex_error so add a try catch
    try {
        rgx.assign(regex);
    } catch (std::regex_error e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << regex << std::endl;
        return std::string::npos;
    }
    //std::regex_search returns the first match in match[0], true if successful
    if (std::regex_search(s.begin() + start, s.end(), match, rgx)) {
        return s.find(match[0], start);
    }
    //The pattern was not found so return std::string::npos
    else {
        return std::string::npos;
    }
}

std::string escapeRegex(const std::string& s) {
    //Copy s and use the list of special characters which would show
    //up in variable and function names to remove them from copyS
    std::string copyS = s;
    std::string specialChars = "\\[].^$+*(){}&";
    //For each char, turn it into a string, and replace it with "\char"
    for (char c : specialChars) {
        std::string ch = std::string(1,c);
        copyS = regex_replace(copyS, std::regex("\\" + ch), "\\" + ch);
    }
    return copyS;
}

int getDepth(const std::string& s, int position) {
    //open increases depth, close decreases depth, both flip flops
    std::string open = "({";
    std::string close = ")}";
    std::string both = "\"'";
    //Keep track of depth and the last flip flop of both
    int last[2] = {-1, -1};
    int depth = 0;
    //Iterate through the string so we can replace chars
    for (int i = 0; i < s.length(); i++) {
        //If the current character is an open char, increase depth
        if (open.find(s[i]) != std::string::npos)
            depth++;
        //If the current character is a close char, decrease depth
        else if (close.find(s[i]) != std::string::npos)
            depth--;
        //If the current character is a flip flop, clip it and add it to depth
        else if (both.find(s[i]) != std::string::npos) {
            int pos = both.find(s[i]);
            last[pos] = last[pos] * -1;
            depth += last[pos];
        }
        //If we reached the position in question, return the depth
        if (i == position)
            return depth;
    }
    return 0;
}

std::string replaceAtDepth(const std::string& s, const std::string& replacee,
        const std::string& replacer) {
    //Copy s to keep it const
    std::string copyS = s;
    //open increases depth, close decreases depth, both flip flops
    std::string open = "({";
    std::string close = ")}";
    std::string both = "\"'";
    //Keep track of depth and the last flip flop of both
    int last[2] = {-1, -1};
    int depth = 0;
    //Iterate through the string so we can replace chars
    for (int i = 0; i < copyS.length(); i++) {
        //If the current character is an open char, increase depth
        if (open.find(copyS[i]) != std::string::npos)
            depth++;
        //If the current character is a close char, decrease depth
        else if (close.find(copyS[i]) != std::string::npos)
            depth--;
        //If the current character is a flip flop, clip it and add it to depth
        else if (both.find(copyS[i]) != std::string::npos) {
            int pos = both.find(copyS[i]);
            last[pos] = last[pos] * -1;
            depth += last[pos];
        }
        //If the replacee is at char i and the depth is > 0
        if (Util::startsWith(copyS.substr(i), replacee) && (depth > 0))
            //Replace it with the replacer
            copyS.replace(i, replacee.length(), replacer);
    }
    return copyS;
}

//Using splitNotAtDepth prevents "x,(a,b,c),z" from becoming
//{x, (a, b, c), z} instead of {x, (a,b,c), z}
std::vector<std::string> splitNotAtDepth(const std::string& s, const std::string& delimiter) {
    //Switch characters at depth with something that's not the delimiter
    std::string copyS = replaceAtDepth(s, delimiter, "\n");
    //Now we're splitting not at depths so we can just split
    std::vector<std::string> splits = split(copyS, delimiter);
    //Need to go ahead and put the old delimiter back before returning
    for (std::string& string : splits)
        string = std::regex_replace(string, std::regex("\n"), delimiter);
    return splits;
}

} //namespace Util
