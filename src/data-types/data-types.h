#ifndef DATA_TYPES_H_INCLUDED
#define DATA_TYPES_H_INCLUDED

enum VALUE_TYPE
{
    VALUE_TYPE_INTEGER,
    VALUE_TYPE_DOUBLE,
    VALUE_TYPE_OBJ,
};

struct VALUE
{
    unsigned mark;
    
    union
    {
        long long int_val;
        double double_val;
        struct OBJECT*obj_val;
    };

    enum VALUE_TYPE type;
};

struct PROPERTY
{
    unsigned long long key;
    struct VALUE val;
};

struct OBJECT
{
    unsigned properties_len;
    unsigned properties_cap;
    struct PROPERTY properties[];
};

#endif  /* DATA_TYPES_H_INCLUDED */
