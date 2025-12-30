#pragma once
#include <iostream>
#include <string>
#include <fstream>

namespace gore {
    enum class LogType {
        CONSOLE,
        FILE_LOG,
        BOTH,
        NONE
    };
    class logger {
        private:
            LogType logType;
            std::ofstream logFile;
            std::string file_name;
        public:
            logger () {
                logType = LogType::NONE;
            }
            logger(LogType type, const std::string& filePath = "") : logType(type) {
                file_name = filePath;
                if (logType == LogType::FILE_LOG || logType == LogType::BOTH) {
                    logFile.open(filePath, std::ios::app);
                    if (!logFile.is_open()) {
                        throw std::runtime_error("Failed to open log file: " + filePath);
                    }
                }
            }
            ~logger() {
                if (logFile.is_open()) {
                    logFile.close();
                }
            }
            // copy
            logger (const logger& l) {
                this->logType = l.logType;
                this->file_name = l.file_name;
                if (l.logFile.is_open()) {
                    this->logFile.open(l.file_name, std::ios::app);
                }
            }
            // move
            logger (logger&& l) {
                this->file_name = l.file_name;
                this->logType = l.logType;
                this->logFile = std::move(l.logFile);
            }
            // operators
            logger& operator= (const logger& l) {
                if (this != &l) {
                    this->logType = l.logType;
                    this->file_name = l.file_name;
                    if (l.logFile.is_open()) {
                        this->logFile.open(l.file_name, std::ios::app);
                    }
                }
                return *this;
            }
            logger& operator= (logger&& l) {
                if (this != &l) {
                    this->file_name = l.file_name;
                    this->logType = l.logType;
                    this->logFile = std::move(l.logFile);
                }
                return *this;
            }

            void log(const std::string& message) {
                if (logType == LogType::NONE) {
                    return;
                }
                if (logType == LogType::CONSOLE || logType == LogType::BOTH) {
                    std::cout << message << "\n";
                }
                if ((logType == LogType::FILE_LOG || logType == LogType::BOTH) && logFile.is_open()) {
                    logFile << message << "\n";
                }
            }
            void flush() {
                switch (logType) {
                    case LogType::NONE:
                        return;
                    case LogType::CONSOLE:
                        std::cout.flush();
                        break;
                    case LogType::FILE_LOG:
                        if (logFile.is_open()) {
                            logFile.flush();
                        }
                        break;
                    case LogType::BOTH:
                        std::cout.flush();
                        if (logFile.is_open()) {
                            logFile.flush();
                        }
                        break;
                }
            }
    };
}