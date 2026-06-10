#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

int safeStoi(const std::string& str, int defaultValue = 0);
std::string toLowerCase(std::string str);
bool containsSubstringIgnoreCase(std::string haystack, std::string needle);
bool parseChannelMatch(const std::string& confChan, unsigned char midiStatusByte);
unsigned char getOutputChannelByte(unsigned char statusBase, const std::string& confChan);
void appendToLogFile(const std::string& line);

#endif // UTILS_H
