#pragma once

#ifndef BOARDHISTORY_H
#define BOARDHISTORY_H

#include <vector>
#include <unordered_map>

class BoardHistory {
private:
	std::vector<uint64_t> history;
	std::unordered_map<uint64_t, size_t> mapHistory;
public:
	void push(uint64_t value);
	void pop();
	uint64_t operator[](const int& index);
	bool operator[](const uint64_t& key);
};

#endif // !BOARDHISTORY_H
