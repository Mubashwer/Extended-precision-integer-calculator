/* Algorithms are fun!
 * Name: myass1.c
 * Author: Mubashwer Salman Khurshid (mskh, 601738)
 * Date: 5/09/2013
 * Description: Extended precision integer calculator program (Project 1)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#define INTSIZE	 100	      /* max number of digits per integer value */
#define LINELEN	 102	      /* maximum length of any input line */
#define NVARS	 26	          /* number of different variables */

#define CH_BLANK ' '          /* character blank */
#define CH_ZERO  '0'          /* character zero */
#define CH_A     'a'          /* character 'a', first variable name */

#define ERROR	 (-1)	      /* error return value from some functions */

#define PRINT	 '?'          /* the print operator */
#define ASSIGN	 '='	      /* the assignment operator */
#define PLUS	 '+'	      /* the addition operator */
#define MULTIPLY '*'          /* the multiplication operator */
#define POWER_OF '^'          /* "power of" operator */
#define ALLOPS   "?=+*^"      /* list of all valid operators */
#define SGNCHRS  "+-"         /* the two sign characters */
#define NUMCHRS  "0123456789" /* list of characters legal within numbers */

/* one extended-precision "variable" */
typedef struct longint_t
{
    int digits[INTSIZE];
    int size; /* sign is 0 for negative values and 1 for positive values */
    int sign;

} longint_t;

/******************************************************************************/

/* function prototypes */

longint_t *makeint(void);
void print_prompt(void);
int read_line(char *line, int maxlen);
void process_line(longint_t vars[], char *line);
int to_varnum(char ident);
int get_second_value(longint_t vars[], char *rhsarg, longint_t *second_value);
int to_int(char digit);
char to_digit(int number); /* unused but it was present in skeleton */
void do_print(longint_t *var);
void do_assign(longint_t *var1, longint_t *var2);
void do_plus(longint_t *var1, longint_t *var2);
longint_t *addition(longint_t *var1, longint_t *var2);
longint_t *subtraction(longint_t *var1, longint_t *var2);
void right_shift(longint_t *var, int carry);
void do_multiply(longint_t *var1, longint_t *var2);
longint_t *times_digit(longint_t *var, int digit);
void times_ten(longint_t *var, int exp);
int evaluate(longint_t *var);
void do_power(longint_t *var1, longint_t *var2);
void zero_vars(longint_t vars[]);

/******************************************************************************/

/* main program controls all the action
 */
int main(int argc, char **argv)
{
    char line[LINELEN+1];
    longint_t vars[NVARS];

    zero_vars(vars);
    print_prompt();
    while (read_line(line, LINELEN))
    {
        if (strlen(line)>0)
        	process_line(vars, line); /* non empty line, so process it */

        print_prompt();
    }

    /* all done, so pack up and go home */
    printf("\n");
    return 0;
}

/******************************************************************************/

/* allocates memory for the long integer type variable
 * default value is +1
 */
longint_t *makeint()
{
    longint_t *var = malloc(sizeof(longint_t));
    var->sign = 1;
    var->size = 1;
    var->digits[0] = 1;
    return var;
}

/******************************************************************************/

/* prints the prompt indicating ready for input
 */
void print_prompt(void)
{
    printf("> ");
}

/******************************************************************************/

/* read a line of input into the array passed as argument
 * returns false if there is no input available
 * all whitespace characters and commas are removed
 */
int read_line(char *line, int maxlen)
{
    int i=0, c;

    while (((c=getchar())!=EOF) && (c!='\n'))
        if (i<maxlen && !isspace(c) && c != ',')
        	line[i++] = c;

    line[i] = '\0';
    return ((i>0) || (c!=EOF));
}

/******************************************************************************/

/* process a command by parsing the input line into parts
 */
void process_line(longint_t vars[], char *line)
{
    int varnum, optype, status;
    longint_t *second_value = makeint();

    /* determine the LHS variable, it
     * must be first character in line
     */
    varnum = to_varnum(line[0]);
    if (varnum==ERROR)
    {
        printf("Invalid LHS variable\n");
        return;
    }

    /* more testing for validity
     */
    if (strlen(line)<2)
    {
        printf("No operator supplied\n");
        return;
    }

    /* determine the operation to be performed, it
     * must be second character in line
     */
    optype = line[1];
    if (strchr(ALLOPS, optype) == NULL)
    {
        printf("Unknown operator\n");
        return;
    }

    /* determine the RHS argument (if one is required),
     * it must start in third character of line
     */
    if (optype != PRINT)
    {
        if (strlen(line)<3)
        {
            printf("No RHS supplied\n");
            return;
        }
        status = get_second_value(vars, line+2, second_value);
        if (status==ERROR)
        {
            printf("RHS argument is invalid\n");
            return;
        }
    }

    /* finally, do the actual operation
     */
    if (optype == PRINT)
    	do_print(vars+varnum);

    else if (optype == ASSIGN)
    	do_assign(vars+varnum, second_value);

    else if (optype == PLUS)
    	do_plus(vars+varnum, second_value);

    else if (optype == MULTIPLY)
        do_multiply(vars+varnum, second_value);

    else if (optype == POWER_OF)
        do_power(vars+varnum, second_value);

    return;
}

/******************************************************************************/

/* convert a character variable identifier to a variable number
 */
int to_varnum(char ident)
{
    int varnum;
    varnum = ident - CH_A;

    if (0<=varnum && varnum<NVARS)
        return varnum;
    else
        return ERROR;
}

/******************************************************************************/

/* process the input line to extract the RHS argument, which
 * should start at the pointer that is passed
 */
int get_second_value(longint_t vars[], char *rhsarg, longint_t *second_value)
{
    char *p;
    int varnum2, i = 0, j;
    if (strchr(NUMCHRS, *rhsarg) != NULL || strchr(SGNCHRS, *rhsarg) != NULL)
    {
        /* first character is a digit or a sign, so RHS
         * should be a number
         */
        p = rhsarg+1;
        while (*p)
        {
            if (strchr(NUMCHRS, *p) == NULL)
                return ERROR; /* nope, found an illegal character */
            p++;
        }

        /* leading zero error */
        if(rhsarg[0] == CH_ZERO && rhsarg[1] != '\0')
            return ERROR;

        if(strchr(SGNCHRS, *rhsarg) != NULL)
        {   i++;
            /* sign is updated for negative number */
            if(rhsarg[0] == '-')
                second_value->sign = 0;
        }

        /* number is converted and copied to variable digit by digit */
        for(j = 0; rhsarg[i] != '\0'; i++, j++)
        	second_value->digits[j] = to_int(rhsarg[i]);

        second_value->size = j;
        return !ERROR;
    }
    else
    {
        /* argument is not a number, so might be a variable */
        varnum2 = to_varnum(*rhsarg);
        if (varnum2==ERROR || strlen(rhsarg)!=1)
        	return ERROR; /* nope, not a variable either */

        /* is a variable, so can use its value to assign to
         * second_value
         */
        do_assign(second_value, vars+varnum2);
        return !ERROR;
    }
    return ERROR;
}

/******************************************************************************/

/* convert a character digit to the int equivalent, but null bytes
 * stay as zero integers
 */
int to_int(char digit)
{
    if (digit != '\0')
        return digit - CH_ZERO;
    else
        return 0;
}

/******************************************************************************/

/* and back again to a digit
 */
char to_digit(int number)
{
    return number + CH_ZERO;
}

/******************************************************************************/

/* print out a longint value digit by digit
 */
void do_print(longint_t *var)
{
    int i, shift;
    /* shift determines the position of first comma */
    shift = (var->size) % 3;
    if(!shift)
        shift = 3;

    if(!(var->sign))
        printf("-");

    for(i = 0; i < var->size; i++)
    {
        if(i == shift && i != var->size -1)
        {
            /* after first comma, there are commas between 3 digits */
            shift += 3;
            printf(",");
        }
        printf("%d", var->digits[i]);
    }

    printf("\n");
    return;
}

/******************************************************************************/

/* update the indicated variable var1 by doing an assignment
 * using var2
 */
void do_assign(longint_t *var1, longint_t *var2)
{
    /* copies integer of var2 into var1 array digit by digit */
    int i;
    for(i = 0; i < var2->size; i++)
        var1->digits[i] = var2->digits[i];

    var1->sign = var2->sign;
    var1->size = var2->size;
    return;
}


/******************************************************************************/

/* update the indicated variable var1 by doing an addition
 * using var2 to compute var1 = var1 + var2
 */
void do_plus(longint_t *var1, longint_t *var2)
{
    longint_t *result;

    /* If the two variables have different signs then subtraction occurs */
    if(var1->sign == var2->sign)
        result = addition(var1, var2);
    else
        result = subtraction(var1, var2);

    do_assign(var1, result);

    free(result); /* freeing dynamically allocated memory */
    return;
}

/******************************************************************************/

/* when two integers have same sign, the addition of var1 and var2
 * is computed and pointer to the result is returned
 */
longint_t *addition(longint_t *var1, longint_t *var2)
{
    int carry = 0, i, j;
    longint_t *small = var1;
    longint_t *big = var2; /* big is the larger number */
    longint_t *result = makeint(); /* defualt value is 1 */

    if(var1->size > var2->size)
    {
        small = var2;
        big = var1;
    }
    result->size = big->size;

    for(i = big->size - 1, j = small->size - 1; i >= 0; i--, j--)
    {
        /*when all the digits of small digits are added along with carry
         * big digits are just copied along with the residual carry
         */
        if(j >= 0)
            result->digits[i] = big->digits[i] + small->digits[j] + carry;
        else
            result->digits[i] = big->digits[i] + carry;

        /* if addition of digits lead to a two-digit number
         * 1 is carried on and 1- is subtracted from result */
        if(result->digits[i] > 9)
        {
            result->digits[i] -= 10;
            carry = 1;
        }
        else
            carry = 0;
    }

    /* shifts the digits of integer by 1 digit to the right and adds the final
     * carry to the leftmost position
     */
    if(carry)
        right_shift(result, carry);

    return result;
}

/******************************************************************************/

/* when two integers have different sign, the integer with smaller magnitude
 * is subtracted from that with bigger magnitude and pointer to the result
 * is returned
 */
longint_t *subtraction(longint_t *var1, longint_t *var2)
{
    int borrow = 0, i, j, k, shift = 0;
    longint_t *small = var1;
    longint_t *big = var2; /* integer with larger magnitude */
    longint_t *result = makeint();
    result->digits[0] = 0; /* defualt value is 0 */

    if(var1->size > var2->size)
    {
        small = var2;
        big = var1;
    }

    if(var1->size == var2->size)
    {
        /* If two variables have same number of digits, number is compared
         * digit by digit to find out the integer with larger magnitude
         */
        for(i = 0; i < var1->size && (var1->digits[i] == var2->digits[i]); i++);

        /* If the integers have identical digits, then result is 0 */
        if(i == var1->size)
            return result;

        if(var1->digits[i] > var2->digits[i])
        {
            small = var2;
            big = var1;
        }
    }
    /* result integer has the same sign as the integer with bigger magnitude */
    result->sign = big->sign;
    result->size = big->size;

    for(i = big->size - 1, j = small->size - 1; i >= 0; i--, j--)
    {
        borrow = 0;
        if(big->digits[i] < small->digits[j] && j >= 0)
        {
            /*If digit of the big integer is smaller than 10 is borrowed */
            borrow = 10;

            /*If the digit to be borrowed is 0, it becomes 9 and the
             *next digit is checked until a non-zero digit is found
             */
            for(k = i - 1; k >= 0 && big->digits[k] == 0; k--)
                big->digits[k] = 9;

            if(k >= 0)
                big->digits[k] -= 1;
        }
        /* small's digit is subtracted from big's digit and borrow is added
         * when all the digits of small are subtracted, the remaining digits
         * of big are just copied to the result
         */
        if(j >= 0)
            result->digits[i] = (big->digits[i] + borrow) - small->digits[j];
        else
            result->digits[i] = big->digits[i];
    }

    /* due to subtraction, integer may become smaller and thus have leading
     * zeroes, the integer is shifted to the left according to the number
     * of the leading zeroes
     */
    for(i = 0; !(result->digits[i]) && i < big->size; i++)
        shift += 1;
    result->size -= shift;

    for(i = 0; i < result->size; i++)
            result->digits[i] = result->digits[i+shift];

    return result;
}
/******************************************************************************/
/* shifts the digits of integer by 1 digit to the right and adds the final
 * carry to the leftmost position
 */
void right_shift(longint_t *var, int carry)
{
    int i;
    if(var->size == INTSIZE)
    {
        printf("Number of digits of result exceeds %d\n", INTSIZE);
        exit(EXIT_FAILURE);
    }
    else
    {
        var->size += 1;
        for(i = var->size; i > 0; i--)
            var->digits[i] = var->digits[i-1];
        var->digits[0] = carry;
    }
    return;
}
/******************************************************************************/

/* update the indicated variable var1 by doing multiplication
 * using var2 to compute var1 = var1 * var2
 */
void do_multiply(longint_t *var1, longint_t *var2)
{
    int i, exp = 1;
    longint_t *small = var1;
    longint_t *big = var2; /* big is the larger number */
    longint_t *intermed = makeint();
    longint_t *result;

    if(var1->size > var2->size)
    {
        small = var2;
        big = var1;
    }

    /* first, result is the larger number multiplied by the units digit
     * of the smaller one */
    result = times_digit(big, small->digits[small->size-1]);

    if(var1->sign == var2->sign)
        result->sign = 1;
    else result->sign = 0;

    if(small->size > 1)
        for(i = small->size - 2; i >= 0; i--)
        {
            /*intermed is the larger number multiplied by the preceding
             * digits and then multiplied by 10 with increasing power
             */
            intermed = times_digit(big, small->digits[i]);
            times_ten(intermed, exp++);
            /* result is updated by adding intermed to it */
            do_plus(result,intermed);
            free(intermed);
        }

    do_assign(var1, result);
    free(result);
    return;
}

/******************************************************************************/

longint_t *times_digit(longint_t *var, int digit)
{
    longint_t *result = makeint();
    int carry = 0, i;
    result->size = var->size;

    /* Anything multiplied by 0 is 0 */
    if(digit == 0)
    {
        result->digits[0] = 0;
        result->size = 1;
        return result;
    }

    for(i = var->size - 1; i >= 0; i--)
    {
        /* each digit is multiplied and carry is added */
        result->digits[i] = (var->digits[i] * digit) + carry;
        /*if result exceeds 9, carry is assigned the tens digit and result
          becomes the units digit */
        if(result->digits[i] > 9)
        {
            carry = result->digits[i] / 10;
            result->digits[i] = result->digits[i] % 10;
        }
        else
            carry = 0;
    }
    /* shifts the digits of integer by 1 digit to the right and adds the final
     * carry to the leftmost position
     */
    if(carry)
        right_shift(result, carry);

    return result;
}
/******************************************************************************/

/* modifies var1 by multiplying it by 10^exp
 */
void times_ten(longint_t *var, int exp)
{
    int i, new_size = var->size + exp;
    /* if exponent is 1 or base is 0, number is unchanged */
    if(var->digits[0] == 0 || exp == 0)
        return;

    /* if product size exceeds INTSIZE digits, program is closed */
    if((var->size + exp) > INTSIZE)
    {
        printf("Number of digits of result exceeds %d\n", INTSIZE);
        exit(EXIT_FAILURE);
    }

    /* trailing zeroes are added */
    for(i = var->size; i < new_size; i++)
        var->digits[i] = 0;

    var->size = new_size;
    return;
}

/******************************************************************************/

/* converts longint_t to normal int type
 */
int evaluate(longint_t *var)
{
    int sum = 0, multiplier = 1, i;
    for(i = var->size - 1; i >= 0; i--)
    {
        sum += var->digits[i] * multiplier;
        multiplier *= 10;
    }
    return sum;
}

/******************************************************************************/

/* update the indicated variable var1 by raising it to the power of
 * var2 to compute var1 = var1 ^ var2
 */
void do_power(longint_t *var1, longint_t *var2)
{
    longint_t result;
    int exp = evaluate(var2), i; /* exp is the int version of variable 2 */

    /* the following if-else statements contain the basic rules of
     * raising an integer to the power of another
     */
    if(var1->digits[0] == 1 && var1->size == 1)
    {
        if(var1->sign == 0)
            var1->sign = 1 - (var2->digits[var2->size - 1] % 2);
        return;
    }
    else if(var2->sign == 0)
    {
        /* this may lead to floating point numbers */
        printf("Negative exponent is not supported when base is not 1\n");
        exit(EXIT_FAILURE);
    }
    else if(var1->digits[0] == 0)
    {
        if(var2->digits[0] == 0)
        {
            printf("Math ERROR\n");
            exit(EXIT_FAILURE);
        }
        else
            return;
    }
    else if(var2->digits[0] == 0)
    {
        var1->digits[0] = 1;
        var1->size = 1;
    }

    /* var1 is cumulatively multiplied by itself var2 - 1 times */
    do_assign(&result, var1);
    for(i = 1; i < exp; i++)
        do_multiply(&result, var1);

    do_assign(var1, &result);
    return;
}


/******************************************************************************/

/* set the vars array to all zero values
 */
void zero_vars(longint_t vars[])
{
    int i;
    for (i=0; i<NVARS; i++)
    {
        vars[i].digits[0] = 0;
        vars[i].size = 1;
        vars[i].sign = 1;
    }
    return;
}
