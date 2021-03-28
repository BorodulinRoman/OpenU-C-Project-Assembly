#include "assembler.h"

/********************************************//**
 * main fail manages first, second read and file I\O
 *
 * written by:
 *  Borodulin Roman - 320821648
 ***********************************************/



int main(int argc, char* argv[])
{
    int i;

    if (argc < 2)
    {
        printf("[INFO] No files were supplied.\nExiting...\n");
    }

    for (i = 1; i < argc; i++)
    {
        FILE* file;
        if (argc == i)
        {
            return 1;
        }


        printf("[INFO] Start of file parsing:%s\n", argv[i]);

        if ((file = file_open(argv[i], ".as", "r")) != NULL)
        {
            int IC = 100, DC = 0, error_cout = 0, line_count = 0;
            line_node* line_list_head = NULL;
            label_node* label_head_list = NULL;
            extern_list* extern_list_head = NULL;

            /*first read*/
            first_read(file, &line_list_head, &error_cout, &line_count, &IC, &DC);
            printf("[INFO] First read OK.\n");
            /*create label list*/
            label_head_list = create_label_list(line_list_head);

            if (error_cout > 0) {
                printf("[INFO] Errors found on first read.\nExiting...\n");
            }
            else
            {
                /*second read*/
                printf("[INFO] Starting second read...\n");
                second_read(line_list_head, &extern_list_head, label_head_list, &error_cout);

                if (error_cout > 0) {
                    printf("[INFO] Errors found on second read.\nExiting...\n");
                }
                else
                {
                    /*print files here*/
                    create_files(extern_list_head, label_head_list, argv[i], IC, DC);
                    printf("[INFO] Finished parsing file:%s\n", argv[i]);
                }

            }

            fclose(file);
            free_data();
        }
    }
    return 1;
}
