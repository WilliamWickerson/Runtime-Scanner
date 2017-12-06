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
 * File:   GrepParser.cpp
 * Author: William Wickerson
 * 
 * Created on December 3, 2017, 1:36 PM
 */

#include "GrepParser.h"

GrepParser::GrepParser() : input(&std::cin) {};

GrepParser::GrepParser(const std::string& filePath) :
    input(new std::ifstream(filePath)) {};
    
GrepParser::~GrepParser() {
    if (this->input != &std::cin)
        delete this->input;
}

//Sets the istream given a file path, deletes previous one if not std::cin
void GrepParser::setInput(const std::string& filePath) {
    if (this->input != &std::cin)
        delete this->input;
    this->input = new std::ifstream(filePath);
}

//Return a map between all file paths and the line numbers specified by grep
std::map<std::string, std::vector<int>> GrepParser::parseInput() {
    std::map<std::string, std::vector<int>> fileLines;
    std::string line;
    //Read through until the eof
    while(getline(*this->input, line)) {
        //Format is [file path]:[line number]:[line content] so split on ":"
        size_t firstColon = line.find(":");
        size_t secondColon = line.find(":", firstColon + 1);
        std::string filePath = line.substr(0, firstColon);
        //Remove "./" from beginning of file name if it exists there
        if (!filePath.substr(0,2).compare("./"))
            filePath = filePath.substr(2);
        //Parse the number and convert it to an integer
        std::string lineString = line.substr(firstColon+1, secondColon-firstColon);
        int lineNumber = stoi(lineString);
        //If file path is not in the map yet, create a new vector it
        if (!fileLines.count(filePath)) {
            std::vector<int> newFile;
            newFile.push_back(lineNumber);
            fileLines[filePath] = newFile;
        }
        //If it is in the map, update and push back the updated vector
        else {
            std::vector<int>& oldFile = fileLines[filePath];
            oldFile.push_back(lineNumber);
        }
    }
    return fileLines;
}
