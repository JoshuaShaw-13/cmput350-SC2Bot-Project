/**
 * Specialized queue encapsulation for tracking which enemy base to attack
 */
#include "AttackBaseQueue.h"


AttackBaseQueue::AttackBaseQueue(const AttackBaseQueue &q){
    //copy constructor
    this->data = q.data;
}; 
AttackBaseQueue& AttackBaseQueue::operator=(const AttackBaseQueue &q){
    //assignment operator
    this->data = q.data;
    return *this;
}; 

//getter(s)
std::priority_queue<EnemyBase*>* AttackBaseQueue::getQueue(){
    return this->data;
};

//methods
bool AttackBaseQueue::isEmpty(){
    /** returns true if queue is empty */
    return data ? data->empty() : true;}
EnemyBase* AttackBaseQueue::peek()const{
    /** returns a pointer reference to the next Base in the queue */
    return (data && !data->empty()) ? data->top() : nullptr;}
EnemyBase* AttackBaseQueue::pop(){
    /** removes the next Base in the queue, and returns a pointer reference to this Base */
    EnemyBase* popped = data->top();
    data->pop();
    return popped;
    }
void AttackBaseQueue::push(const EnemyBase &base){
    /** adds a new base to the queue */
    data->push(new EnemyBase(base));}

// Destructor
AttackBaseQueue::~AttackBaseQueue() {
    while (data && !data->empty()) {
        delete data->top();
        data->pop();
    }
    delete data;
}

// Constructor
AttackBaseQueue::AttackBaseQueue() : data(new std::priority_queue<EnemyBase*>()) {}