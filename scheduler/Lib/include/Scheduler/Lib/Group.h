#pragma once

#include <Scheduler/Lib/Chain.h>

namespace Scheduler {
namespace Lib {

    class Group;
    typedef std::shared_ptr<Group> GroupPtr;

    class Group : public Chain
    {
        template<typename T, typename... Args>
        friend std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
        After(const Clock::time_point& point, Args&& ...args);

        template<typename T, typename... Args>
        friend std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
        Before(const Clock::time_point& point, Args&& ...args);

        template<typename T, typename... Args>
        friend std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
        Between(
            const Clock::time_point& after,
            const Clock::time_point& before,
            Args&& ...args);

        template<typename T, typename... Args>
        friend std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
        MakeTask(Args&& ...args);

    public:
        ~Group();

        /// Add a linked child task to the chain. If the chain is already
        /// active or complete then the Add will be skipped. It is always a
        /// good idea to verify that a child was successfully added with the
        /// IsChild call.
        Group* Add(Task* task) override;

        /// Add a linked child task to the chain. If the chain is already
        /// active or complete then the Add will be skipped. It is always a
        /// good idea to verify that a child was successfully added with the
        /// IsChild call.
        Group* Add(TaskPtr& task) override { return Add(task.get()); }

        /// The name of the class as it should appear in ToString. Useful
        /// for implementing classes which only need to change the instance
        /// name.
        const char* Instance() const override { return "Group"; }

        GroupPtr shared_from_this();

    protected:
        Group();

        template<typename... Args,
            typename = typename std::enable_if<
                are_all_convertible<TaskPtr, Args...>::value>::type>
        Group(
            const Clock::time_point& after,
            const Clock::time_point& before,
            Args&& ...args)
            : Chain(after, before)
        {
            GetChildren().reserve(PackUtils<Args...>::size);
            PackUtils<Args...>::ForEach([&](auto& task){
                this->Add(task);
            }, std::forward<Args>(args)...);
        }

        template<typename... Args,
            typename = typename std::enable_if<
                are_all_convertible<TaskPtr, Args...>::value>::type>
        Group(Args&& ...args) : Chain()
        {
            GetChildren().reserve(PackUtils<Args...>::size);
            PackUtils<Args...>::ForEach([&](auto& task){
                this->Add(task);
            }, std::forward<Args>(args)...);
        }
    };

}  // namespace Lib
}  // namespace Scheduler
