/**
 * Specialized Queue encapsulation for tracking the build order
 */
#include "BuildQueue.h"
#include <queue>
#include "BuildQueue.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

using namespace BuildQueue;

BuildQueue(const BuildQueue &q){
    /** copy constructor */
    this->data = q.data;
};
BuildQueue &operator=(const BuildQueue &q){
    /** assignment operator */
    this->data = q->data;
    return *this;
};

//getter(s)
std::queue<BuildOrderItem*>* getQueue() const{
    return this->data;
}

//methods
bool isEmpty() const{
    /** returns true if the queue is empty */
    return data->empty();
}
BuildOrderItem* peek() const{
    /** returns a pointer to the next building in the queue */
    return data->front();}
BuildOrderItem* pop() {
    /** removes the next building in the queue, and returns a pointer to that building */
    BuildOrderItem* popped = data->front();
    data->pop();
    return popped;
    };
void push(const BuildOrderItem &BuildOrderItem){
    /**  adds the given BuildOrderItem to the queue*/
    data->push(BuildOrderItem);
    }
