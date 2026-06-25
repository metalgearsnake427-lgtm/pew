typedef enum
{
    OBJ_STRING,
    OBJ_ARRAY,
    OBJ_FUNCTION,
    OBJ_CLASS,
    OBJ_INSTANCE
} ObjType;

typedef struct GCObject
{
    ObjType type;

    unsigned char marked;

    struct GCObject *next;

} GCObject;
