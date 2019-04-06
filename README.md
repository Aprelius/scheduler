# Task Scheduler

A little insight here. I have been thinking about smart task schedulers for a long time. Using Celery as an influence I started playing around with an interface for how I could accomplish something similar in C++. Python is really a beautiful language for something like this, but I like compilers and the piece of mind they bring.

The core concept of this code if for an intelligent dependency graph between tasks that can be determined at runtime. The situation in a real environment is highly fluid and needs the ability to change on depend, to adapt as new parameters or work is discovered. Every task scheduler I have looked at which defines workflows in code is limited to that concept.

What happens when a task is long running or waiting for another resource to become active? What happens when a task fails and requires intelligent handling? What about a long running task that can be run in parallel with other tasks, but should wait for a sync point between multiple tasks? ALl of these are cases I have discovered professionally, and so this library came to exist.

## Examples

The best thing you can do is look at the unit tests. There are a few different primitives which can be used to build concepts or workflows.

### Task

The Task is the lowest level unit of schedulable work. It can accept a lambda or be extended for extra functionality. At its core the Task is the basis of all other workflows. By composing Tasks into a flow, a dependency graph is created which is scheduled accordingly.

A Task is fundamentally about executing work now, later, or after a certain time has elapsed.

### Chain

A Chain is a composed set of Tasks which are executed sequentially. Tasks can be combined into a Chain and scheduled as a single unit. When executed any failure in the Chain automatically halts processing and fails the Chain along with the remaining tasks in the flow.

### Group

Like a Chain, a Group is a set of tasks which can be scheduled as a group, but executed in parallel. When executed, any failure will result in the Group and all remaining Tasks marked as a failure.

### Workflows

Workflows are all about composing the basic units. A Chain can depend on a Group completing and vice versa. Chain a task to a Group to automatically execute a cleanup operation after a Group completes.

## The Name

Note, the name sucks. The hardest thing we do as software engineers is come up with names. I will rename this to something more fitting one day.