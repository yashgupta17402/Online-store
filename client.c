#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#define MAX_PROD 30
 struct Product
{
    int id;//product id
    char name[50];//product name
    int quantity;//product quanitity
     int price;//product price
};
struct cart 
{
    int cid;//customer id
    struct Product products[MAX_PROD];//cart having products
};
//functions defined
void loginadmin(int sd);
void loginuser(int sd);
void printProduct(struct Product p);
//program starting
int main()
{
    printf("Establishing connection to server\n");
    int sd = socket(AF_INET, SOCK_STREAM, 0);

    if (sd == -1){
        perror("Error: ");
        return -1;
    }

    struct sockaddr_in serv;
    
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(5710);

    if (connect(sd, (struct sockaddr *)&serv, sizeof(serv)) == -1){
        perror("Error: ");
        return -1;
    }
    printf("connection established successfully\n");
    while(1)
    {
        int choice;
        printf("+---------------------+\n");
        printf("1.ADMIN\n");
        printf("2.USER\n");
        printf("enter your choice\n");
        scanf("%d",&choice);
        write(sd,&choice,sizeof(int));//sending choice between admin or user
        if(choice==1)
        {
             loginadmin(sd);
             break;
        }
        else if(choice==2)
        {
                loginuser(sd);
                break;
        }
        else
        printf("incorrect option\n");

    }
}
void loginadmin(int sd)//when admin logins this function comes into play
{
    struct Product p;
    printf("------\n");
    while(1)
    {
    printf("1.ADD A PRODUCT\n");
    printf("2.Delete a product\n");
    printf("3.Update price of a product \n");
    printf("4.update quantity of a product\n");
    printf("5.view inventory\n");
    printf("6.exit\n ");
    printf("enter your choice\n");
    int achoice;//admin's choice
    scanf("%d",&achoice);
    write(sd,&achoice,sizeof(int));//sending choice of admin
    if(achoice==1)
    {
    printf("enter id of product\n");
    scanf("%d",&p.id);
     printf("enter name of product\n");
     scanf("%s",p.name);
     printf("enter price of product\n");
     scanf("%d",&p.price);
     printf("enter quantity of the product\n");
     scanf("%d",&p.quantity); 
     write(sd,&p,sizeof(struct Product));//sending details of product
      char response[80];
                int n = read(sd, response, sizeof(response));
                response[n] = '\0';

                printf("%s", response);
    }
    else if(achoice==2)
    {
        int id;
      printf("enter id of product to be deleted\n");
     scanf("%d",&id);
     if(id<=0)
     {
        printf("invalid choice of id\n");
     }
     else
     {
     write(sd,&id,sizeof(int));
     char response[80];
                read(sd, response, sizeof(response));
                printf("%s\n", response);
     }
    }
   
      else  if(achoice==3)//changing price
       {
          int id;
           printf("enter the id of product\n");
           scanf("%d",&id);
           printf("enter the new price\n");
           int nprice;
           scanf("%d",&nprice);
           struct Product p;
                p.id = id;
                p.price = nprice;
                write(sd, &p, sizeof(struct Product));
                char response[80];
                read(sd, response, sizeof(response));
                printf("%s\n", response);
            
          
       }
       else if(achoice==4)//changing quantity
       {
        int id;
           printf("enter the id of product\n");
           scanf("%d",&id);
           if(id<0)
           {
            printf("invalid product id\n");
           }
           else
           {
                printf("enter the new quantity\n");
                int nq;
                scanf("%d",&nq);
                struct Product p;
                p.id = id;
                p.quantity = nq;
                write(sd, &p, sizeof(struct Product));
                char response[80];
                read(sd, response, sizeof(response));
                printf("%s\n", response);
                     
           }
       }
    else if(achoice==5)//viewing inventory
    {
        printf("ProductID\tProductName\tQuantity\tPrice\n");
       while (1)
       {
        struct Product p;
        read(sd, &p, sizeof(struct Product));
        if (p.id != -1)
        {
            if (p.id != -1 && p.quantity > 0){
            printf("%d \t\t%s \t%d \t %d \n", p.id, p.name, p.quantity, p.price);
    }
        }
        else
        {
            break;
        }
      }
        
    }
    else
    {
        break;
    }

    }
}

void loginuser(int sd)//when user logins this function comes into play
{
  
    while(1)
    {
    //     bool res;
    //     int k;
    //    int cid = customerId();
    //    write(sd, &cid, sizeof(int));
    //    k=read(sd,&res,sizeof(bool));
    //    if(k==1)
    //    {
    //     printf("customer already registered");
    //    }
    //    if(k==0)
    //    {
    //     printf("customer need to register");
    //    }
      printf("----\n");
    printf("Menu:\n");
    printf("1. To see all the products available\n");
    printf("2. To see your cart\n");
    printf("3. To add products to your cart\n");
    printf("4. To edit an existing product in your cart\n");
    printf("5. To proceed for payment\n");
    printf("6. To register a new customer\n");
    printf("7. To exit the menu\n");
    printf("Please enter your choice\n");
    int choice;
    scanf("%d",&choice);//taking choice from menu of user
    write(sd,&choice,sizeof(int));//sending user choice
    if(choice==1)//seeing all products available list
    {
          
      printf("ProductID\tProductName\tQuantity\tPrice\n");
      while (1)
      {
        struct Product p;
         read(sd, &p, sizeof(struct Product));
        if (p.id != -1)
        {
            if ( p.quantity > 0)
            {
             printf("%d\t \t%s \t\t%d \t \t %d \n", p.id, p.name, p.quantity, p.price);
            }
        }
        else
        {
            break;
        }
      }
    }
    else if(choice==2)//seeing user's cart
    {
        int cid=-1;
        while (1)
        {
           printf("Enter customer id\n");
           scanf("%d", &cid);

           if (cid < 0)
           {
            printf("Customer id can't be negative, try again\n");
           }
           else
           {
            break;
           }
        }
        write(sd, &cid, sizeof(int));
        struct cart o;
                read(sd, &o, sizeof(struct cart));
                if (o.cid != -1){
                    printf("Customer ID %d\n", o.cid);
                    printf("ProductID\tProductName\tQuantity\tPrice\n");
                    for (int i=0; i<MAX_PROD; i++){
                        printProduct(o.products[i]);
                    }
                }else{
                    printf(" customer id has nothing in cart/invalid id\n");
                }
 
    }
    else if(choice==3)//adding product to the cart
    {
        int cid=-1;
        while (1)
        {
           printf("Enter customer id\n");
           scanf("%d", &cid);

           if (cid < 0)
           {
            printf("Customer id can't be negative, try again\n");
           }
           else
           {
            break;
           }
        }
        write(sd, &cid, sizeof(int));
        int res;
                read(sd, &res, sizeof(int));
                if (res == -1){
                    printf("Invalid customer id\n");
                    continue;
                }
              
                int prodId = -1;
          while (1)//taking product id
                {
                 printf("Enter product id\n");
                         scanf("%d", &prodId);

                    if (prodId < 0)
                    {
                       printf("Product id can't be negative, try again\n");
                    }
                    else
                     {
                       break;
                     }
                }
                int quantity;
                printf("enter quantity to want\n");//taking quantity user wants to add to cart
                scanf("%d",&quantity);
                struct Product p;
                p.id = prodId;
                p.quantity = quantity;
                 write(sd, &p, sizeof(struct Product));
                 char response[80];
                read(sd, response, sizeof(response));
                printf("%s", response);
            }
    else if(choice==4)//editing exisiting product in user's cart
    {

           int cid=-1;
        while (1)
        {
           printf("Enter customer id\n");
           scanf("%d", &cid);

           if (cid < 0)
           {
            printf("Customer id can't be negative, try again\n");
           }
           else
           {
            break;
           }
        }
        write(sd, &cid, sizeof(int));
        int res;
                read(sd, &res, sizeof(int));
                if (res == -1)
                {
                    printf("Invalid customer id\n");
                    continue;
                }
                
                int prodId = -1;
          while (1)//taking product id
                {
                 printf("Enter product id\n");
                         scanf("%d", &prodId);

                    if (prodId < 0)
                    {
                       printf("Product id can't be negative, try again\n");
                    }
                    else
                     {
                       break;
                     }
                }
                int quantity;
                printf("enter quantity to want\n");//taking quantity user wants to add to cart
                scanf("%d",&quantity);
                struct Product p;
                p.id = prodId;
                p.quantity = quantity;
                 write(sd, &p, sizeof(struct Product));
                 char response[80];
                read(sd, response, sizeof(response));
                printf("%s", response);
    }
    else if(choice==5)//payment of user's cart
    {
          int cid=-1;
        while (1)
        {
           printf("Enter customer id\n");
           scanf("%d", &cid);

           if (cid < 0)
           {
            printf("Customer id can't be negative, try again\n");
           }
           else
           {
            break;
           }
        }
        write(sd, &cid, sizeof(int));
        int res;
        read(sd, &res, sizeof(int));
                if (res == -1)
                {
                    printf("Invalid customer id\n");
                    continue;
                }
                struct cart c;
                read(sd, &c, sizeof(struct cart));
                int ordered, instock;
                int price;
                for (int i=0; i<MAX_PROD; i++)
                {

                    if (c.products[i].id != -1)
                    {
                        read(sd, &ordered, sizeof(int));
                        read(sd, &instock, sizeof(int));
                        read(sd, &price, sizeof(int));
                        printf("Product id- %d\n", c.products[i].id);
                        printf("Ordered - %d; Incart - %d; Price - %d\n", ordered, instock, price);
                        c.products[i].quantity = instock;
                        c.products[i].price = price;
                    }
                }
                int total = 0;
                for (int i=0; i<MAX_PROD; i++)
                {
                        if (c.products[i].id != -1)
                        {
                           total += c.products[i].quantity * c.products[i].price;
                        }
                }
                printf("Total in your cart\n");
                printf("%d\n", total);
                int payment;

                while (1){
                    printf("Please enter the amount to pay\n");
                    scanf("%d", &payment);

                    if (payment != total){
                        printf("Wrong total entered, enter again\n");
                    }else{
                        break;
                    }
                }

                char ch = 'y';
                printf("Payment recorded, order placed\n");
                write(sd, &ch, sizeof(char));
                read(sd, &ch, sizeof(char));
                write(sd, &total, sizeof(int));
                write(sd, &c, sizeof(struct cart));


    }
    else if(choice==6)
    {
        char conf;
                printf("Press y/n if you want to continue\n");
                scanf("%c", &conf);
                scanf("%c", &conf);

                write(sd, &conf, sizeof(char));
                if (conf == 'y')
                {
                    int cid;
                    read(sd, &cid, sizeof(int));
                    printf("Your new customer id : %d\n",cid);
                    
                }
                else
                {
                    printf("Request aborted\n");
                }

    }
    else if(choice==7)
    {
         break;
    }
    else
    {
    printf("invalid choice\n");
    continue;
    }

    }
}
void printProduct(struct Product p){
    if (p.id != -1 && p.quantity > 0){
        printf("%d \t\t%s \t\t%d \t\t %d  \n", p.id, p.name, p.quantity, p.price);
    }
}
