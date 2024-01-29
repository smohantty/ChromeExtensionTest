#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <chrono>

template <typename T>
class ConcurrentQueue {
public:
    // Push a value onto the queue
    void push(const T& value) {
        {
            // Lock the mutex to protect the shared data (queue)
            std::lock_guard<std::mutex> lock(mutex_);
            
            // Push the value onto the queue
            queue_.push(value);
        } // Lock is automatically released when lock_guard goes out of scope
        
        // Notify one waiting thread that data is available
        condition_.notify_one();
    }

    // Pop a value from the queue (non-blocking)
    std::optional<T> pop() {
        // Lock the mutex to protect the shared data (queue)
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Check if the queue is not empty
        if (!queue_.empty()) {
            // Retrieve the front value
            T value = queue_.front();
            
            // Pop the value from the queue
            queue_.pop();
            
            // Return the popped value
            return value;
        }
        
        // Queue is empty, return an empty optional
        return std::nullopt;
    }

    // Pop a value from the queue with a specified timeout (blocking)
    std::optional<T> pop(std::chrono::milliseconds timeout) {
        // Unique lock allows manual unlocking
        std::unique_lock<std::mutex> lock(mutex_);

        // Wait for the condition to be true or until the timeout expires
        if (condition_.wait_for(lock, timeout, [this] { return done_ || !queue_.empty(); })) {
            
            if (done_) {
                return std::nullopt;
            }
            
            // The condition was met within the specified timeout
            T value = queue_.front();
            queue_.pop();
            
            // Return the popped value
            return std::optional<T>(value);
        } else {
            // Timeout occurred, and the condition was not met
            // Return an empty optional to indicate failure
            return std::nullopt;
        }
    }

    void notifyAll() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            done_ = true;
        }
        condition_.notify_all();
    }

private:
    bool          done_{false};
    std::queue<T> queue_;              // The underlying queue
    std::mutex mutex_;                 // Mutex to protect access to the queue
    std::condition_variable condition_; // Condition variable for signaling changes in the queue
};