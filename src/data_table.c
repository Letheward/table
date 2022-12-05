
/* ==== Data Table ==== */

enum {
    DTE_TYPE_EMPTY,
    DTE_TYPE_UINT64,
    DTE_TYPE_INT64,
    DTE_TYPE_FLOAT64,
    DTE_TYPE_STRING,
    DTE_TYPE_USER_DATA,
};

typedef struct {
    u32 type;
    union {
        u64    uint64;
        s64    int64;
        f64    float64;
        String string;
        void*  user_data;
    } data;
} DataTable_Entry;

typedef struct {
    DataTable_Entry* data;
    u64              display_width;
    u64              display_height;
    u64              width;
    u64              height;
    u64              allocated;
    u8               is_from_file; // todo: solve this better 
} DataTable;

DataTable data_table_init(u64 width, u64 height) {
    return (DataTable) {
        .data      = calloc(width * height, sizeof(DataTable_Entry)),
        .width     = width,
        .height    = height,
        .allocated = width * height,
    };
}

// todo: validate 
void data_table_free(DataTable* table) {
    if (!table->is_from_file) free(table->data);
    *table = (DataTable) {0};
}

// todo: validate 
void data_table_resize(DataTable* table, u64 width, u64 height) {

    assert(width  >= table->width);
    assert(height >= table->height);
    
    if (width == table->width && height == table->height) return;
    
    DataTable_Entry* new = calloc(width * height, sizeof(DataTable_Entry)); 
    
    for (u64 i = 0; i < table->height; i++) {
        for (u64 j = 0; j < table->width; j++) {
            new[i * width + j] = table->data[i * table->width + j];
        }
    }
    
    if (table->is_from_file) {
        table->is_from_file = 0;
    } else {
        free(table->data);
    }
    
    table->data      = new;
    table->width     = width;
    table->height    = height;
    table->allocated = width * height;
}

void data_table_set(DataTable* table, u64 x, u64 y, DataTable_Entry entry) {
    
    u64 new_width  = table->width;
    u64 new_height = table->height;
    
    if (x >= table->width) {
        u64 size  = x + 1;
        u64 twice = table->width * 2;
        new_width = size > twice ? size : twice;
    } 
    
    if (y >= table->height) {
        u64 size   = y + 1;
        u64 twice  = table->height * 2;
        new_height = size > twice ? size : twice;
    } 
    
    data_table_resize(table, new_width, new_height);
    if (x >= table->display_width)  table->display_width  = x + 1;    
    if (y >= table->display_height) table->display_height = y + 1;    
    
    table->data[y * table->width + x] = entry;
}

DataTable_Entry data_table_get(DataTable* table, u64 x, u64 y) {
    // todo: handle these
    assert(x < table->width);
    assert(y < table->height);
    return table->data[y * table->width + x];
}

void data_table_clear(DataTable* table, u64 x, u64 y) {
    data_table_set(table, x, y, (DataTable_Entry) {0});
}

void data_table_set_uint64(DataTable* table, u64 x, u64 y, u64 item) {
    data_table_set(table, x, y, (DataTable_Entry) {.data.uint64 = item, .type = DTE_TYPE_UINT64});
}

void data_table_set_int64(DataTable* table, u64 x, u64 y, s64 item) {
    data_table_set(table, x, y, (DataTable_Entry) {.data.int64 = item, .type = DTE_TYPE_INT64});
}

void data_table_set_float64(DataTable* table, u64 x, u64 y, f64 item) {
    data_table_set(table, x, y, (DataTable_Entry) {.data.float64 = item, .type = DTE_TYPE_FLOAT64});
}

void data_table_set_string(DataTable* table, u64 x, u64 y, String s) {
    data_table_set(table, x, y, (DataTable_Entry) {.data.string = s, .type = DTE_TYPE_STRING});
}

void data_table_set_user_data(DataTable* table, u64 x, u64 y, void* data) {
    data_table_set(table, x, y, (DataTable_Entry) {.data.user_data = data, .type = DTE_TYPE_USER_DATA});
}




/* ---- Basic Operations ---- */

// todo: validate
// todo: do we want to shrink the spaces in up and left?
void data_table_shrink_unused(DataTable* table) {
    
    u64 max_width = 0;
    for (u64 i = 0; i < table->display_height; i++) {
        
        u64 pos = table->display_width;
        while (pos > 0) {
            pos--;
            DataTable_Entry entry = table->data[i * table->width + pos];
            if (entry.type != DTE_TYPE_EMPTY)  break;
        }
        
        u64 width = pos + 1;
        max_width = width > max_width ? width : max_width;
    }

    table->display_width = max_width;
    
    u64 max_height = 0;
    for (u64 i = 0; i < table->display_width; i++) {
        
        u64 pos = table->display_height;
        while (pos > 0) {
            pos--;
            DataTable_Entry entry = table->data[pos * table->width + i];
            if (entry.type != DTE_TYPE_EMPTY)  break;
        }
        
        u64 height = pos + 1;
        max_height = height > max_height ? height : max_height;
    }

    table->display_height = max_height;
}


void data_table_swap_column(DataTable* table, u64 a, u64 b) {
    
    assert(a < table->display_width);
    assert(b < table->display_width);
    
    for (u64 i = 0; i < table->display_height; i++) {
        
        DataTable_Entry ea = table->data[i * table->width + a];
        DataTable_Entry eb = table->data[i * table->width + b];
    
        table->data[i * table->width + a] = eb;
        table->data[i * table->width + b] = ea;
    }
}

void data_table_swap_row(DataTable* table, u64 a, u64 b) {
    
    assert(a < table->display_height);
    assert(b < table->display_height);
    
    for (u64 i = 0; i < table->display_width; i++) {
        
        DataTable_Entry ea = table->data[a * table->width + i];
        DataTable_Entry eb = table->data[b * table->width + i];
    
        table->data[a * table->width + i] = eb;
        table->data[b * table->width + i] = ea;
    }
}


void data_table_clear_row(DataTable* table, u64 index) {
    assert(index < table->display_height);
    for (u64 i = 0; i < table->display_height; i++) {
        table->data[index * table->width + i] = (DataTable_Entry) {0};
    }
}

void data_table_clear_column(DataTable* table, u64 index) {
    assert(index < table->display_width);
    for (u64 i = 0; i < table->display_height; i++) {
        table->data[i * table->width + index] = (DataTable_Entry) {0};
    }
}

// copy a to overwrite b
void data_table_copy_overwrite_row(DataTable* table, u64 a, u64 b) {
    assert(a < table->display_height);
    assert(b < table->display_height);
    if (a == b) return;
    for (u64 i = 0; i < table->display_width; i++) {
        table->data[b * table->width + i] = table->data[a * table->width + i];
    }
}

// copy a to overwrite b
void data_table_copy_overwrite_column(DataTable* table, u64 a, u64 b) {
    assert(a < table->display_width);
    assert(b < table->display_width);
    if (a == b) return;
    for (u64 i = 0; i < table->display_height; i++) {
        table->data[i * table->width + b] = table->data[i * table->width + a];
    }
}

// cut a to overwrite b
void data_table_cut_overwrite_row(DataTable* table, u64 a, u64 b) {
    if (a == b) return;
    data_table_copy_overwrite_row(table, a, b);
    data_table_clear_row(table, a);
}

// cut a to overwrite b
void data_table_cut_overwrite_column(DataTable* table, u64 a, u64 b) {
    if (a == b) return;
    data_table_copy_overwrite_column(table, a, b);
    data_table_clear_column(table, a);
}


typedef void DataTable_Apply_Function(DataTable_Entry* in);

void data_table_row_apply(DataTable* table, u64 row_index, u64 offset, DataTable_Apply_Function* func) {
    assert(row_index < table->height);
    for (u64 i = offset; i < table->width; i++) {
        func(&table->data[row_index * table->width + i]);
    }
}

void data_table_column_apply(DataTable* table, u64 column_index, u64 offset, DataTable_Apply_Function* func) {
    assert(column_index < table->width);
    for (u64 i = offset; i < table->height; i++) {
        func(&table->data[i * table->width + column_index]);
    }
}


typedef DataTable_Entry DataTable_Reduce_Function(DataTable_Entry acc, DataTable_Entry current);

// todo: cleanup, use inclusive for end?
DataTable_Entry data_table_row_reduce(DataTable* table, u64 row_index, u64 start, u64 end, DataTable_Reduce_Function* func) {
    
    assert(row_index < table->display_height);
    assert(start <  table->display_width);
    assert(end   <= table->display_width);
    
    if (end == 0) end = table->display_width;
    
    DataTable_Entry acc = table->data[row_index * table->width + start];  
    for (u64 i = start + 1; i < end; i++) {
        acc = func(acc, table->data[row_index * table->width + i]);
    }

    return acc;
}

// todo: cleanup, use inclusive for end?
DataTable_Entry data_table_column_reduce(DataTable* table, u64 column_index, u64 start, u64 end, DataTable_Reduce_Function* func) {
    
    assert(column_index < table->display_width);
    assert(start <  table->display_height);
    assert(end   <= table->display_height);
    
    if (end == 0) end = table->display_height;
    
    DataTable_Entry acc = table->data[start * table->width + column_index];  
    for (u64 i = start + 1; i < end; i++) {
        acc = func(acc, table->data[i * table->width + column_index]);
    }

    return acc;
}

void data_table_reduce_all_column(DataTable* table, DataTable_Reduce_Function* func) {
    u64 old_height = table->display_height;
    for (u64 i = 0; i < table->display_width; i++) {
        DataTable_Entry result = data_table_column_reduce(table, i, 0, old_height, func);
        data_table_set(table, i, old_height, result);
    }
}

void data_table_reduce_all_row(DataTable* table, DataTable_Reduce_Function* func) {
    u64 old_width = table->display_width;
    for (u64 i = 0; i < table->display_height; i++) {
        DataTable_Entry result = data_table_row_reduce(table, i, 0, old_width, func);
        data_table_set(table, old_width, i, result);
    }
}




typedef u8 DataTable_Compare(DataTable_Entry a, DataTable_Entry b);

// todo: solve these copy pasta
void data_table_sort_row_helper(DataTable* table, s64 start, s64 end, u64 column_index, DataTable_Compare* compare) {

    if (start < 0 || start >= end) return;

    DataTable_Entry pivot = table->data[end * table->width + column_index];

    s64 i = start;
    for (s64 j = start; j < end; j++) {
        DataTable_Entry a = table->data[j * table->width + column_index];
        if (compare(a, pivot)) {
            data_table_swap_row(table, i, j);
            i++;
        }
    }

    data_table_swap_row(table, i, end);

    data_table_sort_row_helper(table, start, i - 1, column_index, compare);
    data_table_sort_row_helper(table, i + 1, end,   column_index, compare);
}

void data_table_sort_column_helper(DataTable* table, s64 start, s64 end, u64 row_index, DataTable_Compare* compare) {

    if (start < 0 || start >= end) return;

    DataTable_Entry pivot = table->data[row_index * table->width + end];

    s64 i = start;
    for (s64 j = start; j < end; j++) {
        DataTable_Entry a = table->data[row_index * table->width + j];
        if (compare(a, pivot)) {
            data_table_swap_column(table, i, j);
            i++;
        }
    }

    data_table_swap_column(table, i, end);

    data_table_sort_column_helper(table, start, i - 1, row_index, compare);
    data_table_sort_column_helper(table, i + 1, end,   row_index, compare);
}

// quick sort
void data_table_sort_row(DataTable* table, u64 column_index, u64 start, DataTable_Compare* compare) {
    assert(column_index < table->display_width);
    assert(start        < table->display_width);
    if (table->height < 2) return;
    data_table_sort_row_helper(table, start, (s64) table->display_height - 1, column_index, compare);
}

void data_table_sort_column(DataTable* table, u64 row_index, u64 start, DataTable_Compare* compare) {
    assert(row_index < table->display_height);
    assert(start     < table->display_height);
    if (table->width < 2) return;
    data_table_sort_column_helper(table, start, (s64) table->display_width - 1, row_index, compare);
}




/* ---- Display and Export ---- */

void data_table_debug_print(DataTable* table) {
    for (u64 i = 0; i < table->display_height; i++) {
        printf("| ");
        for (u64 j = 0; j < table->display_width; j++) {
            DataTable_Entry entry = table->data[i * table->width + j];
            switch (entry.type) {
                case DTE_TYPE_STRING:    print(string("@ | "), entry.data.string);    break;
                case DTE_TYPE_UINT64:    printf("%llu | ",     entry.data.uint64);    break;
                case DTE_TYPE_INT64:     printf("%lld | ",     entry.data.int64);     break;
                case DTE_TYPE_FLOAT64:   printf("%f | ",       entry.data.float64);   break;
                case DTE_TYPE_USER_DATA: printf("%p | ",       entry.data.user_data); break;
                default:                 printf("  | ");                              break;
            }
        }
        printf("\n");
    }
    printf("\n");
}

// todo: validate, version number? user pointers? more error checking?
u8 data_table_save_file(DataTable* table, char* file_name) {

    FILE* f = fopen(file_name, "wb");
    if (!f) return 0; 

    u64 acc = 0;
    acc += file_print(f, string("datatabl")); // data table header
    
    acc += file_print(f, data_string(table->display_width));
    acc += file_print(f, data_string(table->display_height));
    acc += file_print(f, data_string(table->width));
    acc += file_print(f, data_string(table->height));
    
    u64 section_count_pos = acc;
    
    u64 string_section_count = 0; // fill 0 first, then write the actual number later
    acc += file_print(f, data_string(string_section_count));
    
    for (u64 i = 0; i < table->height; i++) {
        for (u64 j = 0; j < table->width; j++) {
            DataTable_Entry entry = table->data[i * table->width + j];
            if (entry.type == DTE_TYPE_STRING) {
                acc += file_print(f, entry.data.string); 
            }
        }
    }
    
    // write the actual string_section_count
    string_section_count = acc - section_count_pos - sizeof(u64);
    fseek(f, section_count_pos, SEEK_SET);
    fwrite(&string_section_count, sizeof(u64), 1, f);
    fseek(f, 0, SEEK_END);
    
    // we will write the bad string pointers into the file, 
    // but correcting/clearing all of them is more complicated and slower than just dump the whole table
    acc += file_print(f, (String) { (u8*) table->data, table->width * table->height * sizeof(DataTable_Entry) });

    fflush(f);
    fclose(f);

    return 1;
}

// todo: validate, more error checking?
String data_table_load_file(DataTable* table, char* file_name) { // fill the table, return file content
    
    String start = load_file(file_name); 
    if (!start.count) return (String) {0};
    
    String file  = start;
    
    String header = string_eat(&file, 8);
    if (!string_equal(header, string("datatabl"))) goto fail;
    
    String display_width_string = string_eat(&file, sizeof(u64)); 
    if (display_width_string.count < sizeof(u64)) goto fail;

    String display_height_string = string_eat(&file, sizeof(u64)); 
    if (display_height_string.count < sizeof(u64)) goto fail;
    
    String width_string = string_eat(&file, sizeof(u64)); 
    if (width_string.count < sizeof(u64)) goto fail;
    
    String height_string = string_eat(&file, sizeof(u64)); 
    if (height_string.count < sizeof(u64)) goto fail;

    String string_section_count_string = string_eat(&file, sizeof(u64)); 
    if (string_section_count_string.count < sizeof(u64)) goto fail;
    
    u64 string_section_count = *(u64*) string_section_count_string.data;
    
    String string_section = string_eat(&file, string_section_count); 
    if (string_section.count != string_section_count) goto fail;
    
    u64 width          = *(u64*) width_string.data; 
    u64 height         = *(u64*) height_string.data; 
    u64 display_width  = *(u64*) display_width_string.data;
    u64 display_height = *(u64*) display_height_string.data;

    if (display_width > width || display_height > height)       goto fail;
    if (file.count != width * height * sizeof(DataTable_Entry)) goto fail;
    
    // correct string pointers
    DataTable_Entry* data = (DataTable_Entry*) file.data;
    u64 acc = 0;
    for (u64 i = 0; i < height; i++) {
        for (u64 j = 0; j < width; j++) {
            DataTable_Entry* entry = &data[i * width + j];
            if (entry->type == DTE_TYPE_STRING) {
                entry->data.string.data = string_section.data + acc;
                acc += entry->data.string.count;
            }
        }
    }

    *table = (DataTable) {
        .data           = data,
        .display_width  = display_width,
        .display_height = display_height,
        .width          = width,
        .height         = height,
        .allocated      = width * height,
        .is_from_file   = 1,
    };

    return start;

    fail:
    free(start.data);
    return (String) {0};
}


u8 data_table_export_csv(DataTable* table, char* file_name, Array(String) header) {

    FILE* f = fopen(file_name, "wb");
    if (!f) return 0; 
    
    if (header.count) {
        assert(header.count == table->display_width);

        for (u64 i = 0; i < header.count; i++) {
            file_print(f, header.data[i]);
            file_print(f, string(","));
        }
        fprintf(f, "\n");
    }

    for (u64 i = 0; i < table->display_height; i++) {
        for (u64 j = 0; j < table->display_width; j++) {
            DataTable_Entry entry = table->data[i * table->width + j];
            switch (entry.type) {
                case DTE_TYPE_STRING:  {
                    file_print(f, entry.data.string);
                    file_print(f, string(","));
                    break;
                }
                case DTE_TYPE_UINT64:  fprintf(f, "%llu,",     entry.data.uint64);   break;
                case DTE_TYPE_INT64:   fprintf(f, "%lld,",     entry.data.int64);    break;
                case DTE_TYPE_FLOAT64: fprintf(f, "%f,",       entry.data.float64);  break;
                default:               fprintf(f, ",");                              break;
            }
        }
        fprintf(f, "\n");
    }

    fflush(f);
    fclose(f);
    
    return 1;
}

u8 data_table_export_js(DataTable* table, char* var_name, char* file_name, u8 row_major_order) {

    FILE* f = fopen(file_name, "wb");
    if (!f) return 0; 
    
    fprintf(f, "let %s = [\n", var_name);
    
    // todo: solve this copy pasta
    if (row_major_order) {
        
        for (u64 i = 0; i < table->display_height; i++) {
            fprintf(f, "[");
            for (u64 j = 0; j < table->display_width; j++) {
                DataTable_Entry entry = table->data[i * table->width + j];
                switch (entry.type) {
                    case DTE_TYPE_STRING:  {
                        file_print(f, string("\""));
                        file_print(f, entry.data.string);
                        file_print(f, string("\","));
                        break;
                    }
                    case DTE_TYPE_UINT64:  fprintf(f, "%llu,",     entry.data.uint64);   break;
                    case DTE_TYPE_INT64:   fprintf(f, "%lld,",     entry.data.int64);    break;
                    case DTE_TYPE_FLOAT64: fprintf(f, "%f,",       entry.data.float64);  break;
                    default:               fprintf(f, ",");                              break;
                }
            }
            fprintf(f, "],\n");
        }
        fprintf(f, "]\n");

    } else {
    
        for (u64 j = 0; j < table->display_width; j++) {
            fprintf(f, "[");
            for (u64 i = 0; i < table->display_height; i++) {
                DataTable_Entry entry = table->data[i * table->width + j];
                switch (entry.type) {
                    case DTE_TYPE_STRING:  {
                        file_print(f, string("\""));
                        file_print(f, entry.data.string);
                        file_print(f, string("\","));
                        break;
                    }
                    case DTE_TYPE_UINT64:  fprintf(f, "%llu,",     entry.data.uint64);   break;
                    case DTE_TYPE_INT64:   fprintf(f, "%lld,",     entry.data.int64);    break;
                    case DTE_TYPE_FLOAT64: fprintf(f, "%f,",       entry.data.float64);  break;
                    default:               fprintf(f, ",");                              break;
                }
            }
            fprintf(f, "],\n");
        }
        fprintf(f, "]\n");
    }
    
    fflush(f);
    fclose(f);
    
    return 1;
}

// todo: add additional headers?
u8 data_table_export_html(DataTable* table, char* file_name) {

    const char* html_template_start = 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "  <meta charset=\"utf-8\">\n"
        "  <head>\n"
        "    <style>\n"
        "      html {\n"
        "        padding: 0;\n"
        "      }\n"
        "      body {\n"
        "        font-family: \"Roboto Mono\", \"Consolas\", monospace;\n"
        "        font-size: 16px;\n"
        "        background-color: #000;\n"
        "        color: #eee;\n"
        "        padding: 10px;\n"
        "        margin: 0;\n"
        "      }\n"
        "      table {\n"
        "        table-layout: fixed;\n"
        "      }\n"
        "      thead {\n"
        "        text-align: left;\n"
        "      }\n"
        "      td {\n"
        "        background-color: #222;\n"
        "        padding: 10px;\n"
        "        font-size: 16px;\n"
        "        min-width: 40px;\n"
        "        min-height: 40px;\n"
        "      }\n"
        "    </style>\n"
        "  </head>\n"
        "  <body>\n"
        "    <table>\n"
    ;

    const char* html_template_end = 
        "    </table>\n"
        "  </body>\n"
        "</html>\n"
    ;

    FILE* f = fopen(file_name, "wb");
    if (!f) return 0; 
    
    fputs(html_template_start, f);
    
    fprintf(f, "<tbody>");
    for (u64 i = 0; i < table->display_height; i++) {
        fprintf(f, "<tr>");
        for (u64 j = 0; j < table->display_width; j++) {
            DataTable_Entry entry = table->data[i * table->width + j];
            switch (entry.type) {
                case DTE_TYPE_STRING:  {
                    fputs("<td>", f);
                    file_print(f, entry.data.string);
                    fputs("</td>", f);
                    break;
                }
                case DTE_TYPE_UINT64:  fprintf(f, "<td>%llu</td>",     entry.data.uint64);   break;
                case DTE_TYPE_INT64:   fprintf(f, "<td>%lld</td>",     entry.data.int64);    break;
                case DTE_TYPE_FLOAT64: fprintf(f, "<td>%f</td>",       entry.data.float64);  break;
                default:               fprintf(f, "<td></td>");                              break;
            }
        }
        fprintf(f, "</tr>\n");
    }
    fprintf(f, "</tbody>");

    fputs(html_template_end, f);
 
    fflush(f);
    fclose(f);

    return 1;
}


