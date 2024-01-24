#ifndef OPTIONS_H
#define OPTIONS_H

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * Diese Datei definiert Klassen zur Speicherung und
 * Verwaltung von UCI-Optionen.
 */

namespace UCI {
    /**
     * @brief Typen von UCI-Optionen.
     * 
     * Es gibt zwei Typen von UCI-Optionen:
     * Check-Optionen, die entweder an oder aus sind
     * und Spin-Optionen, die einen Wert zwischen
     * einem Minimum und einem Maximum haben.
     */
    enum class OptionType {
        Check,
        Spin
    };

    /**
     * @brief Eine UCI-Option mit Name, Wert, Typ und Wertebereich (bei Spin-Optionen).
     * 
     * Die Werte und der Wertebereich werden als Strings gespeichert.
     * Die Klasse bietet Methoden, um die Werte als numerischen Typ bzw. bool
     * zu lesen und zu schreiben.
     */
    class Option {
        private:
            std::string name;
            std::string value;
            OptionType type;
            std::string minValue;
            std::string maxValue;

            std::function<void(std::string)> onChange;

            /**
             * @brief Konvertiert einen numerischen Typ bzw. bool in einen String.
             * 
             * @tparam T Der Typ des Wertes.
             * @param value Der Wert.
             * @return Der Wert als String.
             */
            template <typename T>
            inline std::string toString(T value) const {
                return std::to_string(value);
            }

            /**
             * @brief Konvertiert einen String in einen numerischen Typ bzw. bool.
             * 
             * @tparam T Der Typ des Wertes.
             * @param value Der Wert als String.
             * @return Der Wert.
             */
            template <typename T>
            inline T fromString(std::string value) const {
                return T(value);
            }

        public:
            /**
             * @brief Konstruktor für eine Spin-Option.
             * 
             * @param name Der Name der Option.
             * @param value Der Standardwert der Option.
             * @param minValue Untere Grenze des Wertebereichs.
             * @param maxValue Obere Grenze des Wertebereichs.
             * @param onChange Callback, der aufgerufen wird, wenn sich der Wert ändert.
             */
            Option(std::string name, std::string value,
                   std::string minValue, std::string maxValue,
                   std::function<void(std::string)> onChange = nullptr) :
                    name(name), value(value), type(OptionType::Spin),
                    minValue(minValue), maxValue(maxValue),
                    onChange(onChange) {}

            /**
             * @brief Konstruktor für eine Check-Option.
             * 
             * @param name Der Name der Option.
             * @param value Der Standardwert der Option.
             * @param onChange Callback, der aufgerufen wird, wenn sich der Wert ändert.
             */
            Option(std::string name, std::string value,
                   std::function<void(std::string)> onChange = nullptr) :
                    name(name), value(value), type(OptionType::Check),
                    minValue("false"), maxValue("true"),
                    onChange(onChange) {}

            /**
             * @brief Getter für den Namen der Option.
             */
            inline std::string getName() const { return name; }

            /**
             * @brief Getter für den Wert der Option.
             * 
             * @tparam T Der Typ, in den der Wert konvertiert werden soll.
             * @return Der Wert.
             */
            template <typename T>
            inline T getValue() const {
                return fromString<T>(value);
            }

            /**
             * @brief Setter für den Wert der Option.
             * 
             * @tparam T Der Typ des Parameters.
             * @param value Der neue Wert.
             */
            template <typename T>
            inline void setValue(T value) {
                T min = fromString<T>(minValue);
                T max = fromString<T>(maxValue);

                this->value = toString(std::clamp<T>(value, min, max));

                if(onChange)
                    onChange(this->value);
            }

            /**
             * @brief Zuweisungsoperator für den Wert der Option.
             * Hat dieselbe Funktionalität wie setValue.
             * 
             * @tparam T Der Typ des Parameters.
             * @param value Der neue Wert.
             * @return Referenz auf this.
             */
            template <typename T>
            inline Option& operator=(T value) {
                setValue(value);
                return *this;
            }

            /**
             * @brief Explizite Konvertierung des Wertes in einen numerischen Typ bzw. bool.
             * 
             * @tparam T Der Typ, in den der Wert konvertiert werden soll.
             * @return Der Wert.
             */
            template <typename T>
            inline explicit operator T() const {
                return getValue<T>();
            }

            /**
             * Getter für den Typ und den Wertebereich der Option.
             */

            inline OptionType getType() const { return type; }

            inline std::string getMinValue() const { return minValue; }
            inline std::string getMaxValue() const { return maxValue; }
    };

    /**
     * @brief Eine Liste von UCI-Optionen.
     * Die Klasse ist ein Wrapper für std::vector<Option> mit dem
     * Unterschied, dass die Optionen über ihren Namen indiziert werden
     * anstatt über ihren Index.
     */
    class Options {
        private:
            std::vector<Option> options;

        public:
            /**
             * @brief Konstruiert ein Options-Objekt
             * aus einer Initializer-Liste von Optionen.
             * 
             * @param options Die Liste von Optionen.
             */
            Options(std::initializer_list<Option> options) :
                options(options) {}

            /**
             * @brief Kopierkonstruktor.
             * 
             * @param options Das, zu kopierende, Options-Objekt.
             */
            Options(const Options& options) :
                options(options.options) {}

            /**
             * @brief Zuweisungsoperator.
             * 
             * @param options Das, zu kopierende, Options-Objekt.
             * @return Referenz auf this.
             */
            inline Options& operator=(const Options& options) {
                this->options = options.options;
                return *this;
            }

            /**
             * @brief Gibt den internen std::vector<Option> zurück.
             */
            constexpr const std::vector<Option>& getOptions() { return options; }

            /**
             * @brief Sucht eine Option anhand ihres Namens und gibt sie zurück.
             * 
             * @param name Der Name der Option.
             * @return Referenz auf die Option.
             * 
             * @throw Wenn die Option nicht existiert.
             */
            inline Option& getOption(std::string name) {
                for(Option& option : options)
                    if(option.getName() == name)
                        return option;

                throw std::invalid_argument("Option " + name + " does not exist");
            }

            /**
             * @brief Alias für getOption().
             * 
             * @param name Der Name der Option.
             * @return Referenz auf die Option.
             * 
             * @throw Wenn die Option nicht existiert.
             */
            inline Option& operator[](std::string name) {
                return getOption(name);
            }

            /**
             * begin() und end() für die Verwendung in range-based for-loops.
             */

            inline Option* begin() { return options.data(); }
            inline Option* end() { return options.data() + options.size(); }
    };

    extern Options options;

    /**
     * Template-Spezialisierungen für die Konvertierung von Strings 
     * in numerische Typen bzw. bool und umgekehrt.
     */

    template <>
    inline std::string Option::fromString(std::string value) const {
        return value;
    }

    template <>
    inline bool Option::fromString(std::string value) const {
        return value == "true";
    }

    template <>
    inline int Option::fromString(std::string value) const {
        return std::stoi(value);
    }

    template <>
    inline long long Option::fromString(std::string value) const {
        return std::stoll(value);
    }

    template <>
    inline size_t Option::fromString(std::string value) const {
        return std::stoul(value);
    }

    template <>
    inline double Option::fromString(std::string value) const {
        return std::stod(value);
    }

    template <>
    inline std::string Option::toString(std::string value) const {
        return value;
    }

    template <>
    inline std::string Option::toString(bool value) const {
        return value ? "true" : "false";
    }
}

#endif