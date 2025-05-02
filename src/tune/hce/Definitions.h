#ifndef TUNE_DEFINITIONS_H
#define TUNE_DEFINITIONS_H

#include "core/chess/Board.h"
#include "tune/Simulation.h"

#include <sstream>
#include <tuple>
#include <type_traits>
#include <vector>

struct DataPoint {
    Board board;
    int result;
};

class Variable;

extern std::vector<Variable*> tuneVariables;

class Variable {
    public:
        enum class Type {
            STRING,
            BOOL,
            CHAR,
            UNSIGNED,
            SIGNED,
            FLOATING_POINT
        };

    private:
        void* value = nullptr;
        const std::string name;
        const std::string description;
        const Type type;

    public:
        template <typename T> requires(std::is_same_v<T, std::string> || std::is_same_v<T, const char*>)
        inline Variable(const std::string& name, const std::string& description, T value) : name(name), description(description), type(Type::STRING) {
            this->value = new std::string(value);
            tuneVariables.push_back(this);
        }

        template <typename T> requires(std::is_same_v<T, bool>)
        inline Variable(const std::string& name, const std::string& description, T value) : name(name), description(description), type(Type::BOOL) {
            this->value = new bool(value);
            tuneVariables.push_back(this);
        }

        template <typename T> requires(std::is_same_v<T, char>)
        inline Variable(const std::string& name, const std::string& description, T value) : name(name), description(description), type(Type::CHAR) {
            this->value = new char(value);
            tuneVariables.push_back(this);
        }

        template <typename T> requires(std::is_unsigned_v<T> && !(std::is_same_v<T, bool> || std::is_same_v<T, char>))
        inline Variable(const std::string& name, const std::string& description, T value) : name(name), description(description), type(Type::UNSIGNED) {
            this->value = new unsigned long long(value);
            tuneVariables.push_back(this);
        }

        template <typename T> requires(std::is_signed_v<T> && !(std::is_same_v<T, bool> || std::is_same_v<T, char> || std::is_floating_point_v<T>))
        inline Variable(const std::string& name, const std::string& description, T value) : name(name), description(description), type(Type::SIGNED) {
            this->value = new long long(value);
            tuneVariables.push_back(this);
        }

        template <typename T> requires(std::is_floating_point_v<T>)
        inline Variable(const std::string& name, const std::string& description, T value) : name(name), description(description), type(Type::FLOATING_POINT) {
            this->value = new double(value);
            tuneVariables.push_back(this);
        }

        inline ~Variable() {
            if(type == Type::STRING)
                delete (std::string*)value;
            else if(type == Type::BOOL)
                delete (bool*)value;
            else if(type == Type::CHAR)
                delete (char*)value;
            else if(type == Type::UNSIGNED)
                delete (unsigned long long*)value;
            else if(type == Type::SIGNED)
                delete (long long*)value;
            else if(type == Type::FLOATING_POINT)
                delete (double*)value;
        }

        Variable(const Variable&) = delete;
        Variable& operator=(const Variable&) = delete;
        inline Variable(Variable&&) = delete;
        Variable& operator=(Variable&&) = delete;

        inline const std::string& getName() const { return name; }
        inline const std::string& getDescription() const { return description; }
        inline Type getType() const { return type; }

        inline void set(const std::string& value) {
            if(type == Type::STRING)
                *(std::string*)this->value = value;
            else if(type == Type::BOOL)
                *(bool*)this->value = value == "true" || value == "1" || value == "True" || value == "TRUE";
            else if(type == Type::CHAR)
                *(char*)this->value = value[0];
            else if(type == Type::UNSIGNED)
                *(unsigned long long*)this->value = std::stoull(value);
            else if(type == Type::SIGNED)
                *(long long*)this->value = std::stoll(value);
            else if(type == Type::FLOATING_POINT)
                *(double*)this->value = std::stod(value);
        }

        inline Variable& operator=(const std::string& value) {
            set(value);
            return *this;
        }

        inline std::string getAsString() {
            if(type == Type::STRING)
                return *(std::string*)value;
            else if(type == Type::BOOL)
                return *(bool*)value ? "true" : "false";
            else if(type == Type::CHAR)
                return std::string(1, *(char*)value);
            else if(type == Type::UNSIGNED)
                return std::to_string(*(unsigned long long*)value);
            else if(type == Type::SIGNED)
                return std::to_string(*(long long*)value);
            else if(type == Type::FLOATING_POINT) {
                std::stringstream result;
                result << *(double*)value;
                return result.str();
            } else
                throw std::invalid_argument("Invalid type");
        }

        template <typename T> requires(std::is_same_v<T, std::string> || std::is_same_v<T, bool> || std::is_same_v<T, char> ||
                                       std::is_unsigned_v<T> || std::is_signed_v<T> || std::is_floating_point_v<T>)
        inline T get() {
            if constexpr(std::is_same_v<T, std::string>)
                return getAsString();
            else if constexpr(std::is_same_v<T, bool>) {
                switch(type) {
                    case Type::STRING:
                        return *(std::string*)value == "true" || *(std::string*)value == "1" || *(std::string*)value == "True" || *(std::string*)value == "TRUE";
                    case Type::BOOL:
                        return *(bool*)value;
                    case Type::CHAR:
                        return *(char*)value == '1';
                    case Type::UNSIGNED:
                        return *(unsigned long long*)value != 0;
                    case Type::SIGNED:
                        return *(long long*)value != 0;
                    case Type::FLOATING_POINT:
                        return *(double*)value != 0.0;
                    default:
                        throw std::invalid_argument("Invalid type conversion");
                }
            } else if constexpr(std::is_same_v<T, char>) {
                switch(type) {
                    case Type::STRING:
                        return ((std::string*)value)->c_str()[0];
                    case Type::BOOL:
                        return *(bool*)value ? '1' : '0';
                    case Type::CHAR:
                        return *(char*)value;
                    case Type::UNSIGNED:
                        return (char)*(unsigned long long*)value;
                    case Type::SIGNED:
                        return (char)*(long long*)value;
                    case Type::FLOATING_POINT:
                        return (char)*(double*)value;
                    default:
                        throw std::invalid_argument("Invalid type conversion");
                }
            } else if constexpr(std::is_unsigned_v<T>) {
                switch(type) {
                    case Type::STRING:
                        return (T)std::stoull(*(std::string*)value);
                    case Type::BOOL:
                        return *(bool*)value ? (T)1 : (T)0;
                    case Type::CHAR:
                        return (T)(*(char*)value);
                    case Type::UNSIGNED:
                        return (T)*(unsigned long long*)value;
                    case Type::SIGNED:
                        return (T)*(long long*)value;
                    case Type::FLOATING_POINT:
                        return (T)*(double*)value;
                    default:
                        throw std::invalid_argument("Invalid type conversion");
                }
            } else if constexpr(std::is_signed_v<T>) {
                switch(type) {
                    case Type::STRING:
                        return (T)std::stoll(*(std::string*)value);
                    case Type::BOOL:
                        return *(bool*)value ? (T)1 : (T)0;
                    case Type::CHAR:
                        return (T)(*(char*)value);
                    case Type::UNSIGNED:
                        return (T)*(unsigned long long*)value;
                    case Type::SIGNED:
                        return (T)*(long long*)value;
                    case Type::FLOATING_POINT:
                        return (T)*(double*)value;
                    default:
                        throw std::invalid_argument("Invalid type conversion");
                }
            } else if constexpr(std::is_floating_point_v<T>) {
                switch(type) {
                    case Type::STRING:
                        return (T)std::stod(*(std::string*)value);
                    case Type::BOOL:
                        return *(bool*)value ? (T)1.0 : (T)0.0;
                    case Type::CHAR:
                        return (T)(*(char*)value);
                    case Type::UNSIGNED:
                        return (T)*(unsigned long long*)value;
                    case Type::SIGNED:
                        return (T)*(long long*)value;
                    case Type::FLOATING_POINT:
                        return (T)*(double*)value;
                    default:
                        throw std::invalid_argument("Invalid type conversion");
                }
            }
        }

        inline std::string getTypeString() {
            if(type == Type::STRING)
                return "string";
            else if(type == Type::BOOL)
                return "bool";
            else if(type == Type::CHAR)
                return "char";
            else if(type == Type::UNSIGNED)
                return "unsigned";
            else if(type == Type::SIGNED)
                return "signed";
            else if(type == Type::FLOATING_POINT)
                return "floating_point";
            else
                throw std::invalid_argument("Invalid type");
        }
};

/**
 * Dateipfade.
 */

extern Variable pgnFilePath;
extern Variable samplesFilePath;

/**
 * Variablen der Trainingsdatengenerierung.
 */

extern Variable numThreads;
extern Variable numGames;
extern Variable numGamesIncrement;
extern Variable timeControl;
extern Variable increment;
extern Variable timeGrowth;
extern Variable openingBookMovesMin;
extern Variable openingBookMovesMax;
extern Variable randomMovesMin;
extern Variable randomMovesMax;
extern Variable useNoisyParameters;
extern Variable noiseDefaultStdDev;
extern Variable noiseLinearStdDev;
extern Variable noiseDecay;

/**
 * Variablen des Trainings.
 */

extern Variable validationSplit;
extern Variable k;
extern Variable learningRate;
extern Variable learningRateDecay;
extern Variable numEpochs;
extern Variable numEpochsIncrement;
extern Variable numGenerations;
extern Variable noImprovementPatience;
extern Variable batchSize;
extern Variable epsilon;
extern Variable discount;
extern Variable alpha;
extern Variable beta1;
extern Variable beta2;
extern Variable weightDecay;

#endif