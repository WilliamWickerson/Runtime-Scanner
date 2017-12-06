/*
 * The MIT License
 *
 * Copyright 2017 William Wickerson.
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
 * File:   JavaReader.h
 * Author: William Wickerson
 *
 * Created on December 3, 2017, 4:01 PM
 */

#ifndef JAVAREADER_H
#define JAVAREADER_H

#include <fstream>
#include <map>

class JavaReader {
public:
    JavaReader(const std::string& filePath);
    std::string readLine(int lineNumber);
    std::string readLines(std::pair<int,int> bounds);
    std::string readFunction(int lineNumber);
    std::string readFunction(const std::string& functionName);
    std::string readFunctionName(int lineNumber);
    std::string getClassName();
    std::vector<std::string> readFunctionNames();
    std::pair<int,int> getFunctionBounds(int lineNumber);
    std::pair<int,int> getFunctionBounds(const std::string& functionName);
private:
    std::ifstream fileStream;
    std::string className;
    std::map<std::string, std::vector<std::pair<int,int>>> functions;
};

#endif /* JAVAREADER_H */
