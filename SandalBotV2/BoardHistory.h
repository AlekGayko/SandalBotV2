#pragma once

#ifndef BOARDHISTORY_H
#define BOARDHISTORY_H

#include <vector>
#include <unordered_map>

class BoardHistory {
private:
	std::unordered_map<uint64_t, size_t> mapHistory;
public:
	BoardHistory();
	~BoardHistory();
	void push(uint64_t value);
	void pop(uint64_t value);
	bool operator[](const uint64_t& key);
};

#endif // !BOARDHISTORY_H
