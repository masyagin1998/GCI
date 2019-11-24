#ifndef DATA_TYPES_H_INCLUDED
#define DATA_TYPES_H_INCLUDED

#include <stdlib.h>

enum VALUE_TYPE
{
    VALUE_TYPE_INTEGER,
    VALUE_TYPE_DOUBLE,
    VALUE_TYPE_OBJ,
    VALUE_TYPE_ARR,
};

struct VALUE
{
    char mark;
    
    union
    {
        long long int_val;
        double double_val;
        struct OBJECT*obj_val;
        struct ARRAY*arr_val;
    };

    enum VALUE_TYPE type;
};

struct PROPERTY
{
    size_t key;
    struct VALUE val;
};

struct OBJECT
{
    size_t properties_len;
    size_t properties_cap;
    struct PROPERTY properties[];
};

struct ARRAY
{
    size_t len;
    size_t cap;
    struct VALUE values[];
};

#endif  /* DATA_TYPES_H_INCLUDED */
