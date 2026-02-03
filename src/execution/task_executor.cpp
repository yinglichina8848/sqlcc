#include "task_executor.h"
#include "core/execution_context.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <ctime>

namespace sqlcc {

TaskExecutor::TaskExecutor()
    : running_(false), max_concurrent_tasks_(10), task_timeout_(std::chrono::seconds(30)),
      next_task_id_(1) {
}

TaskExecutor::~TaskExecutor() {
    shutdown();
}

bool TaskExecutor::initialize(int num_threads /*= 4*/) {
    if (running_.load()) {
        return false; // 已初始化
    }

    running_.store(true);

    // 创建工作线程
    for (int i = 0; i < num_threads; ++i) {
        worker_threads_.emplace_back([this] { workerThread(); });
    }

    return true;
}

void TaskExecutor::shutdown() {
    if (!running_.exchange(false)) {
        return; // 已经关闭
    }

    // 通知所有工作线程停止
    queue_cv_.notify_all();

    // 等待所有工作线程完成
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // 清空任务队列
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!task_queue_.empty()) {
        task_queue_.pop();
    }

    // 清空任务状态
    std::lock_guard<std::mutex> status_lock(status_mutex_);
    task_statuses_.clear();
}

int TaskExecutor::submitTask(std::unique_ptr<Task> task) {
    if (!running_.load()) {
        return -1; // 执行器已停止
    }

    int task_id = next_task_id_.fetch_add(1);

    // 创建任务项并添加到队列
    TaskItem task_item;
    task_item.task_id = task_id;
    task_item.task = std::move(task);
    task_item.submit_time = std::chrono::steady_clock::now();

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_queue_.push(std::move(task_item));
    }

    // 通知工作线程
    queue_cv_.notify_one();

    // 记录任务状态
    {
        std::lock_guard<std::mutex> status_lock(status_mutex_);
        task_statuses_[task_id] = TaskStatus::PENDING;
    }

    return task_id;
}

int TaskExecutor::submitTask(std::function<void(ExecutionContext&)> func,
                           const std::string& description,
                           TaskPriority priority /*= TaskPriority::NORMAL*/) {
    // 简单实现：创建一个包装 func 的 Task 类
    class FunctionTask : public Task {
    public:
        FunctionTask(const std::string& description, std::function<void(ExecutionContext&)> func)
            : description_(description), func_(std::move(func)) {}

        std::shared_ptr<TaskResult> execute() override {
            auto result = std::make_shared<TaskResult>(true, TaskType::UNKNOWN);
            try {
                // 简单的执行上下文
                ExecutionContext context;
                func_(context);
                result->set_result_message(description_ + " executed successfully");
            } catch (const std::exception& e) {
                result->set_success(false);
                result->set_error_message(e.what());
            }
            return result;
        }

        std::string getDescription() const override {
            return description_;
        }

    private:
        std::string description_;
        std::function<void(ExecutionContext&)> func_;
    };

    auto task = std::make_unique<FunctionTask>(description, std::move(func));
    return submitTask(std::move(task));
}

bool TaskExecutor::cancelTask(int task_id) {
    std::lock_guard<std::mutex> status_lock(status_mutex_);
    auto it = task_statuses_.find(task_id);

    if (it != task_statuses_.end() && it->second == TaskStatus::PENDING) {
        it->second = TaskStatus::CANCELLED;
        return true;
    }

    return false;
}

TaskStatus TaskExecutor::getTaskStatus(int task_id) const {
    std::lock_guard<std::mutex> status_lock(status_mutex_);
    auto it = task_statuses_.find(task_id);

    return (it != task_statuses_.end()) ? it->second : TaskStatus::CANCELLED;
}

size_t TaskExecutor::getPendingTaskCount() const {
    std::lock_guard<std::mutex> status_lock(status_mutex_);
    return std::count_if(task_statuses_.begin(), task_statuses_.end(),
                        [](const auto& pair) { return pair.second == TaskStatus::PENDING; });
}

size_t TaskExecutor::getRunningTaskCount() const {
    std::lock_guard<std::mutex> status_lock(status_mutex_);
    return std::count_if(task_statuses_.begin(), task_statuses_.end(),
                        [](const auto& pair) { return pair.second == TaskStatus::RUNNING; });
}

ExecutionResult TaskExecutor::waitForTask(int task_id) {
    // 简单实现：轮询任务状态
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
        auto status = getTaskStatus(task_id);
        if (status == TaskStatus::COMPLETED || status == TaskStatus::FAILED || status == TaskStatus::CANCELLED) {
            return ExecutionResult(); // 返回空结果
        }

        // 超时检查
        if (std::chrono::steady_clock::now() - start_time > task_timeout_) {
            cancelTask(task_id);
            return ExecutionResult();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

ExecutionResult TaskExecutor::waitForAllTasks() {
    // 简单实现：等待所有任务完成
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
        if (getPendingTaskCount() == 0 && getRunningTaskCount() == 0) {
            return ExecutionResult();
        }

        if (std::chrono::steady_clock::now() - start_time > task_timeout_) {
            return ExecutionResult();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void TaskExecutor::setMaxConcurrentTasks(size_t max_tasks) {
    max_concurrent_tasks_ = max_tasks;
}

void TaskExecutor::setTaskTimeout(std::chrono::seconds timeout) {
    task_timeout_ = timeout;
}

void TaskExecutor::workerThread() {
    while (running_.load()) {
        TaskItem task_item;
        bool has_task = false;

        // 等待任务
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] {
                return !running_.load() || !task_queue_.empty();
            });

            if (!running_.load()) {
                break;
            }

            if (!task_queue_.empty()) {
                TaskItem task_item_temp = std::move(const_cast<TaskItem&>(task_queue_.top()));
                task_queue_.pop();
                task_item = std::move(task_item_temp);
                has_task = true;
            }
        }

        if (has_task) {
            // 检查任务是否已取消
            {
                std::lock_guard<std::mutex> status_lock(status_mutex_);
                auto it = task_statuses_.find(task_item.task_id);
                if (it != task_statuses_.end() && it->second == TaskStatus::CANCELLED) {
                    continue;
                }
                it->second = TaskStatus::RUNNING;
            }

            // 执行任务
            try {
                processTask(task_item);

                // 标记任务完成
                std::lock_guard<std::mutex> status_lock(status_mutex_);
                auto it = task_statuses_.find(task_item.task_id);
                if (it != task_statuses_.end()) {
                    it->second = TaskStatus::COMPLETED;
                }
            } catch (const std::exception& e) {
                std::cerr << "Task " << task_item.task_id << " failed: " << e.what() << std::endl;

                std::lock_guard<std::mutex> status_lock(status_mutex_);
                auto it = task_statuses_.find(task_item.task_id);
                if (it != task_statuses_.end()) {
                    it->second = TaskStatus::FAILED;
                }
            }
        }
    }
}

void TaskExecutor::processTask(TaskItem& task_item) {
    // 执行任务
    auto result = task_item.task->execute();

    // 简单的结果处理
    if (result) {
        std::string task_desc = task_item.task->getDescription();
        if (result->get_success()) {
            // 任务成功
            if (!task_desc.empty()) {
                std::cout << "Task completed: " << task_desc << std::endl;
            }
        } else {
            // 任务失败
            std::cerr << "Task failed";
            if (!task_desc.empty()) {
                std::cerr << ": " << task_desc;
            }
            if (!result->get_error_message().empty()) {
                std::cerr << ", Error: " << result->get_error_message();
            }
            std::cerr << std::endl;
        }
    }
}

} // namespace sqlcc
