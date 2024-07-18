#pragma once

#ifndef BOARDHISTORY_H
#define BOARDHISTORY_H

#include <vector>
#include <unordered_map>

class BoardHistory {
private:
	std::vector<unsigned long long int> history;
	std::unordered_map<unsigned long long int, size_t> mapHistory;
public:
	void push(unsigned long long int value);
	void pop();
	unsigned long long int operator[](const int& index);
	bool operator[](const unsigned long long int& key);
};

#endif // !BOARDHISTORY_H
