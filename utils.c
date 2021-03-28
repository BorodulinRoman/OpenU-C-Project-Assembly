#include "assembler.h"
#include <ctype.h> 
#include <stdio.h> 

/********************************************//**
 * \brief copy number to data list
 *
 * \return none
 ***********************************************/
void cpy_num2data_list(int num, img_node* new_img_node, int isReg)
{
    if (isReg)
    {
        new_img_node->data = ONE;
        new_img_node->data = new_img_node->data << num;
        new_img_node->ARE = 'A';

    }
    else if (num < 0)
    {
        num = 4095 + num + 1;
        new_img_node->data = num;
        new_img_node->ARE = 'A';
    }
    else
    {
        new_img_node->data = num;
        new_img_node->ARE = 'A';
    }
}

/********************************************//**
 * \brief check if label is external
 *
 * \return true if it is else false
 ***********************************************/
int is_external_label(label_node* label_list_head, char* token)
{
    label_node* curr_label_node;

    curr_label_node = label_list_head;

    while (curr_label_node != NULL)
    {
        if (!strcmp(curr_label_node->label, token) && !strcmp(curr_label_node->dirc_type, "extern"))
        {
            return TRUE;
        }
        curr_label_node = curr_label_node->next_node;
    }
    return FALSE;
}

/********************************************//**
 * \brief takes a set of tokens and creates, initializes line node with tokens set
 *
 * \return the newly created line node
 ***********************************************/
line_node* insert_set2line_list(line_node** line_node_head, int* line_count, char*** token_set, int* token_count)
{
    line_node* new_line_node;

    new_line_node = create_line_node(line_node_head);

    new_line_node->num_tokenz = (*token_count);
    new_line_node->line_num = (*line_count);
    new_line_node->tokenz = (*token_set);
    new_line_node->tok_idx = 0;

    return new_line_node;
}

/********************************************//**
 * \brief check if a line of assembly code is a comment or an empty line
 *
 * \return TRUE if it is else FALSE
 ***********************************************/

int is_comment_empty(char* tmp_line, int* line_count)
{
    char* str_ptr;

    str_ptr = tmp_line;

    if (*str_ptr == ';')
    {
        return TRUE;
    }

    while (*str_ptr != '\n')
    {
        if (*str_ptr == ';')
        {
            print_error(*line_count, "Comment line should start with \';\'.");
            return TRUE;
        }

        if (*str_ptr != '\t' && *str_ptr != ' ')
        {
            return FALSE;
        }
        str_ptr++;
    }
    return TRUE;
}
/********************************************//**
 * \brief check if register addressing and if legal register
 *
 * \return TRUE if register addressing else FALSE
 ***********************************************/
int is_register_addressing(line_node* curr_line_node, char* token)
{
    if (*token == 'r')
    {
        int reg_num;

        token++;
        reg_num = atoi(token);
        if (reg_num >= 0 && reg_num <= 7)
        {
            return TRUE;
        }
        else
        {
            print_error(curr_line_node->line_num, "unrecognized register.");
            curr_line_node->error_flag = TRUE;
            return FALSE;
        }
    }
    return FALSE;
}
/********************************************//**
 * \brief check if label is existing and external
 *
 * \return TRUE if label existing and external else FALSE
 ***********************************************/
int is_label_ext(line_node* line_list_head, char* token)
{
    line_node* line_node_ptr;

    line_node_ptr = line_list_head;

    while (line_node_ptr != NULL)
    {
        if (line_node_ptr->label != NULL)
        {
            if (!strcmp(line_node_ptr->label->label, token) && (!strcmp(line_node_ptr->label->dirc_type, "extern")))
            {
                return TRUE;
            }
        }
        line_node_ptr = line_node_ptr->next_node;
    }
    return FALSE;
}
/********************************************//**
 * \brief check if label
 *
 * \return true if it is else false
 ***********************************************/
int is_label(char* token)
{
    int result = FALSE;

    if (isupper(*token))
    {

        result = TRUE;
        while (*token && (*token != 13))
        {

            if (isalnum(*token))
            {

                result = TRUE;
            }
            else
            {
                return FALSE;
            }
            token++;
        }
    }
    return result;
}
/********************************************//**
 * \brief check if operand is relative addressing
 *
 * \return TRUE if it is else FALSE
 ***********************************************/
int is_relative_addressing(line_node* curr_line_node, char* token)
{
    int result = FALSE;
    char* tmp_ptr = token;

    if (*tmp_ptr == '%')
    {
        result = TRUE;
        result = is_label((++tmp_ptr));

        if (result == FALSE)
        {
            print_error(curr_line_node->line_num, "label should be linked together with %.");
            curr_line_node->error_flag = TRUE;
        }
    }
    return result;
}
/********************************************//**
 * \brief check if direct addressing
 *
 * \return TRUE if is else FALSE
 ***********************************************/
int is_direct_addressing(char* token)
{
    int result = FALSE;

    result = is_label(token);
    return result;
}
/********************************************//**
 * \brief check if immidiate addressing
 *
 * \return TRUE if it is else FALSE
 ***********************************************/
int is_immidiate_addressing(line_node* curr_line_node, char* token)
{

    int result = FALSE;
    char* tmp_ptr = token;


    if (*tmp_ptr == '#')
    {
        tmp_ptr++;
        while (*tmp_ptr != '\0')
        {
            if (*tmp_ptr == '-' || isdigit(*tmp_ptr))
            {
                result = TRUE;
            }
            else
            {
                result = FALSE;
                break;
            }
            tmp_ptr++;
        }
        if (strlen(token) < 2)
        {
            print_error(curr_line_node->line_num, "immidiate addressing operands must be without spaces.");
            curr_line_node->error_flag = TRUE;
        }
    }
    return result;
}


/********************************************//**
 * \brief a checker to determine dirctive type
 *
 * \return a macro of a type of action
 ***********************************************/
int dirc_type(line_node* curr_line_node)
{
    int j;
    char* token;

    token = curr_line_node->tokenz[0];

    for (j = 0; j < (curr_line_node->num_tokenz); j++)
    {
        token = curr_line_node->tokenz[j];
        if ((strstr(token, ".data") || strstr(token, ".string")))
        {
            return DATA_STRING_DIRC;
        }
        else  if (strstr(token, ".entry"))
        {
            return ENTRY_DIRC;
        }
        else  if (strstr(token, ".extern"))
        {
            return EXTERN_DIRC;
        }
    }
    return COMMAND_LINE;
}
/********************************************//**
 * \brief used to search for label in assembly code line
 *
 * \return pointer to the label else NULL
 ***********************************************/
char* get_label(line_node* curr_label_node)
{
    int i;
    char* token;

    for (i = 0; curr_label_node->num_tokenz > i; i++)
    {
        token = curr_label_node->tokenz[i];
        if (strchr(token, ':'))
        {
            if (!((token[0] > 64 && token[0] < 91) || (token[0] > 96 && token[0] < 123)))
            {
                printf("[ERROR] label  %s strats with namber!.\n", curr_label_node->tokenz[i]);
                curr_label_node->error_flag = TRUE;
            }
            return (strtok(token, ":"));
        }
    }
    return NULL;
}

/********************************************//**
 * \brief check to see if line has a label deceleration and if written correctly
 *
 * \return True if it does else False
 ***********************************************/
int is_label_decleration(line_node* curr_line_node)
{
    int i;
    int tok_idx = curr_line_node->tok_idx;
    char* token = curr_line_node->tokenz[tok_idx];

    if (strchr(token, ':'))
    {


        for (i = 0; (strlen(token)) > i; i++)
        {
            if (!isalnum(token[i]) && !(token[i] == ':'))
            {
                print_error(curr_line_node->line_num, "valid label contains only A-Z,a-z,0-9 characters.");
                curr_line_node->error_flag = TRUE;
            }
        }
        for (i = 0; i < NUM_CMD; i++)
        {
            if (strstr(token, cmd_array[i].cmd_name))
            {
                print_error(curr_line_node->line_num, "label name can't be a command name.");
                curr_line_node->error_flag = TRUE;
            }
        }
        if (*token == 'r' && atoi(token + 1) >= 0 && atoi(token + 1) < 8)
        {
            print_error(curr_line_node->line_num, "label name can't be a register 0-7.");
            curr_line_node->error_flag = TRUE;
        }
        return TRUE;
    }
    return FALSE;
}
/********************************************//**
 * \brief handle file opening
 *
 * \return pointer to opened file else NULL
 ***********************************************/
FILE* file_open(char* file_name, char* file_type, char* mode)
{
    if (strstr((const char*)file_name, (const char*)file_type) != NULL)
    {
        FILE* file = fopen(file_name, mode);
        if (file == NULL)
        {
            printf("[ERROR] File:%s failed to open.\n", file_name);
            return  NULL;
        }
        else
        {
            printf("[INFO] Successfully opened file:%s\n", file_name);
            return file;
        }
    }
    printf("[ERROR] Argument:%s is not \"%s\" type. Skipping...\n", file_name, file_type);
    return NULL;
}
/********************************************//**
 * \brief print error formated message
 *
 * \return None
 ***********************************************/
void print_error(int line_num, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    printf("[ERROR] In line %d: ", line_num);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}








