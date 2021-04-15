#include "assembler.h"
/********************************************//**
 * linked_list manages all linked list operations
 *
 * written by:
 *  Borodulin Roman - 320821648
 ***********************************************/


/********************************************//**
 * \brief create external label list
 *
 * \return none
 ***********************************************/
void create_extern_node(extern_list **extern_list_head, char *token, img_node *curr_inst_node)
{
    extern_list *curr_extern_node;
    extern_list *new_extern_node;

    curr_extern_node = *extern_list_head;

    if (curr_extern_node == NULL)
    {
        curr_extern_node = (extern_list *)malloc(sizeof(extern_list));;

        if(curr_extern_node == NULL)
        {
            printf("[ERROR] Failed to allocate memory for extern_node_list.\n");
            exit(0);
        }
        *extern_list_head = curr_extern_node;
    }
    else
    {
        while (curr_extern_node->next_node != NULL)
        {
            curr_extern_node = curr_extern_node->next_node;
        }

        new_extern_node = (extern_list *)malloc(sizeof(extern_list));
        if(curr_extern_node == NULL)
        {
            printf("[ERROR] Failed to allocate memory for extern_node_list.\n");
            exit(0);
        }
        curr_extern_node->next_node = new_extern_node;
        curr_extern_node = curr_extern_node->next_node;
    }
    curr_extern_node->address = curr_inst_node->address;
    curr_extern_node->label   = token;
    curr_extern_node->next_node = NULL;

}

/********************************************//**
 * \brief create label list - go throug line list and connect all label struct to create a list
 *
 * \return whole list of label - label list head
 ***********************************************/
label_node *create_label_list(line_node *line_head_list)
{
    line_node *curr_line_node = line_head_list;
    label_node *label_head_list = NULL;
    label_node *label_curr_node = NULL;
    label_node *label_prev_node = NULL;

    while (curr_line_node != NULL)
    {
        if (curr_line_node->label != NULL)
        {
            if (label_head_list == NULL)
            {
                label_head_list  = curr_line_node->label;
                label_curr_node = label_head_list;
                label_prev_node = label_head_list;
            }
            else
            {
                label_prev_node = label_curr_node;
                label_curr_node = curr_line_node->label;
                label_prev_node->next_node = label_curr_node;
            }
        }

        curr_line_node = curr_line_node->next_node;
    }
    return label_head_list;
}

/********************************************//**
 * \brief search for the end of linked list, create, initialize and insert a line_node
 *
 * \return newly created line_node
 ***********************************************/
line_node *create_line_node(line_node **head)
{
    line_node *tail = *head;
    line_node *new_node = (line_node *)malloc(sizeof(line_node));

    if (new_node == NULL)
    {
        printf("[ERROR] Failed to allocate memory for line_list_head.\n");
        exit(0);
    }

    if(tail == NULL)
    {
        *head = new_node;
        tail  = *head;
    }
    else
    {
        while(tail->next_node != NULL)
        {
            tail = tail->next_node;
        }
        tail->next_node = new_node;
        tail = tail->next_node;
    }
    tail->label_flag = FALSE;
    tail->label = NULL;
    tail->error_flag = FALSE;
    tail->line_num   = 0;
    tail->num_tokenz = 0;
    tail->tokenz     = NULL;
    tail->next_node  = NULL;
    tail->tok_idx = 0;
    return tail;
}
/********************************************//**
 * \brief create and initialize label_node, label_node is attached to line_node struct
 *
 * \return None
 ***********************************************/
void create_label_node(line_node *curr_line_node)
{
    label_node *new_node = (label_node *)malloc(sizeof(label_node));

    if (new_node == NULL)
    {
        printf("[ERROR] Failed to allocate memory for label_list_head.\n");
        exit(0);
    }

    new_node->address       = 0;
    new_node->dirc_type     = NULL;
    new_node->label         = NULL;
    new_node->entry_flag    = FALSE;
    new_node->next_node     = NULL;
    new_node->ARE           = 0;
    curr_line_node->label   = new_node;
}
/********************************************//**
 * \brief search for end of linked list of data, create and initialize a new node
 *
 * \return Node
 ***********************************************/
void create_data_node(int *DC, int tok_chr)
{
    img_node *tail = g_data_head;
    img_node *new_node = (img_node *)malloc(sizeof(img_node));

    if (new_node == NULL)
    {
        printf("[ERROR] Failed to allocate memory for line_list_head.\n");
        exit(0);
    }

    if(tail == NULL)
    {
        g_data_head = new_node;
        tail        = g_data_head;
    }
    else
    {
        while(tail->next_node != NULL)
        {
            tail = tail->next_node;
        }
        tail->next_node = new_node;
        tail = tail->next_node;
    }
    tail->address   = (*DC);
    tail->data      = tok_chr;
    tail->next_node = NULL;
    (*DC)++;
}
/********************************************//**
 * \brief pars string data and a null char at the end and insert to data list
 *
 * \return None
 ***********************************************/
void insert_string2data_list(int *DC, line_node *curr_line_node)
{
    char *p_token = curr_line_node->tokenz[curr_line_node->tok_idx];

    while (curr_line_node->tok_idx < curr_line_node->num_tokenz)
    {
        while ((*p_token != '\0')&&(*p_token != 13))
        {
            if(*p_token != '"')
            {
		create_data_node(DC, (int)*p_token);

            }
            p_token++;
        }
        curr_line_node->tok_idx++;
    }
    create_data_node(DC, '\0');
}
/********************************************//**
 * \brief pars integer data and insert to data linked list
 *
 * \return None
 ***********************************************/
void insert_int2data_list(int *DC, line_node *curr_line_node)
{
    char *delims = ", \t";
    char *p_token = curr_line_node->tokenz[curr_line_node->tok_idx];
    char *token;

    while (curr_line_node->tok_idx < curr_line_node->num_tokenz)
    {
        token = strtok(p_token, delims);

        while(token != NULL)
        {
            if (isdigit(*token) || *token == '-' || *token == '+')
            {
                create_data_node(DC, atoi(token));
            }
            else if (isalpha(*token))
            {
                curr_line_node->error_flag = TRUE;
                print_error(curr_line_node->line_num, "integer data type allows only 16,777,214 to -16,777,214, can be with '+' symbol.");
            }
            token = strtok(NULL, delims);
        }
        p_token = curr_line_node->tokenz[++(curr_line_node->tok_idx)];
    }
}
/********************************************//**
 * \brief create instuction list
 *
 * \return newly created instruction list
 ***********************************************/
img_node *create_inst_node(int *IC)
{
    img_node *tail = g_inst_head;
    img_node *new_node = (img_node *)malloc(sizeof(img_node));

    if (new_node == NULL)
    {
        printf("[ERROR] Failed to allocate memory for line_list_head.\n");
        exit(0);
    }

    if(tail == NULL)
    {
        g_inst_head = new_node;
        tail        = g_inst_head;
    }
    else
    {
        while(tail->next_node != NULL)
        {
            tail = tail->next_node;
        }
        tail->next_node = new_node;
        tail = tail->next_node;
    }
    tail->address = (*IC);
    tail->data    = 0;
    tail->next_node = NULL;
    tail->ARE = 0;
    (*IC)++;

    return tail;
}
/********************************************//**
 * \brief add addressing and register value to instruction words
 *
 * \return none
 ***********************************************/
void add_addressing_and_registers(img_node *curr_inst_node, int op_count, int adr_val, int reg_val)
{
    if (op_count == NO_OPERANDS)
    {
        curr_inst_node->ARE = ARE_A;
    }
    if (op_count == FIRST_OP)
    {
        curr_inst_node->data |= (adr_val << ADD_SRC_BITS);
        return;
    }
    else if (op_count == SECOND_OP)
    {
        curr_inst_node->data |= (adr_val << ADD_DEST_BITS);
        return;
    }
}

/********************************************//**
 * \brief add numbers defined as data to data list
 *
 * \return none
 ***********************************************/
void add_num2data_list(char *token, int *IC, char ARE,int isReg)
{
    img_node *new_img_node = create_inst_node(IC);
    int num;

    if (token == NULL)
    {
        return;
    }

    if (!isdigit(*token))
    {
        token++;
    }
    num = atoi(token);
    cpy_num2data_list(num, new_img_node,isReg);

    if (ARE == 0)
    {
        return;
    }
    else
    {
        new_img_node->ARE = ARE;
    }
}

/********************************************//**
 * \brief creates a label struct and initializes
 *
 * \return None
 ***********************************************/

void insert_label(line_node *curr_line_node, char *type, int DC, int extern_entry)
{
    label_node *curr_label_node = curr_line_node->label;
    char *label_str;
    int tok_idx = curr_line_node->tok_idx;

    curr_label_node->address = DC;
    if (!extern_entry)
        label_str = get_label(curr_line_node);
    else
    {
        label_str = curr_line_node->tokenz[++tok_idx];
    }
    curr_label_node->label = label_str;
    curr_label_node->dirc_type = type;

    curr_line_node->tok_idx++;
}

