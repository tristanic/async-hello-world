#include <thread>
#include <future>
#include <chrono>
#include <sstream>



class Foo
{
public:

    bool thread_done() const { return !thread_running_; }

    std::string do_something_slow(int num_seconds) {
        std::this_thread::sleep_for(std::chrono::seconds(num_seconds));
        std::stringstream msg;
        msg << "It took me " << num_seconds << " seconds to say this: Hello, world!";
        return msg.str();
    }

    void do_something_slow_asynchronously(int num_seconds)
    {
        thread_result_ = std::async(std::launch::async,
            &Foo::do_something_slow_asynchronously_, this, num_seconds
        );
    }

    std::string get_asynchronous_result()
    {
        return thread_result_.get();
    }




private:
    bool thread_running_ = false;
    std::string do_something_slow_asynchronously_(int num_seconds)
    {
        if(thread_running_)
            throw std::runtime_error("Thread already running!");
        thread_running_ = true;
        auto r = do_something_slow(num_seconds);
        thread_running_ = false;
        return r;
    };
    std::future<std::string> thread_result_;
};
