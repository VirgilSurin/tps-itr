#include <stdio.h>
#include <stdlib.h>
#include <signal.h>


int total_volume;               /* the total volume available in the stock */
int[5] products;                /* array containing the number of products available.
                                   if products[2] == 6, it means that there are
                                   6 products of type 2.
                                 */
int[5] prod_space;              /* space allowed for a specific product.
                                 if prod_space[3] == 4, it means that there can ba
                                 at most 4 product of type 4 in stock
                                */
int[5] prod_volume;             /* volume taken by a prod.
                                 if prod_volume[1] == 2, it means that product of type 1
                                 has a volume of 2
                                */

int main()
{
    /* encode products values */
    prod_volume = [2, 3, 1, 5, 4];
    prod_space = [10, 10, 10, 10, 10];
    total_volume = 2*10 + 3*10 + 1*10 + 5*10 + 4*10;
    products = [0, 0, 0, 0, 0];
    
    /* handle ready signal (when a manufacturer has built something) */

    /* creates a message queue to handle commands */
    return 0;
}
