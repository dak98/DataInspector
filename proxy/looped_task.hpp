#ifndef LOOPED_TASK_HPP_
#define LOOPED_TASK_HPP_

#include <thread>

class looped_task
{
public:
    template <class F, class... Args>
    explicit looped_task(F&& f, Args&&... args) {
	run = true;
	t = std::thread([&]()
	{
	    while (run)
		f(args...);
	});
    }

    looped_task(const looped_task& other) = delete;
    looped_task(looped_task&& other) noexcept = delete;
    
    ~looped_task() {
	run = false;
	if (t.joinable())
	    t.join();
    }

    looped_task& operator=(const looped_task& other) = delete;
    looped_task& operator=(looped_task&& other) noexcept = delete;

    void stop() noexcept { run = false; }
private:
    bool run {false};
    std::thread t;
};

#endif // LOOPED_TASK_HPP_
