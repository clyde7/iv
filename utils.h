#ifndef UTILS_H
#define UTILS_H

#define array_count(array) (sizeof(array) / sizeof((array)[0]))

#define swap(Type, a, b) { Type t = (a); (a) = (b); (b) = t; }
#define cycle3(Type, a, b, c) { Type t = (a); (a) = (b); (b) = (c); (c) = t; }

#define toggle(flag) (flag) = (! flag)
#define toggle_message(flag, notify) { (flag) = (! flag); if (listener) notify(flag); }
#define toggle_value(flag, a, b) \
    (flag) = (flag) == b ? (a) : (b);

#define cycle_type(Type, value, first, last) \
    (value) = (Type) ((value) == (last) ? (first) : (value) + 1)
#define cycle(value, first, last) \
    (value) = (value) == (last) ? (first) : (value) + 1
#define cycle_down(value, first, last) \
    (value) = (value) == (first) ? (last) : (value) - 1
#define cycle_array(target, array) \
{ \
    int i, length = array_count(array); \
    for (i = 0; i != length; ++ i) \
    { \
        if ((array)[i] == target) \
            break; \
    } \
    (target) = (array)[(i + 1) % length]; \
}

#endif

