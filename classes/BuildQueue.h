/**
 * Specialized Queue encapsulation for tracking the build order
 */

#include <queue>

class BuildQueue{
    public:
        BuildQueue(){};//constructor
        ~BuildQueue(){};//deconstructor
        BuildQueue(const BuildQueue &q){};//copy constructor
        BuildQueue &operator=(const BuildQueue &q){};//assignment operator

        //getter(s)
        std::queue<Unit*>* getQueue() const{}

        //methods
        bool isEmpty() const{/** returns true if the queue is empty */ }
        Unit* peek() const{/** returns a pointer to the next building in the queue */}
        Unit* pop() {/** removes the next building in the queue, and returns a pointer to that building */};
        void push(const Unit &unit){/**  adds the given Unit to the queue*/}

    private:
    //data
    std::queue<Unit*>* data;

};

struct Unit{/** struct for holding Unit information */};