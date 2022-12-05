#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>


#include "context.c"
#include "string.c"
#include "data_table.c"






void program() {


    DataTable table = data_table_init(8, 8);

    String temp_file = {0};

    u32 display_method = 0; // 0 = term, 1 = html, 2 = both


    while (1) {
        
        temp_reset();
        
        printf("> ");
        String line = read_line();
        String full_line = line;
        
        String command = string_eat_by_spaces(&line);
        
        if (string_equal(command, string("help"))) {
            printf(
                "Available Commands:\n"
                "    help\n"
                "    exit | quit\n"
                "    print\n"
                "    save <filename>\n"
                "    load <filename>\n"
                "    display <mode>\n"
                "    shrink\n"
                "    set <x> <y> <value>\n"
                "    clear <x> <y>\n"
                "    clear row <index>\n"
                "    clear col <index>\n"
                "    swap row <a> <b>\n"
                "    swap col <a> <b>\n"
            );
            goto done;
        }
       
        if (string_equal(command, string("exit")) || string_equal(command, string("quit"))) break;
        
        if (!command.count || string_equal(command, string("print"))) goto done;
        
        if (string_equal(command, string("save"))) {
            
            String path = string_eat_by_spaces(&line);

            if (!path.count) {
                printf("[File] No path was given.\n");
                goto done;
            }
            
            u8 ok = data_table_save_file(&table, temp_c_string(path));
            if (!ok) print(string("[File] Unable to save file @\n"), path);
            else     print(string("[File] Saved file @\n"), path);
            
            goto done;
        }
        
        if (string_equal(command, string("load"))) {
            
            String path = string_eat_by_spaces(&line);
            if (!path.count) {
                printf("[File] No path was given.\n");
                goto done;
            }
            
            String file = data_table_load_file(&table, temp_c_string(path));
            if (!file.count) {
                print(string("[File] Unable to load file @\n"), path);
                goto done;
            }
            
            if (temp_file.count) free(temp_file.data); // ehh.....
            temp_file = file;
            print(string("[File] Loaded file @\n"), path);
            goto done;
        }

        if (string_equal(command, string("display"))) {
            
            String method = string_eat_by_spaces(&line);
            
            if (string_equal(method, string("term")) || string_equal(method, string("terminal"))) {
                display_method = 0;
            } else if (string_equal(method, string("html"))) {
                display_method = 1;
            } else if (string_equal(method, string("both"))) {
                display_method = 2;
            } else if (!method.count) {
                print(string("[Display] No method provided!\n"));
                goto next;
            } else {
                print(string("[Display] Unknown display method: \"@\"\n"), method);
                goto next;
            }

            goto done;
        }
        
        if (string_equal(command, string("shrink"))) {
            data_table_shrink_unused(&table);
            goto done; 
        }
        
        if (string_equal(command, string("clear"))) {
            
            String next = string_eat_by_spaces(&line);
            
            // first, check if we want to clear row or column
            if (string_equal(next, string("row"))) {
           
                String index_string = string_eat_by_spaces(&line);
                
                u64 index;
                if (!parse_u64(index_string, &index)) goto invalid_command;

                data_table_clear_row(&table, index);
                goto done;
            
            } else if (string_equal(next, string("col")) || string_equal(next, string("column"))) {

                String index_string = string_eat_by_spaces(&line);
                
                u64 index;
                if (!parse_u64(index_string, &index)) goto invalid_command;

                data_table_clear_column(&table, index);
                goto done;
            } 
            
            // if not col/column, clear one entry (next is now x_string)
            String x_string = next;
            String y_string = string_eat_by_spaces(&line);
            
            u64 x, y;
            if (!parse_u64(x_string, &x)) goto invalid_command;
            if (!parse_u64(y_string, &y)) goto invalid_command;
        
            data_table_clear(&table, x, y);
            goto done;
        }
        
        if (string_equal(command, string("set"))) {
            
            String x_string   = string_eat_by_spaces(&line);
            String y_string   = string_eat_by_spaces(&line);
            String arg_string = string_eat_by_spaces(&line);
            
            u64 x, y;
            if (!parse_u64(x_string, &x)) goto invalid_command;
            if (!parse_u64(y_string, &y)) goto invalid_command;
            
            if (arg_string.count) {
                
                u64 uint64;
                if (parse_u64(arg_string, &uint64)) {
                    data_table_set_uint64(&table, x, y, uint64);
                    goto done;
                }
                
                s64 int64;
                if (parse_s64(arg_string, &int64)) {
                    data_table_set_int64(&table, x, y, int64);
                    goto done;
                }
                 
                f64 float64;
                if (parse_f64(arg_string, &float64)) {
                    data_table_set_float64(&table, x, y, float64);
                    goto done;
                }

                data_table_set_string(&table, x, y, string_copy(arg_string)); // leak!!!!
                goto done;
            
            } else {
                
                goto invalid_command;
            }
        }
        
        if (string_equal(command, string("swap"))) {
            
            String direction = string_eat_by_spaces(&line);
            String a_string  = string_eat_by_spaces(&line);
            String b_string  = string_eat_by_spaces(&line);
            
            u64 a, b;
            if (!parse_u64(a_string, &a)) goto invalid_command;
            if (!parse_u64(b_string, &b)) goto invalid_command;
        
            if (string_equal(direction, string("row"))) {
                data_table_swap_row(&table, a, b);
                goto done;
            } else if (string_equal(direction, string("col")) || string_equal(direction, string("column"))) {
                data_table_swap_column(&table, a, b);
                goto done;
            } else {
                goto invalid_command;
            }
        }
        

        invalid_command:
        print(string("[Error] Invalid command: \"@\". Enter \"help\" to get list of commands.\n"), full_line);
        next: continue;


        done: // render
        switch (display_method) {
            
            case 0: 
            {
                printf("\n[Table]\n");
                data_table_debug_print(&table); 
                break;
            }
            
            case 1: 
            {
                data_table_export_html(&table, "test.html"); 
                printf("[File] Wrote \"test.html\"\n");
                break;
            }
            
            case 2: 
            {
                printf("[File] Wrote \"test.html\"\n");
                printf("\n[Table]\n");
                data_table_debug_print(&table); 
                data_table_export_html(&table, "test.html");
                break;
            }
            
            default: break;
        }

    }


    // safe exit
    printf("Save filename? (Enter to skip and don't save)\n> ");
    String line = read_line();
    if (line.count) {
        String path = string_eat_by_spaces(&line);
        u8 ok = data_table_save_file(&table, temp_c_string(path));
        if (!ok) print(string("[File] Unable to save file @\n"), path); // this is not right, should ask again
    }


}


