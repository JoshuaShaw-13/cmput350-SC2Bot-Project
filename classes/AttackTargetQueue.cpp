#include <queue>
#include <cassert>
#include "AttackBaseQueue.h"

class AttackTargetQueue{
    public:
    AttackTargetQueue(Base &base):base(base){}; //contructor
    ~AttackTargetQueue(){}; //destructor
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

    private:
    //data
    std::priority_queue<Target*>* data;
    Base* base;

};

struct Target{
    Target(int p, sc2::Point2D loc):priority(p), loc(loc){ assert(p>0);};

    bool operator>(Target &t){
        /** comparison override: compare in terms of priority, closer to 1 priority = higher priority */
        return priority < t;
    }

    int distanceFromAgent();
    //data
    int priority; //int representation of attack priority: TODO: evaluate priority from location and strength
    sc2::Point2D loc; //Targets's (x,y) coord location
};