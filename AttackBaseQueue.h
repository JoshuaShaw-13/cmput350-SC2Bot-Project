#ifndef ATTACKBASEQUEUE_H
#define ATTACKBASEQUEUE_H

#include <queue>
#include <sc2api/sc2_common.h>

/**
 * Specialized queue encapsulation for tracking which enemy base to attack
 */

struct EnemyBase{
    /** struct for holding Enemy Base information */
    bool operator<(EnemyBase &b){
        /** Overide comparison operator 
         *  Compare in terms of distance from our agent
        */
        
    }
    sc2::Point2D distanceFromAgent(){};
    sc2::Point2D loc; //Base's (x,y) coord location
    };

class AttackBaseQueue{
    public:
    AttackBaseQueue(); //contructor
    ~AttackBaseQueue(); //destructor
    AttackBaseQueue(const AttackBaseQueue &q); //copy constructor
    AttackBaseQueue &operator=(const AttackBaseQueue &q); //assignment operator

    //getter(s)
    std::priority_queue<EnemyBase*>* getQueue();

    //methods
    bool isEmpty();/** returns true if queue is empty */;
    EnemyBase* peek() const;/** returns a pointer reference to the next Base in the queue */
    EnemyBase* pop();/** removes the next Base in the queue, and returns a pointer reference to this Base */
    void push(const EnemyBase &base);/** adds a new base to the queue */

    private:
    //data
    std::priority_queue<EnemyBase*>* data;
};



#endif
