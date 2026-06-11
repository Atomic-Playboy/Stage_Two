#ifndef UTILS_H
#define UTILS_H

#include <string>

int safeStoi(const std::string& str, int defaultValue = 0);
std::string toLowerCase(std::string str);
bool containsSubstringIgnoreCase(std::string haystack, std::string needle);
void appendToLogFile(const std::string& line);

#endif // UTILS_H
