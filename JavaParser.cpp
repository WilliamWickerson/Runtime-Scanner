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
 * File:   JavaParser.cpp
 * Author: William Wickerson
 * 
 * Created on December 3, 2017, 9:01 PM
 */

#include "JavaParser.h"
#include "Utility.h"
#include <algorithm>

#include <iostream>

JavaParser::JavaParser(const std::string& filePath) : javaReader(filePath) {};

std::string JavaParser::getFunctionName(int lineNumber) {
    return this->javaReader.readFunctionName(lineNumber);
}

std::string JavaParser::getFullStatement(int lineNumber) {
    //Read extra lines to account for the line wrapping
    std::string extraLines = this->javaReader.readLines(
        std::pair<int,int>(lineNumber, lineNumber + 10));
    int quoteDepth = 0;
    int statementEnd = 0;
    //Need to account for ';' being inside a string
    while ((extraLines[statementEnd] != ';') || (quoteDepth > 0)) {
        statementEnd++;
        if (extraLines[statementEnd] == '"')
            quoteDepth = (quoteDepth + 1) % 2;
    }
    //Take the substring and replace newline characters with spaces
    std::string statement = extraLines.substr(0, statementEnd);
    std::replace(statement.begin(), statement.end(), '\n', ' ');
    return statement;
}
  
std::string JavaParser::findType(const std::string& variableName, const std::string& functionName) {
    //If standalone argument is surrounded by " " it's a string
    if (Util::startsWith(variableName,"\"") && Util::endsWith(variableName,"\""))
        return std::string("String literal");
    //If it only has numerical digits it is an integer
    if (variableName.find_first_not_of("0123456789") == std::string::npos)
        return std::string("Integer literal");
    //If it has a decimal point followed by a decimal it's a double
    if (Util::regexFind(variableName, "[0-9]*\\.[0-9]+") != std::string::npos)
        return std::string("Double literal");
    //'this' has the same type as the parent class
    if (!variableName.compare("this"))
        return javaReader.getClassName();
    //Null is likely string type, but making it null type was simpler
    if (!variableName.compare("null"))
        return std::string("null");
    if (Util::regexFind(variableName, "\\(.*\\) *null") != std::string::npos)
        return std::string("null");
    //There should be more of these, but most inputs are just String and String[]
    if (!variableName.compare("String") || !variableName.compare("Integer") || !variableName.compare("System"))
        return std::string("Static Class");
    //If there is a new keyword, the argument will tell us its type
    if (Util::startsWith(variableName, "new")) {
        //Arrays can have specified size, so need to avoid capturing that
        if (Util::startsWith(variableName, "new String["))
            return std::string("String[]");
        int typeStart = 4;
        int typeEnd = Util::regexFind(variableName, "( |\\{|\\()", 4);
        return variableName.substr(typeStart, typeEnd - typeStart);
    }
    //If there is a cast, the argument will also tell us its type
    //Otherwise we have a regular function
    if (Util::regexFind(variableName, "\\(.*\\)") != std::string::npos)
        return std::string("function");
    //If there is a plus sign at the current depth, then we have an expression
    if (variableName.find("+") != std::string::npos)
        if (!Util::getDepth(variableName, variableName.find("+")))
            return std::string("expression");
    //Get the function body so we can look for variable definitions
    std::string functionBody = this->javaReader.readFunction(functionName);
    //One file has an ugly space, so I've corrected it like this
    if (functionBody.find("String [] " + variableName) != std::string::npos)
        return std::string("String[]");
    //Need to declare internal variables outside for the do while loop
    int lineNumber = 0;
    int firstOccurance = 0;
    do {
        //The "[\w<>\[\]] variableName" regex should represent all type definitions
        firstOccurance = Util::regexFind(functionBody, "[\\w<>\\[\\]]+ " + Util::escapeRegex(variableName), firstOccurance + 1);
        //If the variable is defined outside the function just return empty string
        if (firstOccurance == std::string::npos) {
            return std::string();
        }
        //The associated line number is the start of function plus number of newlines
        lineNumber = this->javaReader.getFunctionBounds(functionName).first +
                std::count(functionBody.begin(), functionBody.begin() + firstOccurance, '\n');
    //If line is commented, or declaration is inside (){}"", keep looking
    } while (this->isCommented(lineNumber));
    //Return the substring from the regexFind to the next space
    return functionBody.substr(firstOccurance,
            functionBody.find(" ", firstOccurance) - firstOccurance);
}

std::string JavaParser::findMemberType(const std::string& variableName) {
    //Get the class name so we can find where the constructor is
    //Hoping good style is used, member functions should be before it
    std::string className = this->javaReader.getClassName();
    int endOfMembers = this->javaReader.getFunctionBounds(className).first - 1;
    //If the file doesn't have a constructor than this is the failure value
    if (endOfMembers == -2)
        endOfMembers = 1000;
    //Extract all of the text between the start of the file and the constructor
    std::string textRegion = this->javaReader.readLines(std::pair<int,int>(1,endOfMembers));
    //Look for the start of a method declaration (same as findType but "(=|;)")
    int typeStart = Util::regexFind(textRegion, "[\\w<>\\[\\]]* " + Util::escapeRegex(variableName) + " *(=|;)");
    //If no matching declaration was found return empty string
    if (typeStart == std::string::npos)
        return std::string();
    //Otherwise find the space after the regex and return the type
    int typeEnd = textRegion.find(" ", typeStart);
    std::string type = textRegion.substr(typeStart, typeEnd - typeStart);
    return type;
}

bool JavaParser::isInput(const std::string& variableName, const std::string& functionName) {
    //Find the function header, being all of the text before "{"
    std::string functionBody = this->javaReader.readFunction(functionName);
    std::string functionHeader = functionBody.substr(0, functionBody.find("{"));
    //Find the type of the variable to keep regexing separate
    std::string variableType = this->findType(variableName, functionName);
    //One of the weird poorly formatted files coming back to haunt me
    if (variableType == "String[]")
        if (functionHeader.find("String [] " + variableName) != std::string::npos)
            return true;
    //Return whether or not the definition was in the header
    return (functionHeader.find(variableType + " " + variableName) != std::string::npos);
}

bool JavaParser::isHardcoded(const std::string& variableName, const std::string& functionName) {
    //Check whether the variable is input to the function, if so it's not hardcoded
    if (this->isInput(variableName, functionName))
        return false;
    //Get the function text and also the type of the variable
    std::string functionBody = this->javaReader.readFunction(functionName);
    std::string variableType = this->findType(variableName, functionName);
    if (variableType.empty())
        variableType = this->findMemberType(variableName);
    //If the type is string, check if it was given hard coded " "
    if (!variableType.compare("String")) {
        //Get location so we have somewhere to start
        int location = Util::regexFind(functionBody, Util::escapeRegex(variableName) + " *= *.*;");
        if (location != std::string::npos) {
            //If the string is given a literal return true
            if (Util::regexFind(functionBody, Util::escapeRegex(variableName) + " *= *\".*\"") != std::string::npos)
                return true;
            //Otherwise check the input
            int decStart = functionBody.find("=", location) + 1;
            int decEnd = functionBody.find(";", location);
            std::string rightSide = functionBody.substr(decStart, decEnd - decStart);
            return this->isHardcoded(Util::trim(rightSide), functionName);
        }
        //If the string was created outside of the function, check class variables
        int classVarEnd = this->javaReader.getFunctionBounds(this->javaReader.getClassName()).first - 1;
        std::string classVars = this->javaReader.readLines(std::pair<int,int>(1, classVarEnd));
        return (Util::regexFind(classVars, Util::escapeRegex(variableName) + " *= *\".*\"") != std::string::npos);
    }
    //If the type is int, check if it was defined as a given digit
    else if (!variableType.compare("int")) {
        if (Util::regexFind(functionBody, Util::escapeRegex(variableName) + " *= *\\d+") != std::string::npos)
            true;
        int classVarEnd = this->javaReader.getFunctionBounds(this->javaReader.getClassName()).first - 1;
        std::string classVars = this->javaReader.readLines(std::pair<int,int>(1, classVarEnd));
        return (Util::regexFind(classVars, Util::escapeRegex(variableName) + " *= *\\d+") != std::string::npos);
    }
    //"Null" is an imaginary type for classing these, all are hardcoded
    else if (!variableType.compare("null")) {
        return true;
    }
    //For string arrays, check their definition for any non-hardcoded variables
    else if (!variableType.compare("String[]")) {
        std::string stringArr = this->getStringArr(variableName, functionName);
        //If they were not hard coded with { }, check function if it exists
        if (stringArr.empty()) {
            //Find first allocation and check if right side is hard coded
            int location = Util::regexFind(functionBody, Util::escapeRegex(variableName) + " *= *.*;");
            if (location != std::string::npos) {
                int decStart = functionBody.find("=", location) + 1;
                int decEnd = functionBody.find(";", location);
                std::string rightSide = functionBody.substr(decStart, decEnd - decStart);
                return this->isHardcoded(Util::trim(rightSide), functionName);
            }
        }
        //If the array is not empty, recursively split it, and check if any part is not hardcoded
        else {
            for (const std::string& s : this->parseRecursively(this->parseStringArr(stringArr), functionName))
                if (!this->isHardcoded(s, functionName))
                    return false;
            return true;
        }
    }
    //For functions we need to worry about whether all of the inputs are hardcoded
    else if (!variableType.compare("function")) {
        std::vector<std::string> parts = this->parseExpression(variableName);
        std::vector<std::string> functionParts = this->parseFunction(variableName);
        for (const std::string& s : this->parseRecursively(functionParts, functionName))
            if (!this->isHardcoded(s, functionName))
                return false;
        return true;
    }
    //For expressions we do the same thing as functions but start differently
    else if (!variableType.compare("expression")) {
        std::vector<std::string> parts = this->parseExpression(variableName);
        for (const std::string& s : this->parseRecursively(parts, functionName))
            if (!this->isHardcoded(s, functionName))
                return false;
        return true;
    }
    //If the variable is a literal, return true
    else if (variableType.find(" literal") != std::string::npos) {
        return true;
    }
    //Source code is hardcoded, not the best trick calling this a variable
    else if (!variableType.compare("Static Class"))
        return true;
    //Result may or may not be hard coded, need more tests
    else {
        return false;
    }
    //Something made it past its own block, return true
    return true;
}

//TODO: slowest link, move to JavaReader main
bool JavaParser::isCommented(int lineNumber) {
    //Get the start of the function so we can read it in
    int functionStart = this->javaReader.getFunctionBounds(lineNumber).first;
    //Read in the function up to the specified line
    std::string functionBody = this->javaReader.readLines(
            std::pair<int,int>(functionStart, lineNumber));
    //Reverse find up to the following so we can check for comments
    int bodyEnd = functionBody.size() - 2;
    int slashLoc = functionBody.rfind("//", bodyEnd);
    int endlLoc = functionBody.rfind("\n", bodyEnd);
    int openLoc = functionBody.rfind("/*", bodyEnd);
    int closeLoc = functionBody.rfind("*/", bodyEnd);
    //If there is a // on this line, return true
    if (slashLoc != std::string::npos)
        if (slashLoc > endlLoc)
            return true;
    //If there is no open return false
    if (openLoc == std::string::npos)
        return false;
    //If there is an open and no close return true
    else if (closeLoc == std::string::npos)
        return true;
    //Otherwise return whether the nearest one is an open or a close
    else
        return (openLoc > closeLoc);
}

std::string JavaParser::getExpression(const std::string& functionName, int lineNumber) {
    //Get the full line at the line number
    std::string statement = this->getFullStatement(lineNumber);
    //return empty string if the function is not there
    if (statement.find(functionName + "(") == std::string::npos)
        return std::string();
    //The expression starts after the first open parentheses
    int expressionStart = statement.find(functionName) + functionName.length() + 1;
    int parenDepth = 1;
    //We need to find when the depth of parentheses goes back to 0
    for (int i = expressionStart; i < statement.length(); i++) {
        //Open parentheses increase depth
        if (statement[i] == '(')
            parenDepth++;
        //Close parentheses decrease depth
        else if (statement[i] == ')')
            parenDepth--;
        //If we have exited the function, return the substring
        if (parenDepth == 0)
            return statement.substr(expressionStart, i - expressionStart);
    }   
}

std::string JavaParser::getStringArr(const std::string& stringArrName, const std::string& functionName) {
    //Try and find { } of an array declaration
    int arrStart = stringArrName.find_first_of("{");
    int arrEnd = stringArrName.find_last_of("}") + 1;
    //If { } are found then return the { } part
    if ((arrStart != std::string::npos) && (arrEnd != std::string::npos))
        return stringArrName.substr(arrStart, arrEnd - arrStart);
    //Otherwise we need to look for the declaration
    int location = -1;
    std::string functionBody = this->javaReader.readFunction(functionName);
    //If there is a declaration inside the function, return the { } part
    if ((location = Util::regexFind(functionBody, stringArrName + " *=[^;]*\\{.*\\}")) != std::string::npos) {
        int arrayStart = functionBody.find("{", location);
        int arrayEnd = functionBody.find("}", arrayStart);
        std::string array = functionBody.substr(arrayStart, arrayEnd - arrayStart);
        std::replace(array.begin(), array.end(), '\n', ' ');
        return array;
    }
    //If there is a declaration in the class variables, return the { } part
    int classVarEnd = this->javaReader.getFunctionBounds(this->javaReader.getClassName()).first - 1;
    std::string classVars = this->javaReader.readLines(std::pair<int,int>(1, classVarEnd));
    if ((location = Util::regexFind(classVars, stringArrName + " *=[^;]*\\{.*\\}")) != std::string::npos) {
        int arrayStart = classVars.find("{", location);
        int arrayEnd = classVars.find("}", arrayStart);
        std::string array = classVars.substr(arrayStart, arrayEnd - arrayStart);
        std::replace(array.begin(), array.end(), '\n', ' ');
        return array;
    }
    //Couldn't find anything so return an empty string
    return std::string();
}

std::vector<std::string> JavaParser::parseExpression(const std::string& functionName, int lineNumber) {
    //Get the parsed expression at the line number
    std::string expression = this->getExpression(functionName, lineNumber);
    //Return the parsed variables
    return this->parseExpression(expression);
}

std::vector<std::string> JavaParser::parseExpression(const std::string& expression) {
    //Make a vector for the variables
    std::vector<std::string> variables;
    //If there is no expression, we can just exit now
    if (expression.empty())
        return variables;
    //Split the expression first by function arguments
    std::vector<std::string> parts = Util::splitNotAtDepth(expression, ",");
    for (const std::string& s : parts) {
        //Then split each by the string addition "+" and add to variables
        for (const std::string& spart : Util::splitNotAtDepth(s, "+")) {
            variables.push_back(Util::trim(spart));
        }
    }
    return variables;    
}

std::vector<std::string> JavaParser::parseStringArr(const std::string& stringArr) {
    //Get the inside without the { } part
    int arrStart = stringArr.find_first_of("{") + 1;
    int arrEnd = stringArr.find_last_of("}");
    std::string contents = stringArr.substr(arrStart, arrEnd - arrStart);
    //Split the inside not at depth by , to get the parts of the array
    std::vector<std::string> parts = Util::splitNotAtDepth(contents, ",");
    //Trim each of the strings and return
    for (std::string& s : parts)
        s = Util::trim(s);
    return parts;
}

std::vector<std::string> JavaParser::parseFunction(const std::string& function) {
    std::vector<std::string> parts;
    //Check whether it is a member function
    if (function.find(".") != std::string::npos) {
        //If there is and it's not inside the arguments, add it to parts
        if (function.find(".") < function.find("(")) {
            std::string owner = function.substr(0, function.find("."));
            parts.push_back(owner);
        }
    }
    //Otherwise we need to look for all function arguments in the chain
    int parenDepth = 0;
    int argStart = 0;
    for (int i = 0; i < function.length(); i++) {
        //Open increases depth
        if (function[i] == '(') {
            if (parenDepth == 0)
                argStart = i + 1;
            parenDepth++;
        }
        //Close decrease depth
        else if (function[i] == ')') {
            parenDepth--;
            //If the depth returned to 0, that's the end of a set of arguments
            if (parenDepth == 0) {
                std::string expression = function.substr(argStart, i - argStart);
                //Parse the expression and add each to parts
                for (const std::string& s : this->parseExpression(expression))
                    parts.push_back(Util::trim(s));
            }
        }
    }
    return parts;
}

std::vector<std::string> JavaParser::parseRecursively(const std::string& functionName, int lineNumber) {
    //Assumed to start on an expression, so we get the first parts
    std::vector<std::string> parts = this->parseExpression(functionName, lineNumber);
    std::string currentFunction = this->getFunctionName(lineNumber);
    //Get the function name, and let the private recursive function handle it
    return this->parseRecursively(parts, currentFunction);
}

std::vector<std::string> JavaParser::parseRecursively(const std::vector<std::string>& parts, const std::string& functionName) {
    std::vector<std::string> returnParts;
    //For each thing in parts, we need to try and split it and then call parseRecursively on that
    for (const std::string& part : parts) {
        //Get the type so we know how to parse it
        std::string type = this->findType(part, functionName);
        //Get member type if type not available
        if (type.empty())
            type = findMemberType(part);
        //If String[], parse String Array
        if (type == "String[]") {
            std::string stringArr = this->getStringArr(part, functionName);
            //If there is no { } to parse just add the array
            if (stringArr.empty()) {
                returnParts.push_back(Util::trim(part));
            }
            //Otherwise parse the { } and then return parseRecursively on that
            else {
                std::vector<std::string> stringArrParts = this->parseStringArr(stringArr);
                for (const std::string& s : this->parseRecursively(stringArrParts, functionName))
                    returnParts.push_back(Util::trim(s));
            }
        }
        //If the type is a function, parse the function and parse that recursively
        else if (type == "function") {
            std::vector<std::string> functionParts = this->parseFunction(part);
            for (const std::string& s : this->parseRecursively(functionParts, functionName))
                returnParts.push_back(Util::trim(s));
        }
        //If the type is an expression, parse the expression and parse recursively
        else if (type == "expression") {
            std::vector<std::string> expressionParts = this->parseExpression(part);
            for (const std::string& s : this->parseRecursively(expressionParts, functionName))
                returnParts.push_back(Util::trim(s));
        }
        //Otherwise we've reached something that cannot be parsed further so return it
        else {
            returnParts.push_back(Util::trim(part));
        }
    }
    //Return the result of all of the leafs of the recursive parsing
    return returnParts;
}