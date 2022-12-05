/* ==== Macros ==== */

#define string(s)            (String) {(u8*) s, sizeof(s) - 1}                      // same as utf8_string() on modern compilers?
#define array_string(s)      (String) {(u8*) s, sizeof(s)}
#define data_string(s)       (String) {(u8*) &s, sizeof(s)}
#define count_of(array)      (sizeof(array) / sizeof(array[0]))
#define array(Type, c_array) (Array(Type)) {c_array, count_of(c_array)}

#define Array(Type) Array_ ## Type
#define Define_Array(Type) \
typedef struct {           \
    Type* data;            \
    u64   count;           \
} Array(Type)              \



/* ==== Types ==== */

typedef unsigned char           u8;
typedef unsigned short int      u16;
typedef unsigned int            u32;
typedef unsigned long long int  u64;
typedef signed char             s8;
typedef signed short            s16;
typedef signed int              s32;
typedef signed long long int    s64;
typedef float                   f32;
typedef double                  f64;

typedef struct {
    u8* data;
    u64 count;
} String;

Define_Array(String);






typedef struct {
    u8* data;
    u64 size;
    u64 allocated;
    u64 highest;
} ArenaBuffer;

typedef struct {
    ArenaBuffer   temp_buffer;
    String        input_buffer;
    void*         (*alloc)(u64);
    Array(String) command_line_args;
} Context;

Context context;

void* temp_alloc(u64 count) {

    ArenaBuffer* a = &context.temp_buffer;
    
    u64 current = a->allocated;
    u64 wanted  = current + count;
    
    assert(wanted < a->size);
    
    if (wanted > a->highest) a->highest = wanted; // check the highest here, maybe slow?
    a->allocated = wanted;

    return a->data + current;
}

void temp_free(u64 size) {
    context.temp_buffer.allocated -= size;
}

void temp_reset() {
    ArenaBuffer* a = &context.temp_buffer;
    a->allocated = 0;
    memset(a->data, 0, a->highest); // do we need this?
}

void temp_info() {
    ArenaBuffer* a = &context.temp_buffer;
    printf(
        "\nTemp Buffer Info:\n"
        "Data:      %p\n"
        "Size:      %lld\n"
        "Allocated: %lld\n"
        "Highest:   %lld\n\n",
        a->data, a->size, a->allocated, a->highest
    );
}






void program();

int main(int arg_count, char** args) {
    
    // setup context
    const u64 size = 1024 * 256;
    context.temp_buffer.data = calloc(size, sizeof(u8));
    context.temp_buffer.size = size;
    context.alloc = malloc;

    context.input_buffer      = (String) { calloc(8192, sizeof(u8)), 8192 }; 
    context.command_line_args = (Array(String)) { malloc(sizeof(String) * arg_count), arg_count };

    for (int i = 0; i < arg_count; i++) {
        context.command_line_args.data[i] = (String) {(u8*) args[i], strlen(args[i])};
    }
    
    setvbuf(stdout, NULL, _IONBF, 0); // force some shell to print immediately (so stdout before input will not be hidden) 

    program();
}


