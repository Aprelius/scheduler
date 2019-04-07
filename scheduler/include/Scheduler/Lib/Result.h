#pragma once

#include <memory>

namespace Scheduler {
namespace Lib {

    class Task;

    class Result;
    typedef std::shared_ptr<Result> ResultPtr;

    class Result : public std::enable_shared_from_this<Result>
    {};

    class ResultSet;
    typedef std::shared_ptr<ResultSet> ResultSetPtr;

    class ResultSet : public std::enable_shared_from_this<ResultSet>
    { };

}  // namespace Lib
}  // namespace Scheduler
