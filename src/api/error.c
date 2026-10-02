#include "error/error_api.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int lo_error_message_reserve(
    lo_error_message *message,
    size_t capacity)
{
    char *data;

    if (!message)
        return -1;

    if (capacity <= message->capacity)
        return 0;

    data = realloc(message->data, capacity);

    if (!data)
        return -1;

    message->data = data;
    message->capacity = capacity;

    return 0;
}

void lo_error_init(lo_error *error)
{
    if (!error)
        return;

    lo_diagnostic_init(&error->diagnostic);
    error->code = 0;
}

void lo_error_set(
    lo_error *error,
    int code,
    lo_diagnostic_severity severity,
    lo_diagnostic_kind kind,
    const char *message)
{
    if (!error)
        return;

    error->code = code;

    lo_diagnostic_set(
        &error->diagnostic,
        severity,
        kind,
        message
    );
}

int lo_error_failed(const lo_error *error)
{
    if (!error)
        return 1;

    return error->diagnostic.severity >=
           LO_DIAGNOSTIC_ERROR;
}

void lo_error_message_init(
    lo_error_message *message)
{
    if (!message)
        return;

    message->data = NULL;
    message->length = 0;
    message->capacity = 0;
}

void lo_error_message_free(
    lo_error_message *message)
{
    if (!message)
        return;

    free(message->data);

    message->data = NULL;
    message->length = 0;
    message->capacity = 0;
}

void lo_error_message_clear(
    lo_error_message *message)
{
    if (!message)
        return;

    message->length = 0;

    if (message->data)
        message->data[0] = '\0';
}

int lo_error_message_write(
    lo_error_message *message,
    const char *data,
    size_t length)
{
    size_t required;
    size_t capacity;

    if (!message || (!data && length != 0))
        return -1;

    if (length > SIZE_MAX - message->length - 1)
        return -1;

    required = message->length + length + 1;

    if (required > message->capacity) {
        capacity = message->capacity ?
                   message->capacity : 256;

        while (capacity < required) {
            if (capacity > SIZE_MAX / 2)
                return -1;

            capacity *= 2;
        }

        if (lo_error_message_reserve(
                message,
                capacity) != 0)
            return -1;
    }

    if (length != 0)
        memcpy(
            message->data + message->length,
            data,
            length
        );

    message->length += length;
    message->data[message->length] = '\0';

    return 0;
}

int lo_error_message_puts(
    lo_error_message *message,
    const char *string)
{
    if (!string)
        return -1;

    return lo_error_message_write(
        message,
        string,
        strlen(string)
    );
}

const char *lo_error_message_data(
    const lo_error_message *message)
{
    if (!message || !message->data)
        return "";

    return message->data;
}

size_t lo_error_message_size(
    const lo_error_message *message)
{
    if (!message)
        return 0;

    return message->length;
}

void lo_error_handler_init(
    lo_error_handler *handler)
{
    if (!handler)
        return;

    handler->errors = NULL;
    handler->error_count = 0;
    handler->error_capacity = 0;
    handler->warning_count = 0;
    handler->note_count = 0;
}

void lo_error_handler_free(
    lo_error_handler *handler)
{
    if (!handler)
        return;

    free(handler->errors);

    handler->errors = NULL;
    handler->error_count = 0;
    handler->error_capacity = 0;
    handler->warning_count = 0;
    handler->note_count = 0;
}

int lo_error_handler_add(
    lo_error_handler *handler,
    const lo_error *error)
{
    lo_error *errors;
    size_t capacity;

    if (!handler || !error)
        return -1;

    if (error->diagnostic.severity ==
        LO_DIAGNOSTIC_WARNING)
        handler->warning_count++;

    if (error->diagnostic.severity ==
        LO_DIAGNOSTIC_NOTE)
        handler->note_count++;

    if (error->diagnostic.severity <
        LO_DIAGNOSTIC_ERROR)
        return 0;

    if (handler->error_count ==
        handler->error_capacity) {

        capacity = handler->error_capacity ?
                   handler->error_capacity * 2 : 8;

        if (capacity < handler->error_capacity)
            return -1;

        errors = realloc(
            handler->errors,
            capacity * sizeof(*errors)
        );

        if (!errors)
            return -1;

        handler->errors = errors;
        handler->error_capacity = capacity;
    }

    handler->errors[handler->error_count++] = *error;

    return 0;
}

int lo_error_handler_error(
    lo_error_handler *handler,
    int code,
    lo_diagnostic_kind kind,
    const char *message)
{
    lo_error error;

    lo_error_init(&error);

    lo_error_set(
        &error,
        code,
        LO_DIAGNOSTIC_ERROR,
        kind,
        message
    );

    return lo_error_handler_add(handler, &error);
}

int lo_error_handler_warning(
    lo_error_handler *handler,
    lo_diagnostic_kind kind,
    const char *message)
{
    lo_error error;

    lo_error_init(&error);

    lo_error_set(
        &error,
        0,
        LO_DIAGNOSTIC_WARNING,
        kind,
        message
    );

    return lo_error_handler_add(handler, &error);
}

int lo_error_handler_note(
    lo_error_handler *handler,
    lo_diagnostic_kind kind,
    const char *message)
{
    lo_error error;

    lo_error_init(&error);

    lo_error_set(
        &error,
        0,
        LO_DIAGNOSTIC_NOTE,
        kind,
        message
    );

    return lo_error_handler_add(handler, &error);
}

int lo_error_handler_failed(
    const lo_error_handler *handler)
{
    if (!handler)
        return 1;

    return handler->error_count != 0;
}

size_t lo_error_handler_count(
    const lo_error_handler *handler)
{
    if (!handler)
        return 0;

    return handler->error_count;
}

const lo_error *lo_error_handler_get(
    const lo_error_handler *handler,
    size_t index)
{
    if (!handler || index >= handler->error_count)
        return NULL;

    return &handler->errors[index];
}
