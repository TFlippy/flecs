#include "private_api.h"

static
char *ecs_vasprintf(
    const char *fmt,
    va_list args)
{
    ecs_size_t size = 0;
    char *result  = NULL;
    va_list tmpa;

    va_copy(tmpa, args);

    size = vsnprintf(result, ecs_to_size_t(size), fmt, tmpa);

    va_end(tmpa);

    if ((int32_t)size < 0) { 
        return NULL; 
    }

    result = (char *) ecs_os_malloc(size + 1);

    if (!result) { 
        return NULL; 
    }

    ecs_os_vsprintf(result, fmt, args);

    return result;
}

static int trace_indent = 0;
static int trace_level = 0;

void ecs_log_print(
    int level,
    const char *file,
    int32_t line,
    const char *fmt,
    va_list valist)
{
    (void)level;
    (void)line;

    if (level > trace_level) {
        return;
    }

    /* Massage filename so it doesn't take up too much space */
    char filebuff[256];
    ecs_os_strcpy(filebuff, file);
    file = filebuff;
    char *file_ptr = strrchr(file, '/');
    if (file_ptr) {
        file = file_ptr + 1;
    }

    /* Extension is likely the same for all files */
    file_ptr = strrchr(file, '.');
    if (file_ptr) {
        *file_ptr = '\0';
    }

    char indent[32];
    int i;
    for (i = 0; i < trace_indent; i ++) {
        indent[i * 2] = '|';
        indent[i * 2 + 1] = ' ';
    }
    indent[i * 2] = '\0';

    char *msg = ecs_vasprintf(fmt, valist);

    if (level >= 0) {
        ecs_os_log("%sinfo%s: %s%s%s%s",
            ECS_MAGENTA, ECS_NORMAL, ECS_GREY, indent, ECS_NORMAL, msg);
    } else if (level == -2) {
        ecs_os_warn("%swarn%s: %s%s%s%s",
            ECS_YELLOW, ECS_NORMAL, ECS_GREY, indent, ECS_NORMAL, msg);
    } else if (level <= -2) {
        ecs_os_err("%serr %s: %s%s%s%s",
            ECS_RED, ECS_NORMAL, ECS_GREY, indent, ECS_NORMAL, msg);
    }

    ecs_os_free(msg);
}

void _ecs_trace(
    int level,
    const char *file,
    int32_t line,
    const char *fmt,
    ...)
{
    va_list valist;
    va_start(valist, fmt);

    ecs_log_print(level, file, line, fmt, valist);
}

void _ecs_warn(
    const char *file,
    int32_t line,
    const char *fmt,
    ...)
{
    va_list valist;
    va_start(valist, fmt);

    ecs_log_print(-2, file, line, fmt, valist);
}

void _ecs_err(
    const char *file,
    int32_t line,
    const char *fmt,
    ...)
{
    va_list valist;
    va_start(valist, fmt);

    ecs_log_print(-3, file, line, fmt, valist);
}

void ecs_log_push(void) {
    trace_indent ++;
}

void ecs_log_pop(void) {
    trace_indent --;
}

void ecs_tracing_enable(
    int level)
{
    trace_level = level;
}

static
const char* strip_file(
    const char *file)
{
    const char *f = strrchr(file, '/');
    if (f) {
        return f + 1;
    } else {
        return file;
    }
}

void _ecs_abort(
    int32_t error_code,
    const char *file,
    int32_t line)
{
    ecs_err("abort %s:%d: %s", strip_file(file), line, ecs_strerror(error_code));
    ecs_os_abort();
}

void _ecs_assert(
    bool condition,
    int32_t error_code,
    const char *condition_str,
    const char *file,
    int32_t line)
{
    if (!condition) {
        ecs_err("assert(%s) %s:%d: %s", condition_str, strip_file(file), line, 
            ecs_strerror(error_code));
        ecs_os_abort();
    }
}

const char* ecs_strerror(
    int32_t error_code)
{
    switch (error_code) {

    case ECS_INTERNAL_ERROR:
        return "ECS_INTERNAL_ERROR";
    case ECS_INVALID_OPERATION:
        return "ECS_INVALID_OPERATION";
    case ECS_INVALID_PARAMETER:
        return "ECS_INVALID_PARAMETER";
    case ECS_INVALID_ID:
        return "ECS_INVALID_ID";
    case ECS_INVALID_COMPONENT:
        return "ECS_INVALID_COMPONENT";
    case ECS_OUT_OF_MEMORY:
        return "ECS_OUT_OF_MEMORY";
    case ECS_MISSING_OS_API:
        return "ECS_MISSING_OS_API";
    case ECS_INCONSISTENT_COMPONENT_ACTION:
        return "ECS_INCONSISTENT_COMPONENT_ACTION";
    case ECS_INVALID_FROM_WORKER:
        return "ECS_INVALID_FROM_WORKER";
    }

    return "unknown error code";
}
