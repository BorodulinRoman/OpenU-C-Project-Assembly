
/********************************************//**
 * first read manages line reading, line parsing and creating data table, instructions table, label table.
 *
 * written by:
 *  Borodulin Roman - 320821648
 ***********************************************/

#include "assembler.h"
const cmd_type cmd_array[NUM_CMD] = {
    {"mov" ,  0, 0, Two_operand,ARE_A},
    {"cmp" ,  1, 0, Two_operand,ARE_A},
    {"add" ,  2, 10, Two_operand,ARE_A},
    {"sub" ,  2, 11, Two_operand,ARE_A},
    {"lea" ,  4, 0, Two_operand,ARE_A},
    {"clr" ,  5, 10, One_operand,ARE_A},
    {"not" ,  5, 11, One_operand,ARE_A},
    {"inc" ,  5, 12, One_operand,ARE_A},
    {"dec" ,  5, 13, One_operand,ARE_A},
    {"jmp" ,  9, 10, One_operand,ARE_A},
    {"bne" ,  9, 11, One_operand,ARE_A},
    {"jsr" ,  9, 12, One_operand,ARE_A},
    {"red" , 12, 0, One_operand,ARE_A},
    {"prn" , 13, 0, One_operand,ARE_A},
    {"rts" , 14, 0, No_operand,ARE_A},
    {"stop", 15, 0, No_operand,ARE_A}
};
/********************************************//**
 * \brief updates data address
 *
 * \return none
 ***********************************************/
void update_data_address(int IC)
{
    img_node* curr_data_node;
    curr_data_node = g_data_head;

    while (curr_data_node != NULL)
    {
        curr_data_node->address += (IC);
        curr_data_node->ARE = ARE_A;
        curr_data_node = curr_data_node->next_node;
    }

}

/********************************************//**
 * \brief updates abeel address with IC
 *
 * \return none
 ***********************************************/
void update_label_address(line_node* line_list_head, int* IC)
{
    line_node* curr_node = line_list_head;

    while (curr_node != NULL)
    {
        if (curr_node->label_flag && !strcmp(curr_node->label->dirc_type, "data"))
        {
            curr_node->label->address += *IC;
        }
        curr_node = curr_node->next_node;
    }
}

/********************************************//**
 * \brief simple search of cmd in array
 *
 * \return index of cmd in array or -1 if failed
 ***********************************************/

int search_cmd(char* p_token)
{
    int i;
    for (i = 0; i < NUM_CMD + 1; i++)
    {
        if (!strcmp(p_token, cmd_array[i].cmd_name))
        {
            return i;
        }
    }
    return -1;
}

/********************************************//**
 * \brief add opcode and function bits
 *
 * \return none
 ***********************************************/
void add_opcode_and_funct(int cmd_idx, img_node* curr_inst_node)
{
    int inst_12bit = 0;

    inst_12bit |= (cmd_array[cmd_idx].cmd_code << OPCODE_BITS);
    inst_12bit |= (cmd_array[cmd_idx].cmd_func << FUNCT_BITS);

    curr_inst_node->data |= inst_12bit;
    curr_inst_node->ARE = ARE_A;
    return;
}

/********************************************//**
 * \brief determine operand type
 *
 * \return type of operand defined in header file or -1 if failed
 ***********************************************/
int op_type(line_node* line_list_head, line_node* curr_line_node, char* token)
{
    if (is_immidiate_addressing(curr_line_node, token))
    {
        return IMMIDIATE_ADDRESSING;
    }
    if (is_direct_addressing(token))
    {
        return DIRECT_ADDRESSING;
    }
    if (is_relative_addressing(curr_line_node, token))
    {
        if (is_label_ext(line_list_head, ++token))
        {
            print_error(curr_line_node->line_num, "relative addressing allows only labels defined in source code.");
            curr_line_node->error_flag = TRUE;
            return -1;
        }
        return RELATIVE_ADDRESSING;
    }
    if (is_register_addressing(curr_line_node, token))
    {
        return DIRECT_REGISTER_ADDRESSING;
    }
    return -1;
}

/********************************************//**
 * \brief function to parsing operands, for each operand determine addressing and input into instruction word
 *
 * \return None
 ***********************************************/
void pars_oprnd(line_node* line_list_head, line_node* curr_line_node, img_node* curr_inst_node, int cmd_idx, int* IC)
{
    int op_count = 0;
    char* token;
    char* empty_token = NULL;
    char* cmd_name = NULL;

    /*while there is operands*/
    while (curr_line_node->num_tokenz > curr_line_node->tok_idx)
    {
        /*take next token*/
        token = curr_line_node->tokenz[curr_line_node->tok_idx];
        token = strtok(token, ",\0");
        while (token != NULL)
        {
            if ((++op_count) > cmd_array[cmd_idx].num_op)
            {
                print_error(curr_line_node->line_num, "Too many operands for instruction.");
                curr_line_node->error_flag = TRUE;
                return;
            }

            switch (op_type(line_list_head, curr_line_node, token))
            {
            case IMMIDIATE_ADDRESSING:
                add_addressing_and_registers(curr_inst_node, op_count, IMMIDIATE_ADDRESSING, NO_REG_NEEDED);
                add_num2data_list(token, IC, ARE_A, FALSE);
                break;

            case DIRECT_ADDRESSING:
                if (curr_line_node->num_tokenz < 3)
                    add_addressing_and_registers(curr_inst_node, op_count + 1, DIRECT_ADDRESSING, NO_REG_NEEDED);
                else
                    add_addressing_and_registers(curr_inst_node, op_count, DIRECT_ADDRESSING, NO_REG_NEEDED);
                add_num2data_list(empty_token, IC, 0, FALSE);
                break;

            case RELATIVE_ADDRESSING:
                cmd_name = cmd_array[cmd_idx].cmd_name;

                if (!strcmp(cmd_name, "jsr") || !strcmp(cmd_name, "bne") || !strcmp(cmd_name, "jmp"))
                {
                    if (curr_line_node->num_tokenz < 3)
                        add_addressing_and_registers(curr_inst_node, op_count + 1, RELATIVE_ADDRESSING, NO_REG_NEEDED);
                    else
                        add_addressing_and_registers(curr_inst_node, op_count, RELATIVE_ADDRESSING, NO_REG_NEEDED);
                    add_num2data_list(empty_token, IC, ARE_A, FALSE);
                }
                else
                {
                    print_error(curr_line_node->line_num, "invalid command:%s used with relative addressing.", cmd_name);
                    curr_line_node->error_flag = TRUE;
                }
                break;

            case DIRECT_REGISTER_ADDRESSING:
                if (curr_line_node->num_tokenz < 3)
                    add_addressing_and_registers(curr_inst_node, op_count + 1, DIRECT_REGISTER_ADDRESSING, atoi(++token));
                else
                    add_addressing_and_registers(curr_inst_node, op_count, DIRECT_REGISTER_ADDRESSING, atoi(++token));
                add_num2data_list(token, IC, ARE_A, TRUE);
                break;

            }
            token = strtok(NULL, ",\0");

        }
        curr_line_node->tok_idx++;
    }
    if (op_count < cmd_array[cmd_idx].num_op)
    {
        print_error(curr_line_node->line_num, "Too few operands for instruction.");
        curr_line_node->error_flag = TRUE;
        return;
    }
    /*for command with no operands*/
    if (op_count == 0)
    {
        add_addressing_and_registers(curr_inst_node, op_count, NO_ADDRESSING_NEEDED, NO_REG_NEEDED);
    }

}

/********************************************//**
 * \brief search for command and if valid command with enough operands then add opcode and func code
 *          then pars operands
 * \return Nonde
 ***********************************************/
void pars_inst(line_node* line_list_head, line_node* curr_line_node, int* IC)
{
    img_node* curr_inst_node;
    char* p_token;
    int tok_idx = curr_line_node->tok_idx;
    int cmd_idx = -1;

    p_token = curr_line_node->tokenz[tok_idx];

    if (tok_idx >= curr_line_node->num_tokenz)
    {
        print_error(curr_line_node->line_num, "Too few operands for instruction.");
        curr_line_node->error_flag = TRUE;
        return;
    }
    cmd_idx = search_cmd(p_token);
    if (cmd_idx == -1)
    {
        print_error(curr_line_node->line_num, "unknown command %s.", curr_line_node->label);
        curr_line_node->error_flag = TRUE;
        return;
    }

    curr_inst_node = create_inst_node(IC);
    add_opcode_and_funct(cmd_idx, curr_inst_node);
    curr_line_node->tok_idx++;
    pars_oprnd(line_list_head, curr_line_node, curr_inst_node, cmd_idx, IC);


    return;
}

/********************************************//**
 * \brief pars data of directive line(string\data) and insert to data list
 *
 * \return Nonde
 ***********************************************/
void pars_data(line_node* curr_line_node, int* DC)
{
    char* token_p = curr_line_node->tokenz[curr_line_node->tok_idx];

    if (strstr(token_p, ".string"))
    {
        curr_line_node->tok_idx++;
        insert_string2data_list(DC, curr_line_node);
    }
    else if (strstr(token_p, ".data"))
    {
        curr_line_node->tok_idx++;
        insert_int2data_list(DC, curr_line_node);
    }
}

/********************************************//**
 * \brief insert tokens into set, by allocating array of chars, copying token into it,
 *        reallocating token_set to new memory with the new token
 * \return None
 *
 ***********************************************/
void insert_token2set(char* token, char*** token_set, int* tok_count, int* tok_set_size)
{
    char** token_set_new;
    char* token_new = (char*)malloc(sizeof(*token_new) * (strlen(token) + 1));;

    if (!token_new)
    {
        printf("[ERROR] Failed to allocate memory for token_new.\n");
        return;
    }
    strcpy(token_new, token);

    if (*tok_count >= (*tok_set_size))
    {
        token_set_new = realloc(*token_set, sizeof(*token_set_new) * (*tok_count + 1));
        if (!token_set_new)
        {
            printf("[ERROR] Failed to reallocate memory for token_set.\n");
            return;
        }
        *token_set = token_set_new;

    }

    (*token_set)[*tok_count] = token_new;
    (*tok_set_size)++;

}

/********************************************//**
 * \brief converts string of char to tokens and insert into set(array strings)
 *
 * \param token_set - array of char strings
 * \return None
 *
 ***********************************************/
void tokenize_line(char*** token_set, char* tmp_line, int* tok_count)
{
    char* token;
    char* delims = " ,\t\n";
    int tok_set_size = 0;

    *tok_count = 0;

    token = strtok(tmp_line, delims);

    while (token != NULL)
    {
        insert_token2set(token, token_set, tok_count, &tok_set_size);
        token = strtok(NULL, delims);
        (*tok_count)++;
    }
}

/********************************************//**
 * \brief check if line is a empty or comment and ignore it.
 *        convert line to tokens and store in array of tokens(token_set).
 *        free token_set
 *
 *
 * \param tmp_line - pointer to line in the file.
 * \return None
 *
 ***********************************************/
void line_parser(line_node** line_list_head, char* tmp_line, int* line_count, int* error_count, int* IC, int* DC)
{
    char** token_set = NULL;
    int token_count = 0;
    line_node* curr_line_node;

    /*go to next line if comment or empty line*/
    if (is_comment_empty(tmp_line, line_count))
    {
        return;
    }

    /*create from line token and save to line_node*/
    tokenize_line(&token_set, tmp_line, &token_count);
    curr_line_node = insert_set2line_list(line_list_head, line_count, &token_set, &token_count);

    if (is_label_decleration(curr_line_node))
    {
        curr_line_node->label_flag = TRUE;
    }

    /*analyze directory types*/
    switch (dirc_type(curr_line_node))
    {
    case DATA_STRING_DIRC:
        /*data line with label insert to global data list with "data" directory type of label*/
        if (curr_line_node->label_flag)
        {
            create_label_node(curr_line_node);
            insert_label(curr_line_node, "data", *DC, FALSE);
        }
        /*data line with label insert to global data list*/
        pars_data(curr_line_node, DC);
        break;

    case ENTRY_DIRC:
        break;

    case EXTERN_DIRC:
        /*external label decleration create label node with extern directory type*/
        create_label_node(curr_line_node);
        insert_label(curr_line_node, "extern", 0, TRUE);
        break;

    case COMMAND_LINE:
        /*command line with label decleration crate label with code directory type*/
        if (curr_line_node->label_flag)
        {

            create_label_node(curr_line_node);
            curr_line_node->label->ARE = ARE_A;
            insert_label(curr_line_node, "code", *IC, FALSE);
        }
        /*pars instruction*/
        pars_inst(*line_list_head, curr_line_node, IC);
        break;
    }

    *error_count += curr_line_node->error_flag;
}

/********************************************//**
 * \brief read_line - reads a line from file of max length, replacing new line char with null char
 *       if line is larger then max len keep reading until new line is found or eof
 *
 * \param file - pointer to file
 * \param tmp_line - pointer to string read from a file
 * \param max_len - maximum length of line to read
 * \return TRUE if read line was successful, otherwise FALSE
 *
 ***********************************************/
int read_line(FILE* file, char* tmp_line, int max_len)
{

    char* end_of_line;

    if (!fgets(tmp_line, max_len, file))
    {
        return FALSE;
    }

    end_of_line = strchr(tmp_line, '\n');

    if (!end_of_line)
    {
        char c;
        while ((c = fgetc(file)))
        {
            if (c == '\n' || c == EOF)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}


/********************************************//**
 * \brief first_read - organizes all the actions for the first pass described by the project booklet
 *
 * \param file - pointer to file
 * \param line_list_head - pointer to the start of linked list of type line_node
 *
 * \return no value returned
 *
 ***********************************************/
void first_read(FILE* file, line_node** line_list_head, int* error_count, int* line_count, int* IC, int* DC)
{
    char tmp_line[MAX_LINE_LEN];

    while (1)
    {


        if (read_line(file, tmp_line, MAX_LINE_LEN))
        {
            (*line_count)++;
            line_parser(line_list_head, tmp_line, line_count, error_count, IC, DC);

        }
        else if (!feof(file))
        {
            printf("[ERROR] Line %d is too long, max size of line:%d.\n", *line_count, MAX_LINE_LEN);
            ++(*line_count);
            ++(*error_count);
        }
        if (feof(file))
            break;

    }

    update_data_address(*IC);
    update_label_address(*line_list_head, IC);
}
