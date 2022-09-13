#ifndef _LOG_LOG_HPP_
#define _LOG_LOG_HPP_
#include <thread>
#include <sstream>
#include <atomic>
#include <regex>
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <iostream>
#include <fstream>


// Logging API:

//! Logs an info message. Disable it through Log::SetVerbosity, define LOG_NO_INFO, or being in a release branch
#define logInfo(cat, msg) logInfo_(cat, msg)
//! Logs a warning. Disable reporting through Log::SetVerbosity or define LOG_NO_WARNING
#define logWarning(cat, msg) logWarning_(cat, msg)
//! Logs an error. Disable reporting through define LOG_NO_ERROR
#define logError(cat, msg) logError_(cat, msg)

template<class T>
class DBQueue
{

public:

    DBQueue()
            : mForegroundQueue(&mQueueAlpha)
            , mBackgroundQueue(&mQueueBeta)
    {
    }

    //! Clears foreground queue and swaps queues.
    void Swap()
    {
        std::unique_lock<std::mutex> fgGuard(mForegroundMutex);
        std::unique_lock<std::mutex> bgGuard(mBackgroundMutex);

        // Clear the foreground queue.
        std::queue<T>().swap(*mForegroundQueue);

        auto* swap       = mBackgroundQueue;
        mBackgroundQueue = mForegroundQueue;
        mForegroundQueue = swap;
    }

    //! Pushes to the background queue. Copy constructor.
    void Push(
            const T& item)
    {
        std::unique_lock<std::mutex> guard(mBackgroundMutex);
        mBackgroundQueue->push(item);
    }

    //! Pushes to the background queue. Move constructor.
    void Push(
            T&& item)
    {
        std::unique_lock<std::mutex> guard(mBackgroundMutex);
        mBackgroundQueue->push(std::move(item));
    }

    //! Returns a reference to the front element
    //! in the foregrund queue.
    T& Front()
    {
        std::unique_lock<std::mutex> guard(mForegroundMutex);
        return mForegroundQueue->front();
    }

    const T& Front() const
    {
        std::unique_lock<std::mutex> guard(mForegroundMutex);
        return mForegroundQueue->front();
    }

    //! Pops from the foreground queue.
    void Pop()
    {
        std::unique_lock<std::mutex> guard(mForegroundMutex);
        mForegroundQueue->pop();
    }

    //! Return the front element in the foreground queue by moving it and erase it from the queue.
    T FrontAndPop()
    {
        std::unique_lock<std::mutex> guard(mForegroundMutex);

        // Get value by moving the internal queue reference to a new value
        T value = std::move(mForegroundQueue->front());
        // At this point mForegroundQueue contains a non valid element, but mutex is taken and next instruction erase it

        // Pop value from queue
        mForegroundQueue->pop();

        // Return value (as it has been created in this scope, it will not be copied but moved or directly forwarded)
        return value;
    }

    //! Reports whether the foreground queue is empty.
    bool Empty() const
    {
        std::unique_lock<std::mutex> guard(mForegroundMutex);
        return mForegroundQueue->empty();
    }

    //! Reports whether the both queues are empty.
    bool BothEmpty() const
    {
        std::unique_lock<std::mutex> guard(mForegroundMutex);
        std::unique_lock<std::mutex> bgGuard(mBackgroundMutex);
        return mForegroundQueue->empty() && mBackgroundQueue->empty();
    }

    //! Reports the size of the foreground queue.
    size_t Size() const
    {
        std::unique_lock<std::mutex> guard(mForegroundMutex);
        return mForegroundQueue->size();
    }

    //! Clears foreground and background.
    void Clear()
    {
        std::unique_lock<std::mutex> fgGuard(mForegroundMutex);
        std::unique_lock<std::mutex> bgGuard(mBackgroundMutex);
        std::queue<T>().swap(*mForegroundQueue);
        std::queue<T>().swap(*mBackgroundQueue);
    }

private:

    // Underlying queues
    std::queue<T> mQueueAlpha;
    std::queue<T> mQueueBeta;

    // Front and background queue references (double buffering)
    std::queue<T>* mForegroundQueue;
    std::queue<T>* mBackgroundQueue;

    mutable std::mutex mForegroundMutex;
    mutable std::mutex mBackgroundMutex;
};


class LogConsumer;

class Log
{
public:

    enum Kind
    {
        Error,
        Warning,
        Info,
    };

    /**
     * Registers an user defined consumer to route log output.
     * There is a default stdout consumer active as default.
     * @param consumer r-value to a consumer unique_ptr. It will be invalidated after the call.
     */
     static void RegisterConsumer(
            std::unique_ptr<LogConsumer>&& consumer);

    //! Removes all registered consumers, including the default stdout.
     static void ClearConsumers();

    //! Enables the reporting of filenames in log entries. Disabled by default.
     static void ReportFilenames(
            bool);

    //! Enables the reporting of function names in log entries. Enabled by default when supported.
     static void ReportFunctions(
            bool);

    //! Sets the verbosity level, allowing for messages equal or under that priority to be logged.
     static void SetVerbosity(
            Log::Kind);

    //! Returns the current verbosity level.
     static Log::Kind GetVerbosity();

    //! Sets a filter that will pattern-match against log categories, dropping any unmatched categories.
     static void SetCategoryFilter(
            const std::regex&);

    //! Sets a filter that will pattern-match against filenames, dropping any unmatched categories.
     static void SetFilenameFilter(
            const std::regex&);

    //! Sets a filter that will pattern-match against the provided error string, dropping any unmatched categories.
     static void SetErrorStringFilter(
            const std::regex&);

    //! Returns the logging engine to configuration defaults.
     static void Reset();

    //! Waits until all info logged up to the call time is consumed
     static void Flush();

    //! Stops the logging thread. It will re-launch on the next call to a successful log macro.
    static void KillThread();

    // Note: In VS2013, if you're linking this class statically, you will have to call KillThread before leaving
    // main, due to an unsolved MSVC bug.

    struct Context
    {
        const char* filename;
        int line;
        const char* function;
        const char* category;
    };

    struct Entry
    {
        std::string message;
        Log::Context context;
        Log::Kind kind;
        std::string timestamp;
    };

    /**
     * Not recommended to call this method directly! Use the following macros:
     *  * logInfo(cat, msg);
     *  * logWarning(cat, msg);
     *  * logError(cat, msg);
     */
     static void QueueLog(
            const std::string& message,
            const Log::Context&,
            Log::Kind);

private:

    // Applies transformations to the entries compliant with the options selected (such as
    // erasure of certain context information, or filtering by category. Returns false
    // if the log entry is blacklisted.
    static bool preprocess(
            Entry&);

    static void run();

    static void get_timestamp(
            std::string&);
};

/**
 * Consumes a log entry to output it somewhere.
 */
class LogConsumer
{
public:

    virtual ~LogConsumer() = default;

    virtual void Consume(
            const Log::Entry&) = 0;

protected:

    void print_timestamp(
            std::ostream& stream,
            const Log::Entry&,
            bool color) const;

    void print_header(
            std::ostream& stream,
            const Log::Entry&,
            bool color) const;

     void print_context(
            std::ostream& stream,
            const Log::Entry&,
            bool color) const;

    void print_message(
            std::ostream& stream,
            const Log::Entry&,
            bool color) const;

     void print_new_line(
            std::ostream& stream,
            bool color) const;
};
class OStreamConsumer : public LogConsumer
{
public:

    virtual ~OStreamConsumer() = default;

    /** \internal
     * Called by Log to ask us to consume the Entry.
     * @param Log::Entry to consume.
     */
    void Consume(
            const Log::Entry& entry) override;

protected:

    /** \internal
     * Called by Log consume to get the correct stream
     * @param Log::Entry to consume.
     */
    virtual std::ostream& get_stream(
            const Log::Entry& entry) = 0;
};


class StdoutConsumer : public OStreamConsumer
{
public:

    //virtual ~StdoutConsumer() = default;
    virtual ~StdoutConsumer() ;

private:

    /** \internal
     * Called by Log consume to get the correct stream
     * @param Log::Entry to consume.
     */
    virtual std::ostream& get_stream(
            const Log::Entry& entry) override;
};

class StdoutErrConsumer : public OStreamConsumer
{
public:

    virtual ~StdoutErrConsumer() = default;

    /**
     * @brief Set the stderr_threshold to a Log::Kind.
     * This threshold decides which log messages are output on STDOUT, and which are output to STDERR.
     * Log messages with a Log::Kind equal to or more severe than the stderr_threshold are output to STDERR using
     * std::cerr.
     * Log messages with a Log::Kind less severe than the stderr_threshold are output to STDOUT using
     * std::cout.
     * @param kind The Log::Kind to which stderr_threshold is set.
     */
     virtual void stderr_threshold(
            const Log::Kind& kind);

    /**
     * @brief Retrieve the stderr_threshold.
     * @return The Log::Kind to which stderr_threshold is set.
     */
     virtual Log::Kind stderr_threshold() const;

    /**
     * @brief Default value of stderr_threshold.
     */
    static constexpr Log::Kind STDERR_THRESHOLD_DEFAULT = Log::Kind::Warning;

protected:

    /** \internal
     * Called by Log consume to get the correct stream
     * @param Log::Entry to consume.
     */
     virtual std::ostream& get_stream(
            const Log::Entry& entry) override;

private:

    Log::Kind stderr_threshold_ = STDERR_THRESHOLD_DEFAULT;

};

class FileConsumer : public OStreamConsumer
{
public:

    //! Default constructor: filename = "output.log", append = false.
    FileConsumer();

    /** Constructor with parameters.
     * @param filename path of the output file where the log will be wrote.
     * @param append indicates if the consumer must append the content in the filename.
     */
     FileConsumer(
            const std::string& filename,
            bool append = false);

    virtual ~FileConsumer();

private:

    /** \internal
     * Called by Log consume to get the correct stream
     * @param entry Log::Entry to consume.
     */
     virtual std::ostream& get_stream(
            const Log::Entry& entry) override;

    std::string output_file_;
    std::ofstream file_;
    bool append_;
};



#define logError_(cat, msg)                                                                                            \
    {                                                                                                                  \
        using namespace eprosima::fastdds::dds;                                                                        \
        std::stringstream fastdds_log_ss_tmp__;                                                                        \
        fastdds_log_ss_tmp__ << msg;                                                                                   \
        Log::QueueLog(fastdds_log_ss_tmp__.str(), Log::Context{__FILE__, __LINE__, __func__, #cat}, Log::Kind::Error); \
    }
// ifndef LOG_NO_ERROR


#define logWarning_(cat, msg)                                                                                       \
    {                                                                                                               \
        using namespace eprosima::fastdds::dds;                                                                     \
        if (Log::GetVerbosity() >= Log::Kind::Warning)                                                              \
        {                                                                                                           \
            std::stringstream fastdds_log_ss_tmp__;                                                                 \
            fastdds_log_ss_tmp__ << msg;                                                                            \
            Log::QueueLog(                                                                                          \
                fastdds_log_ss_tmp__.str(), Log::Context{__FILE__, __LINE__, __func__, #cat}, Log::Kind::Warning);  \
        }                                                                                                           \
    }                                                                                                               \
// ifndef LOG_NO_WARNING

#define logInfo_(cat, msg)                                                                              \
    {                                                                                                   \
                                                               \
        if (Log::GetVerbosity() >= Log::Kind::Info)                                                     \
        {                                                                                               \
            std::stringstream fastdds_log_ss_tmp__;                                                     \
            fastdds_log_ss_tmp__ << msg;                                                                \
            Log::QueueLog(fastdds_log_ss_tmp__.str(), Log::Context{__FILE__, __LINE__, __func__, #cat}, \
                    Log::Kind::Info);                                                                   \
        }                                                                                               \
    }


#endif // ifndef _FASTDDS_DDS_LOG_LOG_HPP_
