/**
 * @file TimeModule.hpp
 * @brief Time module for Rhodesia language
 *
 * Provides date and time functionality including:
 * - Date (year, month, day)
 * - Time (hour, minute, second, millisecond)
 * - DateTime (combined date and time)
 * - Duration (time difference)
 * - Timestamp (Unix timestamp)
 */

#ifndef RHODESIA_TIME_MODULE_HPP
#define RHODESIA_TIME_MODULE_HPP

#include "RhoValue.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace Rhodesia {
namespace Time {

/**
 * @brief RhoDate - Date type (year, month, day)
 */
class RhoDate {
public:
    RhoDate() : year_(1970), month_(1), day_(1) {}
    RhoDate(int year, int month, int day)
        : year_(year), month_(month), day_(day) {
        validate();
    }

    int year() const { return year_; }
    int month() const { return month_; }
    int day() const { return day_; }

    void setYear(int year) { year_ = year; validate(); }
    void setMonth(int month) { month_ = month; validate(); }
    void setDay(int day) { day_ = day; validate(); }

    /**
     * @brief Get current date
     */
    static RhoDate now() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm* tm = std::localtime(&time_t_now);
        return RhoDate(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    }

    /**
     * @brief Format as string (YYYY-MM-DD)
     */
    std::string toString() const {
        std::ostringstream oss;
        oss << std::setfill('0')
            << std::setw(4) << year_ << "-"
            << std::setw(2) << month_ << "-"
            << std::setw(2) << day_;
        return oss.str();
    }

    /**
     * @brief Get day of week (0=Sunday, 6=Saturday)
     */
    int dayOfWeek() const {
        std::tm time = {};
        time.tm_year = year_ - 1900;
        time.tm_mon = month_ - 1;
        time.tm_mday = day_;
        std::mktime(&time);
        return time.tm_wday;
    }

    /**
     * @brief Get day of year (1-366)
     */
    int dayOfYear() const {
        std::tm time = {};
        time.tm_year = year_ - 1900;
        time.tm_mon = month_ - 1;
        time.tm_mday = day_;
        std::mktime(&time);
        return time.tm_yday + 1;
    }

private:
    int year_;
    int month_;  // 1-12
    int day_;    // 1-31

    void validate() {
        if (month_ < 1 || month_ > 12) {
            throw std::runtime_error("Invalid month: " + std::to_string(month_));
        }
        if (day_ < 1 || day_ > 31) {
            throw std::runtime_error("Invalid day: " + std::to_string(day_));
        }
        // TODO: Add more sophisticated validation for days per month
    }
};

/**
 * @brief RhoTime - Time type (hour, minute, second, millisecond)
 */
class RhoTime {
public:
    RhoTime() : hour_(0), minute_(0), second_(0), millisecond_(0) {}
    RhoTime(int hour, int minute, int second, int millisecond = 0)
        : hour_(hour), minute_(minute), second_(second), millisecond_(millisecond) {
        validate();
    }

    int hour() const { return hour_; }
    int minute() const { return minute_; }
    int second() const { return second_; }
    int millisecond() const { return millisecond_; }

    void setHour(int hour) { hour_ = hour; validate(); }
    void setMinute(int minute) { minute_ = minute; validate(); }
    void setSecond(int second) { second_ = second; validate(); }
    void setMillisecond(int ms) { millisecond_ = ms; validate(); }

    /**
     * @brief Get current time
     */
    static RhoTime now() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::tm* tm = std::localtime(&time_t_now);
        return RhoTime(tm->tm_hour, tm->tm_min, tm->tm_sec, static_cast<int>(ms.count()));
    }

    /**
     * @brief Format as string (HH:MM:SS.mmm)
     */
    std::string toString() const {
        std::ostringstream oss;
        oss << std::setfill('0')
            << std::setw(2) << hour_ << ":"
            << std::setw(2) << minute_ << ":"
            << std::setw(2) << second_;
        if (millisecond_ > 0) {
            oss << "." << std::setw(3) << millisecond_;
        }
        return oss.str();
    }

    /**
     * @brief Get total seconds since midnight
     */
    int totalSeconds() const {
        return hour_ * 3600 + minute_ * 60 + second_;
    }

    /**
     * @brief Get total milliseconds since midnight
     */
    int64_t totalMilliseconds() const {
        return static_cast<int64_t>(totalSeconds()) * 1000 + millisecond_;
    }

private:
    int hour_;         // 0-23
    int minute_;       // 0-59
    int second_;       // 0-59
    int millisecond_;  // 0-999

    void validate() {
        if (hour_ < 0 || hour_ > 23) {
            throw std::runtime_error("Invalid hour: " + std::to_string(hour_));
        }
        if (minute_ < 0 || minute_ > 59) {
            throw std::runtime_error("Invalid minute: " + std::to_string(minute_));
        }
        if (second_ < 0 || second_ > 59) {
            throw std::runtime_error("Invalid second: " + std::to_string(second_));
        }
        if (millisecond_ < 0 || millisecond_ > 999) {
            throw std::runtime_error("Invalid millisecond: " + std::to_string(millisecond_));
        }
    }
};

/**
 * @brief RhoDateTime - Combined date and time
 */
class RhoDateTime {
public:
    RhoDateTime() = default;
    RhoDateTime(const RhoDate& date, const RhoTime& time)
        : date_(date), time_(time) {}
    RhoDateTime(int year, int month, int day, int hour, int minute, int second)
        : date_(year, month, day), time_(hour, minute, second) {}

    const RhoDate& date() const { return date_; }
    const RhoTime& time() const { return time_; }

    RhoDate& date() { return date_; }
    RhoTime& time() { return time_; }

    /**
     * @brief Get current date and time
     */
    static RhoDateTime now() {
        return RhoDateTime(RhoDate::now(), RhoTime::now());
    }

    /**
     * @brief Format as ISO 8601 string (YYYY-MM-DDTHH:MM:SS)
     */
    std::string toString() const {
        return date_.toString() + "T" + time_.toString();
    }

    /**
     * @brief Convert to Unix timestamp (seconds since epoch)
     */
    int64_t toTimestamp() const {
        std::tm time = {};
        time.tm_year = date_.year() - 1900;
        time.tm_mon = date_.month() - 1;
        time.tm_mday = date_.day();
        time.tm_hour = time_.hour();
        time.tm_min = time_.minute();
        time.tm_sec = time_.second();
        return static_cast<int64_t>(std::mktime(&time));
    }

    /**
     * @brief Create from Unix timestamp
     */
    static RhoDateTime fromTimestamp(int64_t timestamp) {
        time_t time_val = static_cast<time_t>(timestamp);
        std::tm* tm = std::localtime(&time_val);
        return RhoDateTime(
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec
        );
    }

private:
    RhoDate date_;
    RhoTime time_;
};

/**
 * @brief RhoDuration - Time duration/difference
 */
class RhoDuration {
public:
    RhoDuration() : milliseconds_(0) {}
    explicit RhoDuration(int64_t milliseconds) : milliseconds_(milliseconds) {}

    /**
     * @brief Create duration from days
     */
    static RhoDuration fromDays(double days) {
        return RhoDuration(static_cast<int64_t>(days * 24 * 60 * 60 * 1000));
    }

    /**
     * @brief Create duration from hours
     */
    static RhoDuration fromHours(double hours) {
        return RhoDuration(static_cast<int64_t>(hours * 60 * 60 * 1000));
    }

    /**
     * @brief Create duration from minutes
     */
    static RhoDuration fromMinutes(double minutes) {
        return RhoDuration(static_cast<int64_t>(minutes * 60 * 1000));
    }

    /**
     * @brief Create duration from seconds
     */
    static RhoDuration fromSeconds(double seconds) {
        return RhoDuration(static_cast<int64_t>(seconds * 1000));
    }

    /**
     * @brief Get total milliseconds
     */
    int64_t totalMilliseconds() const { return milliseconds_; }

    /**
     * @brief Get total seconds
     */
    double totalSeconds() const { return milliseconds_ / 1000.0; }

    /**
     * @brief Get total minutes
     */
    double totalMinutes() const { return milliseconds_ / (60.0 * 1000.0); }

    /**
     * @brief Get total hours
     */
    double totalHours() const { return milliseconds_ / (60.0 * 60.0 * 1000.0); }

    /**
     * @brief Get total days
     */
    double totalDays() const { return milliseconds_ / (24.0 * 60.0 * 60.0 * 1000.0); }

    /**
     * @brief Format as string
     */
    std::string toString() const {
        std::ostringstream oss;
        int64_t ms = std::abs(milliseconds_);

        if (milliseconds_ < 0) oss << "-";

        int64_t days = ms / (24 * 60 * 60 * 1000);
        ms %= (24 * 60 * 60 * 1000);
        int64_t hours = ms / (60 * 60 * 1000);
        ms %= (60 * 60 * 1000);
        int64_t minutes = ms / (60 * 1000);
        ms %= (60 * 1000);
        int64_t seconds = ms / 1000;
        int64_t millis = ms % 1000;

        if (days > 0) oss << days << "d ";
        if (hours > 0) oss << hours << "h ";
        if (minutes > 0) oss << minutes << "m ";
        if (seconds > 0 || millis > 0) {
            oss << seconds;
            if (millis > 0) oss << "." << std::setfill('0') << std::setw(3) << millis;
            oss << "s";
        }

        std::string result = oss.str();
        if (result.empty()) result = "0s";
        return result;
    }

    /**
     * @brief Add durations
     */
    RhoDuration operator+(const RhoDuration& other) const {
        return RhoDuration(milliseconds_ + other.milliseconds_);
    }

    /**
     * @brief Subtract durations
     */
    RhoDuration operator-(const RhoDuration& other) const {
        return RhoDuration(milliseconds_ - other.milliseconds_);
    }

private:
    int64_t milliseconds_;
};

/**
 * @brief RhoTimestamp - Unix timestamp wrapper
 */
class RhoTimestamp {
public:
    RhoTimestamp() : seconds_(0) {}
    explicit RhoTimestamp(int64_t seconds) : seconds_(seconds) {}

    /**
     * @brief Get current timestamp
     */
    static RhoTimestamp now() {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        return RhoTimestamp(seconds.count());
    }

    /**
     * @brief Get timestamp value in seconds
     */
    int64_t seconds() const { return seconds_; }

    /**
     * @brief Convert to DateTime
     */
    RhoDateTime toDateTime() const {
        return RhoDateTime::fromTimestamp(seconds_);
    }

    /**
     * @brief Format as string (seconds since epoch)
     */
    std::string toString() const {
        return std::to_string(seconds_);
    }

    /**
     * @brief Difference between two timestamps
     */
    RhoDuration operator-(const RhoTimestamp& other) const {
        return RhoDuration::fromSeconds(static_cast<double>(seconds_ - other.seconds_));
    }

private:
    int64_t seconds_;
};

} // namespace Time
} // namespace Rhodesia

#endif // RHODESIA_TIME_MODULE_HPP
