/**
 * Specialized queue encapsulation for tracking which enemy base to attack
 */

#include <queue>
#include <sc2_common.h>

class AttackBaseQueue{
    public:
    AttackBaseQueue(){}; //contructor
    ~AttackBaseQueue(){}; //destructor
    AttackBaseQueue(const AttackBaseQueue &q){
        //copy constructor
        this->data = q.data;
    }; 
    AttackBaseQueue &operator=(const AttackBaseQueue &q){
        //assignment operator
        this->data = q.data;
        return *this;
    }; 

    //getter(s)
    std::priority_queue<Base*>* getQueue(){
        return this->data;
    };

    //methods
    bool isEmpty(){
        /** returns true if queue is empty */
        return data->empty();};
    Base* peek()const{
        /** returns a pointer reference to the next Base in the queue */
        return data->top();}
    Base* pop(){
        /** removes the next Base in the queue, and returns a pointer reference to this Base */
        Base* popped = data->top();
        data->pop();
        return popped;
        }
    void push(const Base &base){
        /** adds a new base to the queue */
        data->push(base);}

    private:
    //data
    std::priority_queue<Base*>* data;
};

struct Base{
    /** struct for holding Enemy Base information */
    bool operator<(Base &b){
        /** Overide comparison operator 
         *  Compare in terms of distance from our agent
        */

    }
    int distanceFromAgent();
    //data
    sc2::Point2D loc; //Base's (x,y) coord location
    };