/**
 * @file macro.h
 * @brief Header file defining custom logging macros for different log levels.
 *
 * This file provides macros for logging messages with custom formats and log levels.
 * The macros allow for formatted logging with additional context such as file name,
 * line number, and function name. The log levels supported include error, warning,
 * info, and debug.
 *
 * Macros:
 * - CUSTOM_LOG_FORMAT(letter, format): Defines the log format string with color codes,
 *   timestamp, log level, file name, line number, and function name.
 * - custom_log(letter, format, ...): General-purpose macro for logging messages with
 *   a specified log level and format.
 * - error(format, ...): Logs an error message.
 * - warn(format, ...): Logs a warning message if the WARN level is enabled.
 * - info(format, ...): Logs an informational message.
 * - debug(format, ...): Logs a debug message if the DEBUG level is enabled.
 *
 * Usage:
 * - Include this header file in your project.
 * - Use the provided macros to log messages at the desired log level.
 * - Enable or disable specific log levels by defining or omitting the WARN and DEBUG macros.
 *
 * @note:
 * - The `pathToFileName` function is expected to extract the file name from the full file path.
 * - The `esp_timer_get_time` function is used to retrieve the current time in microseconds.
 */
#ifndef _MACRO_H_
#define _MACRO_H_

#define DEBUG
#define WARN

#define CUSTOM_LOG_FORMAT(letter, format)  ARDUHAL_LOG_COLOR_ ## letter "[%6u][" #letter "][%s:%u] %s(): " format ARDUHAL_LOG_RESET_COLOR

// Macro pour les logs personnalisés
#define custom_log(letter, format, ...) \
  log_printf(CUSTOM_LOG_FORMAT(letter, format), \
         (unsigned long)(esp_timer_get_time() / 1000ULL), \
         pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)

// Macros spécifiques pour chaque niveau de log
#define error(format, ...) custom_log(E, format, ##__VA_ARGS__) // Erreur

#ifdef WARN
#define warn(format, ...) custom_log(W, format, ##__VA_ARGS__) // Avertissement
#else
#define warn(format, ...) // Ne rien faire si le niveau de log est inférieur à WARN
#endif

#define info(format, ...) custom_log(I, format, ##__VA_ARGS__) // Info

#ifdef DEBUG
#define debug(format, ...) custom_log(D, format, ##__VA_ARGS__) // Debug
#else
#define debug(format, ...) // Ne rien faire si le niveau de log est inférieur à DEBUG
#endif

#endif