/**
 * Specialized queue encapsulation for tracking attack order of targets (within a base)
 */
#ifndef ATTACKTARGETQUEUE_H
#define ATTACKTARGETQUEUE_H

#include <queue>
#include "AttackBaseQueue.h"

class AttackTargetQueue{
    public:
    AttackTargetQueue(){}; //contructor
    ~AttackTargetQueue(){}; //destructor
    AttackTargetQueue(const AttackTargetQueue &q){}; //copy constructor
    AttackTargetQueue &operator=(const AttackTargetQueue &q){}; //assignment operator

    //getter(s)
    std::priority_queue<Target*>* getQueue(){};
    Base* getBase(){};

    //methods
    bool isEmpty(){/** returns true if queue is empty */};
    Target* peek()const{/** returns a pointer reference to the next target in the queue */}
    Target* pop(){/** removes the next target in the queue, and returns a pointer reference to this target */}
    void push(const Target &base){/** adds a new target to the queue */}

    private:
    //data
    std::priority_queue<Target*>* data;
    Base* base;

};

struct Target{};

#endif