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
 * File:   JavaParser.h
 * Author: William Wickerson
 *
 * Created on December 3, 2017, 9:01 PM
 */

#ifndef JAVAPARSER_H
#define JAVAPARSER_H

#include <string>
#include <vector>
#include "JavaReader.h"

class JavaParser {
public:
    JavaParser(const std::string& filePath);
    std::string getFunctionName(int lineNumber);
    std::string getFullStatement(int lineNumber);
    std::string findType(const std::string& variableName, const std::string& functionName);
    std::string findMemberType(const std::string& variableName);
    bool isInput(const std::string& variableName, const std::string& functionName);
    bool isHardcoded(const std::string& variableName, const std::string& functionName);
    bool isCommented(int lineNumber);
    std::string getExpression(const std::string& functionName, int lineNumber);
    std::string getStringArr(const std::string& stringArrName, const std::string& functionName);
    std::vector<std::string> parseExpression(const std::string& functionName, int lineNumber);
    std::vector<std::string> parseExpression(const std::string& expression);
    std::vector<std::string> parseStringArr(const std::string& stringArr);
    std::vector<std::string> parseFunction(const std::string& function);
    std::vector<std::string> parseRecursively(const std::string& functionName, int lineNumber);
private:
    JavaReader javaReader;
    std::vector<std::string> parseRecursively(const std::vector<std::string>& parts, const std::string& functionName);
};

#endif /* JAVAPARSER_H */
