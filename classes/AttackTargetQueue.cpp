/**
 * Specialized queue encapsulation for tracking attack order of targets (within a base)
 */
#include "AttackTargetQueue.h"
#include <queue>
#include <cassert>
#include "AttackBaseQueue.h"


AttackTargetQueue(const AttackTargetQueue &q){
    /** copy constructor */
    this->data = q.data;
    this->base = q.base;
}; 
AttackTargetQueue &operator=(const AttackTargetQueue &q){
    /** assignment operator */
    this->data = q.data;
    this->base = q.base;
    return *this;
};

//getter(s)
std::priority_queue<Target*>* getQueue(){ return this->data;}
Base* getBase(){ return this->base;}

//methods
bool isEmpty(){
    /** returns true if queue is empty */
    return data->empty();
}
Target* peek()const{
    /** returns a pointer reference to the next target in the queue */
    return data->top();
}
Target* pop(){
    /** removes the next target in the queue, and returns a pointer reference to this target */
    Target* popped = data.top();
    data.pop();
    return popped;
}
void push(const Target &t){
    /** adds a new target to the queue */
    data->push(t);
}


