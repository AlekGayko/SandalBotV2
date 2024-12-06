#pragma once

#ifndef BOARDHISTORY_H
#define BOARDHISTORY_H

#define u64 uint64_t

#include <iostream>

class BoardHistory {
private:
	const int historySize = 2048;
	u64* hashHistory = nullptr;
	int* startSearchIndicies = nullptr;
	int numBoards = 0;
public:
	BoardHistory();
	~BoardHistory();
	void push(u64 value, bool reset);
	void pop();
	bool contains(const u64& key);
};

#endif // !BOARDHISTORY_H
