/* ==== Basic ==== */

// note: not safe, use with caution
String string_advance(String s, u64 pos) {
    return (String) {s.data + pos, s.count - pos};
}

String string_eat(String* s, u64 count) {
    if (count > s->count) return (String) {0};
    u8* old = s->data;
    *s = string_advance(*s, count);
    return (String) { old, count };
}

// todo: speed
u8 string_equal(String a, String b) {
    if (a.count != b.count) return 0;
    if (a.data  == b.data ) return 1;
    for (u64 i = 0; i < a.count; i++) {
        if (a.data[i] != b.data[i]) return 0; 
    }
    return 1;
}

u8 string_contains_u8(String s, u8 c) {
    for (u64 i = 0; i < s.count; i++) {
        if (s.data[i] == c) return 1; 
    }
    return 0;
}

// naive search for now
// todo: validate
String string_find(String a, String b) {
    if (!a.count || !b.count || !a.data || !b.data || (b.count > a.count)) return (String) {0};
    for (u64 i = 0; i < a.count - b.count + 1; i++) {
        for (u64 j = 0; j < b.count; j++) {
            if (a.data[i + j] != b.data[j]) goto next;
        }
        return (String) {a.data + i, a.count - i};
        next: continue;
    }
    return (String) {0};
}


// basic print
void print_string(String s) {
    fwrite(s.data, sizeof(u8), s.count, stdout);
}

u64 file_print(FILE* f, String s) {
    fwrite(s.data, sizeof(u8), s.count, f);
    return s.count;
}

char* temp_c_string(String s) {
    char* out = temp_alloc(s.count + 1);
    memcpy(out, s.data, s.count);
    out[s.count] = 0;
    return out;
}

// todo: handle alloc failed
String string_copy(String s) {
    String out = { malloc(s.count), s.count };
    for (u64 i = 0; i < s.count; i++) out.data[i] = s.data[i]; // todo: speed
    return out;
}


// todo: do we really need to switch allocator?
String load_file(char* path) {

    FILE* f = fopen(path, "rb");
    if (!f) return (String) {0}; // todo: this is not enough, what if we have a file of size 0? 

    fseek(f, 0, SEEK_END);
    u64 count = ftell(f);
    fseek(f, 0, SEEK_SET);

    u8* data = malloc(count);
    if (!data) {
        fclose(f);
        return (String) {0}; 
    }
    
    fread(data, 1, count, f);
    fclose(f);

    return (String) {data, count};
}


// todo: not robust, need more testing, handle adjacent items (no space in between)
void print(String s, ...) {
    
    va_list args;
    va_start(args, s);
    
    for (u64 i = 0; i < s.count; i++) {

        u8 c = s.data[i];
        if (c == '@') {
            if (i + 1 < s.count && s.data[i + 1] == '@') { // short circuit 
                putchar('@');
                i++;
            } else {
                print_string(va_arg(args, String)); // not safe, but this is C varargs, what can you do 
            }
            continue;
        }

        putchar(c);
    }
    
    va_end(args);
}


// note: does not give a copy
String read_line() {
    
    String s = context.input_buffer;

    fgets((char*) s.data, s.count, stdin);
    s.count = strlen((const char*) s.data);
    
    if (s.count == 0) return (String) {0};
    if (s.data[s.count - 1] == '\n') s.count -= 1;
    
    return s;
}





/* ---- Trim ---- */

// todo: validate
String string_trim_any_u8_from_start(String s, String u) {
    for (u64 i = 0; i < s.count; i++) {
        if (!string_contains_u8(u, s.data[i])) return string_advance(s, i);
    }
    return (String) {0};
}

// todo: validate
String string_trim_any_u8_from_end(String s, String u) {
    for (u64 i = s.count; i > 0;) {
        i--;
        if (!string_contains_u8(u, s.data[i])) return (String) {s.data, i + 1};
    }
    return (String) {0};
}

// todo: inline these calls?
String string_trim_any_u8(String s, String u) {
    return string_trim_any_u8_from_start(string_trim_any_u8_from_end(s, u), u);
}

/*
    note: 
    the order of spaces affect performance,
    for example: " \n\r\t" will be faster on input data that doesn't have tabs
*/

String string_trim_spaces_from_start(String s) {
    return string_trim_any_u8_from_start(s, string(" \t\r\n"));    
}

String string_trim_spaces_from_end(String s) {
    return string_trim_any_u8_from_end(s, string(" \t\r\n"));    
}

// todo: inline these calls?
String string_trim_spaces(String s) {
    return string_trim_any_u8(s, string(" \t\r\n"));
}





/* ---- Parsing ---- */

// todo: validate
String string_eat_by_separator(String* s, String separator) {
    
    String found = string_find(*s, separator);
    if (!found.count) {
        String out = *s;
        *s = (String) {0};
        return out; 
    }
    
    String out = { s->data, found.data - s->data };
    *s = string_advance(*s, out.count + separator.count);

    return out;
}

// todo: should we make this the default?
// todo: validate
String string_eat_by_separator_excluding_empty(String* s, String separator) {
    
    u8* data  = NULL;     
    u64 count = 0;
    
    while (!count) {
        
        String found = string_find(*s, separator);
        if (!found.count) {
            String out = *s;
            *s = (String) {0};
            return out; 
        }
        
        data  = s->data;
        count = found.data - data;
    
        *s = string_advance(*s, count + separator.count);
    }
    
    return (String) { data, count };
}

// todo: validate
String string_eat_by_any_u8_separators(String* s, String separators) {

    *s = string_trim_any_u8_from_start(*s, separators);
    
    u64 count = 0;
    while (count < s->count) {
        if (string_contains_u8(separators, s->data[count])) break;
        count++; 
    }

    String out = { s->data, count };

    u64 to_skip = 0;
    for (u64 i = count; i < s->count; i++) {
        if (string_contains_u8(separators, s->data[i])) to_skip++;
        else break;
    }

    *s = string_advance(*s, count + to_skip);
    
    return out;
}

String string_eat_by_spaces(String* s) {
    return string_eat_by_any_u8_separators(s, string(" \t\r\n"));
}

String string_eat_line_excluding_empty(String* s) {
    return string_eat_by_any_u8_separators(s, string("\r\n"));
}




/* ==== Number Parsing ==== */

// todo: validate
u8 parse_u64(String s, u64* out) {

    if (!s.count) return 0;

    u64 start  = 0;
    while (start < s.count && s.data[start] <= ' ') start++;
    
    u64 result = 0;
    u64 length = 0;
    
    for (u64 i = start; i < s.count; i++) {
        
        u8 c = s.data[i];
        if (c < '0' || c > '9') return 0;
        
        result *= 10;
        result += c - '0';

        length++;
    }

    if (length == 0 || length > 20) return 0; // todo: not handling all overflows

    *out = result;

    return 1;
}

// todo: validate
u8 parse_s64(String s, s64* out) {

    if (!s.count) return 0;

    u64 start  = 0;
    while (start < s.count && s.data[start] <= ' ') start++;
 
    s64 sign   = 1;
    switch (s.data[start]) {
        case '-': sign = -1; // fall-through
        case '+': start++;
        default:  break;
    }
   
    s64 result = 0;
    u64 length = 0;
    
    for (u64 i = start; i < s.count; i++) {
        
        u8 c = s.data[i];
        if (c < '0' || c > '9') return 0;
        
        result *= 10;
        result += c - '0';

        length++;
    }

    if (length == 0 || length > 20) return 0; // todo: not handling all overflows

    *out = sign * result;

    return 1;
}

// todo: validate
// todo: precision
u8 parse_f64(String s, f64* out) {
    
    if (!s.count) return 0;

    u64 start  = 0;
    while (start < s.count && s.data[start] <= ' ') start++;
 
    f64 sign   = 1;
    switch (s.data[start]) {
        case '-': sign = -1; // fall-through
        case '+': start++;
        default:  break;
    }
   
    f64 result = 0;
    
    u64 dot_pos = start;
    while (dot_pos < s.count) {
       
        u8 c = s.data[dot_pos];
        
        if (c == '.') {
            break;
        } else {
            if (c < '0' || c > '9') return 0;
        }

        dot_pos++;
    }
    
    for (u64 i = start; i < dot_pos; i++) {
        result *= 10.0;
        result += s.data[i] - '0';
    }

    f64 denom = 10;
    for (u64 i = dot_pos + 1; i < s.count; i++) {
        
        u8 c = s.data[i];
       
        if (c < '0' || c > '9') return 0;
        
        result += (f64) (c - '0') / denom;
        denom *= 10;
    }
    
    *out = sign * result;

    return 1;
}


