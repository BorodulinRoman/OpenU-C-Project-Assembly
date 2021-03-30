#include "assembler.h"

/********************************************//**
 * main fail manages first, second read and file I\O
 *
 * written by:
 *  Borodulin Roman - 320821648
 ***********************************************/



void free_data()
{
    free(g_inst_head);
    free(g_data_head);
    g_inst_head = NULL;
    g_data_head = NULL;
}

/********************************************//**
 * \brief converts integer to 12 bit hex and saves to buffer
 *
 * \return None
 ***********************************************/
void int2Base16(int num, char* buf, int index)
{
    const char digits[] = "0123456789ABCDEF";
    int i;
    char byte_mask = 0xf;

    buf[FOUR_BITS] = '\0';
    for (i = THREE_BYTES - 1; i >= 0; i--)
    {

        buf[i] = digits[num & byte_mask];
        num >>= FOUR_BITS;
    }
}

/********************************************//**
 * \brief prints hex number to file
 *
 * \return void
 ***********************************************/
void print2file_hex(FILE* file, int num)
{
    char buf[FOUR_BITS + 1] = { 0 };
    int2Base16(num, buf, 0);
    fprintf(file, "%s", buf);
}

/********************************************//**
 * \brief prints to file address with zeros padding and prints hex
 *
 * \return None
 ***********************************************/
void img_print2file(FILE* file, img_node* curr_node)
{
    fprintf(file, "%04d ", curr_node->address);
    print2file_hex(file, curr_node->data);
    fprintf(file, " %c\n", curr_node->ARE);
}

/********************************************//**
 * \brief open file with name of .as file
 *
 * \return pointer to file opened
 ***********************************************/
FILE* open_file(char* file_name, char* file_ending, const char* mode)
{
    FILE* file;
    char* full_file_name = (char*)malloc((strlen(file_name) + strlen(file_ending) + 1) * sizeof(*full_file_name));
    sprintf(full_file_name, "%s%s", file_name, file_ending);

    file = fopen(full_file_name, mode);
    free(full_file_name);

    return file;
}

/********************************************//**
 * \brief create entry files, if there any entries
 *
 * \return None
 ***********************************************/
void create_entry_file(char* file_name, label_node* label_head_list)
{
    label_node* curr_label_node = label_head_list;
    FILE* file;

    while (curr_label_node != NULL)
    {
        if (curr_label_node->entry_flag)
        {
            break;
        }
        curr_label_node = curr_label_node->next_node;
    }
    /*no entries - no file will be created*/
    if (curr_label_node == NULL)
    {
        return;
    }

    file = open_file(file_name, ".ent", "w");
    curr_label_node = label_head_list;
    while (curr_label_node != NULL)
    {
        if (curr_label_node->entry_flag)
        {
            fprintf(file, "%s %04d\n", curr_label_node->label, curr_label_node->address);
        }
        curr_label_node = curr_label_node->next_node;
    }
    fclose(file);
}

/********************************************//**
 * \brief creates object file
 *
 * \return None
 ***********************************************/
void create_object_file(char* file_name, int IC, int DC)
{
    FILE* file;
    img_node* curr_inst_node, * curr_data_node;

    file = open_file(file_name, ".ob", "w");
    curr_data_node = g_data_head;
    curr_inst_node = g_inst_head;

    /* Print IC with leading spaces and DC */
    fprintf(file, "%4.d %d\n", IC - FIRST_ADDRESS, DC);

    while (curr_inst_node != NULL)
    {
        img_print2file(file, curr_inst_node);
        curr_inst_node = curr_inst_node->next_node;
    }
    while (curr_data_node != NULL)
    {
        img_print2file(file, curr_data_node);
        curr_data_node = curr_data_node->next_node;
    }


    fclose(file);
}

/********************************************//**
 * \brief create extern labels file, if any exists
 *
 * \return none
 ***********************************************/
void create_extern_file(char* file_name, extern_list* extern_list_head)
{
    extern_list* curr_extern_node = extern_list_head;
    FILE* file;

    /*no external variables- no file will be created*/
    if (curr_extern_node == NULL)
    {
        return;
    }
    file = open_file(file_name, ".ext", "w");
    while (curr_extern_node != NULL)
    {
        fprintf(file, "%s %04d\n", curr_extern_node->label, curr_extern_node->address);
        curr_extern_node = curr_extern_node->next_node;
    }
    fclose(file);
}

/********************************************//**
 * \brief creates files: object, entries and extern.
 *
 * \return none
 ***********************************************/

void create_files(extern_list* extern_list_head, label_node* label_head_list, char* file_name, int IC, int DC)

{

    file_name = strtok(file_name, ".");
    create_object_file(file_name, IC, DC);
    create_entry_file(file_name, label_head_list);
    create_extern_file(file_name, extern_list_head);
}


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
