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
 * File:   JavaReader.cpp
 * Author: William Wickerson
 * 
 * Created on December 3, 2017, 4:01 PM
 */

#include <algorithm>
#include "JavaReader.h"
#include "Utility.h"

#include <iostream>

JavaReader::JavaReader(const std::string& filePath) : 
        fileStream(std::ifstream(filePath)),
        functions(std::map<std::string, std::vector<std::pair<int,int>>>()) {
    //Keep track of file position and nesting depth
    int blockDepth = 0;
    int lineNumber = 0;
    //Read until we find the start of the class definition
    std::string line;
    while (getline(this->fileStream,line)) {
        lineNumber++;
        //Find the start of the class
        if (Util::regexFind(line, "class [A-Z]\\w+") != std::string::npos) {
            //Find the position of the class name on the line
            int classStart = Util::regexFind(line, "class [A-Z]\\w+");
            int nameStart = line.find(" ", classStart) + 1;
            int nameEnd = line.find(" ", nameStart);
            //Store the class name in this->className
            this->className = line.substr(nameStart, nameEnd - nameStart);
            //Start reading after class block starts
            while (line.find("{") == std::string::npos) {
                lineNumber++;
                getline(this->fileStream,line);
            }
            break;
        }
    }
    //Keep track of current function and where it started
    int functionStart;
    int internalClassDepth = 0;
    std::string currentFunctionName;
    while (getline(this->fileStream,line)) {
        lineNumber++;
        if (blockDepth == internalClassDepth) {
            //Search for function definitions using regex
            //"(public|private|protected )", functions should start with a modifier
            //"[^=\\.]+" covers any return type and modifiers, and avoids variables
            //" \\w+\\(" should capture the function name, with the '(' being the tell
            if (Util::regexFind(line, "(public|private|protected)?[^=\\.]* \\w+\\(")
                    != std::string::npos) {
                functionStart = lineNumber;
                int nameEnd = line.find("(");
                int nameStart = line.rfind(" ", nameEnd) + 1;
                currentFunctionName = line.substr(nameStart, nameEnd - nameStart);
            }
        }
        int nextBlockDepth = blockDepth;
        //Get the new block depth after the current line
        std::string trimmed = Util::trim(line);
        if (!Util::startsWith(trimmed, "*") && !Util::startsWith(trimmed, "//"))
            nextBlockDepth = blockDepth + std::count(line.begin(), line.end(), '{')
                    - std::count(line.begin(), line.end(), '}');
        //If a function ended, since the block depth decreased
        if ((blockDepth > internalClassDepth) && (nextBlockDepth <= internalClassDepth)) {
            if (currentFunctionName.compare("")) {
                //Create a new std::vector<std:pair<int,int>> just in case
                std::vector<std::pair<int,int>> update;
                if (this->functions.count(currentFunctionName))
                    update = this->functions[currentFunctionName];
                //Add this function to the map with its and end
                update.push_back(std::pair<int,int>(functionStart, lineNumber));
                this->functions[currentFunctionName] = update;
                currentFunctionName = "";
            }
        }
        //If a new internal class opened up, increase the depth
        if (Util::regexFind(line, "class [A-Z]\\w+") != std::string::npos)
            internalClassDepth++;
        //If we have exceeded the bounds of the class, decrease depth
        if (nextBlockDepth == internalClassDepth - 1)
            internalClassDepth--;
        blockDepth = nextBlockDepth;
    }
};
    
std::string JavaReader::readLine(int lineNumber) {
    return this->readLines(std::pair<int,int>(lineNumber,lineNumber));
}

std::string JavaReader::readLines(std::pair<int,int> bounds) {
    //Get the start and end of the function from the map
    int boundsStart = bounds.first;
    int boundsEnd = bounds.second;
    //Reset the stream to the beginning of the file
    this->fileStream.clear();
    this->fileStream.seekg(0, fileStream.beg);
    //Keep track of the line number and function body
    int currentLineNumber = 0;
    std::string line;
    std::string functionBody;
    while(getline(this->fileStream,line)) {
        currentLineNumber++;
        //If the current line is within bounds of function, add it to function body
        if ((currentLineNumber >= boundsStart) && (currentLineNumber <= boundsEnd))
            functionBody += Util::trim(line) + "\n";
    }
    return functionBody;
}

std::string JavaReader::readFunction(int lineNumber) {
    //Find the function name at the given line number
    std::string functionName = readFunctionName(lineNumber);
    //Return the body of the given function name
    return this->readFunction(functionName);
}
    
std::string JavaReader::readFunction(const std::string& functionName) {
    //If function name is not in functions, return empty string
    if (!this->functions.count(functionName))
        return std::string();
    //Read through the functions for a .exec( and return that
    for (const std::pair<int,int>& p : this->functions[functionName]) {
        if (this->readLines(p).find(".exec(") != std::string::npos)
            return this->readLines(p);
    }
    //If not just return the first option
    return this->readLines(this->functions[functionName][0]);
}

std::string JavaReader::readFunctionName(int lineNumber) {
    //Find the function lineNumber is contained in
    for (auto const& v : this->functions) {
        //v.first is the function name
        //v.second is the vector of pairs (start,end)
        for (auto const& p : v.second)
            //p.first is the first in the pair
            //p.second is the second in the pair
            if ((lineNumber >= p.first) && (lineNumber <= p.second))
                return v.first;
    }
    //Otherwise return empty string if there is none
    return std::string();
}

std::string JavaReader::getClassName() {
    return this->className;
}

std::vector<std::string> JavaReader::readFunctionNames() {
    std::vector<std::string> functionNames;
    for (auto const& x : this->functions) {
        //x.first is function name
        //x.second is int pair boundary
        functionNames.push_back(x.first);
    }
    return functionNames;
}

std::pair<int,int> JavaReader::getFunctionBounds(int lineNumber) {
    //Find the function name at the given line number
    std::string functionName = readFunctionName(lineNumber);
    //Return the function bounds
    return this->getFunctionBounds(functionName);
}

std::pair<int,int> JavaReader::getFunctionBounds(const std::string& functionName) {
    //If function name is not in functions, return -1
    if (!this->functions.count(functionName))
        return std::pair<int,int>(-1,-1);
    //Read through the functions for a .exec( and return that
    for (const std::pair<int,int>& p : this->functions[functionName]) {
        if (this->readLines(p).find(".exec(") != std::string::npos)
            return p;
    }
    //If not just return the first option
    return this->functions[functionName][0];
}
