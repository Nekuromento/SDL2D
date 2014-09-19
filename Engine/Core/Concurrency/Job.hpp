#ifndef Job_h__
#define Job_h__

struct Job {
    typedef void (* const ConstJobFunction)(void*);

    Job();
    Job(ConstJobFunction function, void* const payload = nullptr);

    void run();

private:
    typedef void (* JobFunction)(void*);

    JobFunction function;
    void* payload;
};

#endif // Job_h__