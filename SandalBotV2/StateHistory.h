#pragma once
#ifndef STATEHISTORY_H
#define STATEHISTORY_H

class BoardState;

class StateHistory {
private:
	const size_t defaultSize = 100;
	size_t size = 0;
	size_t allocatedSize = 0;
	BoardState* history = nullptr;
public:
	StateHistory();
	~StateHistory();
	void push(BoardState& state);
	void pop();
	void clear();
	BoardState& getSecondLast();
	BoardState& back();
};

#endif // !STATEHISTORY_H
