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
 * File:   main.cpp
 * Author: William Wickerson
 *
 * Created on December 3, 2017, 1:36 PM
 */

#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <iomanip>

#include "GrepParser.h"
#include "JavaParser.h"
#include "Utility.h"

int main(int argc, char** argv) {
    //Set default file path as current directory
    std::string projectPath = std::string("./");
    std::string grepPath;
    //Booleans for printing results
    bool printHardcode = false;
    bool printInput = false;
    bool printOther = false;
    //Look through command line arguments
    for (int i = 1; i < argc; i++) {
        //-p [project path] gets the search path
        if (!strcmp(argv[i], "-p")) {
            projectPath = std::string(argv[i+1]);
        }
        //-g [grep file path] gets the grep file if not piped in
        if (!strcmp(argv[i], "-g")) {
            grepPath = std::string(argv[i+1]);
        }
        //-h shows hardcoded uses
        if (!strcmp(argv[i], "-h")) {
            printHardcode = true;
        }
        //-i shows input uses
        if (!strcmp(argv[i], "-i")) {
            printInput = true;
        }
        //-o shows other uses
        if (!strcmp(argv[i], "-o")) {
            printOther = true;
        }
        //--help and -? show how to use runtime_scanner
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-?")) {
            std::cout << "\t-p [project path] | root folder of grep search" << std::endl;
            std::cout << "\t-g [grep file path] | output grep file (if not directed in as a pipe)" << std::endl;
            std::cout << "\t-h | display hardcoded uses" << std::endl;
            std::cout << "\t-i | display uses with function input" << std::endl;
            std::cout << "\t-o | display other uses" << std::endl;
        }
    }
    
    //Open a grep parser for the grep file
    GrepParser grepParser;
    //If a grep file path was specified, use it
    if (!grepPath.empty()) {
        grepParser.setInput(grepPath);
    }
    //If no grep file and no piped input, output error and exit
    else if (isatty(fileno(stdin))) {
        std::cerr << "Error: no grep input given, exiting\n";
        return 1;
    }
    
    //Get the map between file paths and sets of target lines
    std::map<std::string, std::vector<int>> fileLines = grepParser.parseInput();
    //Create some file counting integers for display at the end
    int runtimeFileCount = 0;
    int currentFileCount = 0;
    int totalFileCount = fileLines.size();
    int testFileCount = 0;
    //Create some function counting integers for display at the end
    int runtimeFunctionCount = 0;
    int inputFunctionCount = 0;
    int hardcodedFunctionCount = 0;
    int totalFunctionCount = 0;
    //Create maps to keep track of the input parameter types
    std::map<std::string,int> typeCounts;
    std::map<std::string,int> typeHardcoded;
    std::map<std::string,int> typeInput;
    //Create vectors to keep track of usage types
    std::vector<std::string> hardcodedUses;
    std::vector<std::string> inputUses;
    std::vector<std::string> otherUses;
    
    //JavaReader jr(projectPath + "/" + "frameworks/testing/uiautomator_test_libraries/src/com/android/uiautomator/platform/SurfaceFlingerHelper.java");    
    //for (std::string s : jr.readFunctionNames())
    //    std::cout << s << std::endl;
    
    //return 0;
    
    //Iterate through all of the pairs in the grep map
    for (auto const& file : fileLines) {
        //file.first is the file path
        //file.second is the vector of line numbers
        //Increment the file count, add number of lines, and print progress
        currentFileCount += 1;
        totalFunctionCount += file.second.size();
        std::cout << "\rCurrent Progress: File " << currentFileCount << "/" << totalFileCount << std::flush;
        //Open up a new JavaParser for the current file
        JavaParser jp(projectPath + "/" + file.first);
        //Count the number of files and file paths containing "test"
        if (Util::regexFind(file.first, "[tT][eE][sS][tT]") != std::string::npos)
            testFileCount += 1;
        //Iterate through the list of target lines
        for (int lineNo : file.second) {
            //Get the full line up to the semicolon
            std::string statement = jp.getFullStatement(lineNo);
            //Check if the line explicitly calls Runtime.getRuntim().exec()
            bool hasRuntime = (statement.find("Runtime.getRuntime().exec(") != std::string::npos);
            bool hasProcess = false;
            //Otherwise we want to see if .exec( returns a Process
            //Use a regex to see if there is an assignment on the left side
            int nameStart = Util::regexFind(statement, "\\w+ *= *.*\\.exec\\(");
            //If the search was successful
            if (nameStart != std::string::npos) {
                //Find the end of the name being a space or an equals sign
                int nameEnd = Util::regexFind(statement, "( |=)", nameStart);
                //If there's no end then that's pretty weird
                if (nameEnd != std::string::npos) {
                    //Get the name of the lvalue and check if its type is "Process"
                    std::string name = statement.substr(nameStart, nameEnd - nameStart);
                    hasProcess = (jp.findType(name, jp.getFunctionName(lineNo)) == "Process");
                }   
            }
            //If neither is true, continue to the next candidate
            if (!hasRuntime && !hasProcess)
                continue;
            //If the line is commented than continue
            else if (jp.isCommented(lineNo))
                continue;
            //Add to the number of functions actually containing runtime
            runtimeFileCount += 1;
            runtimeFunctionCount += 1;
            //Set initial values for hardcoded and input, we'll look for opposite
            bool hardcoded = true;
            bool hasInput = false;
            //Iterate through all of the inputs to .exec()
            for (const std::string& s : jp.parseRecursively("exec", lineNo)) {
                //Get the name of the function at the line
                std::string functionName = jp.getFunctionName(lineNo);
                //Get the type of the string s in the function
                std::string type = jp.findType(s, functionName);
                //If type is missing, it might be a class member
                if (type.empty()) {
                    type = jp.findMemberType(s);
                }
                //Check if type is already in typeCounts, add it if not
                if (!typeCounts.count(type))
                    typeCounts[type] = 1;
                //If it's already there just increment it
                else
                    typeCounts[type] += 1;
                //If jp is not hardcoded, then the whole line isn't
                if (!jp.isHardcoded(s, functionName)) {
                    hardcoded = false;
                }
                //Otherwise add it to typeHardcoded like typeCounts
                else {
                    if (!typeHardcoded.count(type))
                        typeHardcoded[type] = 1;
                    else
                        typeHardcoded[type] += 1;
                }
                //If s is an input to the function surrounding .exec(
                //Add it to typeInput like typeCounts
                if (jp.isInput(s, functionName)) {
                    hasInput = true;
                    if (!typeInput.count(type))
                        typeInput[type] = 1;
                    else
                        typeInput[type] += 1;
                }
            }
            //Keep track of the usages for printing with the flags
            std::string usage = file.first + ": " + std::to_string(lineNo) + ":\n" + statement;
            //Keep track of how many functions have input and how many are hardcoded
            if (hasInput) {
                inputFunctionCount += 1;
                inputUses.push_back(usage);
            }
            else if (hardcoded) {
                hardcodedFunctionCount += 1;
                hardcodedUses.push_back(usage);
            }
            else {
                otherUses.push_back(usage);
            }
        }
    }
    //Add an endline for the \r
    std::cout << "\n" << std::endl;
    //Print the number omitted due to not being Runtime.exec()
    std::cout << totalFunctionCount << " candidates given, ";
    std::cout << (totalFunctionCount - runtimeFunctionCount);
    std::cout << " omitted for being commented or lacking Runtime" << std::endl;
    //Print the total, hardcoded, and containing input
    std::cout << "Out of " << runtimeFunctionCount << " uses: ";
    std::cout << hardcodedFunctionCount << " hardcoded, ";
    std::cout << inputFunctionCount << " from function input, ";
    std::cout << (runtimeFunctionCount - hardcodedFunctionCount - inputFunctionCount);
    std::cout << " other" << std::endl;
    //Print the number of "test" files
    std::cout << testFileCount << "/" << totalFileCount;
    std::cout << " file paths contain \"test\"" << std::endl;
    //Print header for the function table
    std::cout << "\nExec Input Table\n" << std::endl;
    //Print all of the types, their total/hardcoded/input count
    std::cout << std::left << std::setw(20) << "Variable Type" << std::right << " | ";
    std::cout << std::left << std::setw(7) << "Total" << std::right << " | ";
    std::cout << std::left << std::setw(9) << "Hardcoded" << std::right << " | ";
    std::cout << std::left << std::setw(7) << "Input" << std::endl;
    std::cout << std::string(48, '-') << std::endl;
    //Look through typeCount, typeHardcoded, typeInput and print values
    for (auto const& x : typeCounts) {
        std::cout << std::left << std::setw(20) << x.first << std::right << " | ";
        std::cout << std::left << std::setw(7) << x.second << std::right << " | ";
        std::cout << std::left << std::setw(9) << typeHardcoded[x.first] << " | ";
        std::cout << std::left << std::setw(7) << typeInput[x.first] << std::endl; 
    }
    
    //Print all of the hardcoded uses
    if (printHardcode) {
        std::cout << "\nHardcoded Uses:" << std::endl;
        for (const std::string& s : hardcodedUses)
            std::cout << s << std::endl;
    }
    //Print all of the input uses
    if (printInput) {
        std::cout << "\nInput Uses:" << std::endl;
        for (const std::string& s : inputUses)
            std::cout << s << std::endl;
    }
    //Print all of the other uses
    if (printOther) {
        std::cout << "\nOther Uses:" << std::endl;
        for (const std::string& s : otherUses)
            std::cout << s << std::endl;
    }
    
    return 0;
}

