#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#define MAX_PROD 30//maximum 30 products can be taken

struct Product 
{
    int id;
    char name[50];
    int quantity; 
    int price;
};
struct cart 
{
    int custid;
    struct Product products[MAX_PROD];
};
struct index 
{
    int custid; 
    int offset;
};
void setLockCust(int fd_custs, struct flock lock_cust);//setting locks on customers list
void unlock(int fd, struct flock lock);
void productReadLock(int fd, struct flock lock);//puts read lock on products
void productWriteLock(int fd, struct flock lock);//puts write lock  on products
void cartOffsetLock(int fd_cart, struct flock lock_cart, int offset, int ch);
int getOffset(int cust_id, int fd_custs);//locks the customer lists set offset to that particualr cust
void listProducts(int fd, int new_fd);//lists the products
void addCustomer(int fd_cart, int fd_custs, int new_fd);//adding customner to customer list
void viewCart(int fd_cart, int new_fd, int fd_custs);//fcn to view cart for user
void addProductToCart(int fd, int fd_cart, int fd_custs, int new_fd);//adding product to the cart  of user
void editProductInCart(int fd, int fd_cart, int fd_custs, int new_fd);//editing cart of user
void payment(int fd, int fd_cart, int fd_custs, int new_fd);//function perfroming payment for user
void generateAdminReceipt(int fd_admin, int fd);//function to generate admin receipt
void addProducts(int fd, int new_fd, int fd_admin);//function to add product
void deleteProduct(int fd, int new_fd, int id, int fd_admin);//function to delete product
void updateProduct(int fd, int new_fd, int ch, int fd_admin);//function to  change price/quantity of product


int main()
{
    printf("Setting up server\n");

    //file containing all the records is called records.txt
    int fd_custs = open("CUSTOMERS.txt", O_RDWR | O_CREAT, 0777);//fd of file mainting customers info
    int fd = open("REC.txt", O_RDWR | O_CREAT, 0777);//record of whole buy/sell/update
    int fd_cart = open("ORDERS.txt", O_RDWR | O_CREAT, 0777);//fd of file maintaining orders/cart
    int fd_admin = open("AReceipt.txt", O_RDWR | O_CREAT, 0777);//admin receipt
    lseek(fd_admin, 0, SEEK_END);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1){
        perror("Error: ");
        return -1;
    }

    struct sockaddr_in serv, cli;
    
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(5710);

    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("Error: ");
        return -1;
    }

    if (bind(sockfd, (struct sockaddr *)&serv, sizeof(serv)) == -1)
    {
        perror("Error: ");
        return -1;
    }

    if (listen(sockfd, 5) == -1){
        perror("Error: ");
        return -1;
    }

    int size = sizeof(cli);
    printf("Server set up successfully\n");
	while (1)
	{

        int new_fd = accept(sockfd, (struct sockaddr *)&cli, &size);
        if (new_fd == -1){
            // perror("Error: ");
            return -1;
        }

        if (!fork())
		{
            printf("Connection with client successful\n");
            close(sockfd);

            int choice;
            read(new_fd, &choice, sizeof(int));
			if (choice == 1)//admin login
            {
                int achoice;
                while (1)
				{
                    read(new_fd, &achoice, sizeof(int));//reading the choice of admin from admin menu

                    lseek(fd, 0, SEEK_SET);
                    lseek(fd_cart, 0, SEEK_SET);
                    lseek(fd_custs, 0, SEEK_SET);

                    if (achoice == 1){
                        addProducts(fd, new_fd, fd_admin);
                    } 
                    else if (achoice == 2){
                        int id;
                        read(new_fd, &id, sizeof(int));
                        deleteProduct(fd, new_fd, id, fd_admin);
                    }
                    else if (achoice == 3){
                        updateProduct(fd, new_fd, 1, fd_admin);
                    }

                    else if (achoice == 4)
					{
                        updateProduct(fd, new_fd, 2, fd_admin);
                    }

                    else if (achoice == 5){
                        listProducts(fd, new_fd);
                    }

                    else if (achoice == 6){
                        close(new_fd);
                        generateAdminReceipt(fd_admin, fd);
                        break;
                    }
                    else{
                        continue;
                    }
                }
            }
            
            else if (choice == 2)//user login
            {

                int uchoice;
                while (1)
				{
                    read(new_fd, &uchoice, sizeof(int));

                    lseek(fd, 0, SEEK_SET);
                    lseek(fd_cart, 0, SEEK_SET);
                    lseek(fd_custs, 0, SEEK_SET);

                    if (uchoice == 1)//listing of products available
					{
                        listProducts(fd, new_fd);
                    }

                    else if (uchoice == 2)//seeing the user cart
					{
                        viewCart(fd_cart, new_fd, fd_custs);
                    }

                    else if (uchoice == 3)//adding products to cart
					{
                        addProductToCart(fd, fd_cart, fd_custs, new_fd);
                    }

                    else if (uchoice == 4)//editng the cart
					{
                        editProductInCart(fd, fd_cart, fd_custs, new_fd);
                    }

                    else if (uchoice == 5)//proceedig for payment
					{
                        payment(fd, fd_cart, fd_custs, new_fd);
                    }

                    else if (uchoice == 6)//registering new customer
					{
                        addCustomer(fd_cart, fd_custs, new_fd);
                    }
					else if (uchoice == 7)//exiting from user menu
					{
                        close(new_fd);
                        break;
                    }
                }
                printf("Connection terminated\n");

            }
            
            printf("Connection terminated\n");

        }
		else
		{
            close(new_fd);
        }
    }
}
void setLockCust(int fd_custs, struct flock lock_cust){

    lock_cust.l_len = 0;
    lock_cust.l_type = F_RDLCK;
    lock_cust.l_start = 0;
    lock_cust.l_whence = SEEK_SET;
    fcntl(fd_custs, F_SETLK, &lock_cust);

    return ;
}

void unlock(int fd, struct flock lock){
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
}

void productReadLock(int fd, struct flock lock){
    lock.l_len = 0;
    lock.l_type = F_RDLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    fcntl(fd, F_SETLK, &lock);
}

void productWriteLock(int fd, struct flock lock){
    lseek(fd, (-1)*sizeof(struct Product), SEEK_CUR);
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_CUR;
    lock.l_start = 0;
    lock.l_len = sizeof(struct Product);

    fcntl(fd, F_SETLK, &lock);
}

void cartOffsetLock(int fd_cart, struct flock lock_cart, int offset, int ch){
    lock_cart.l_whence = SEEK_SET;
    lock_cart.l_len = sizeof(struct cart);
    lock_cart.l_start = offset;
    if (ch == 1){
        //rdlck
        lock_cart.l_type = F_RDLCK;
    }else{
        //wrlck
        lock_cart.l_type = F_WRLCK;
    }
    fcntl(fd_cart, F_SETLK, &lock_cart);
    lseek(fd_cart, offset, SEEK_SET);
}

int getOffset(int cust_id, int fd_custs)
{
    if (cust_id < 0){
        return -1;
    }
    struct flock lock_cust;
    setLockCust(fd_custs, lock_cust);
    struct index id;

    while (read(fd_custs, &id, sizeof(struct index))){
        if (id.custid == cust_id){
            unlock(fd_custs, lock_cust);
            return id.offset;
        }
    }
    unlock(fd_custs, lock_cust);
    return -1;
}

void addProducts(int fd, int new_fd, int fd_admin){

    char name[50];
    char response[100];
    int id, qty;
    float price;

    struct Product p1;
    int n = read(new_fd, &p1, sizeof(struct Product));

    strcpy(name, p1.name);
    id = p1.id;
    qty = p1.quantity;
    price = p1.price;

    struct flock lock;
    productReadLock(fd, lock);

    struct Product p;

    int flg = 0;
    while (read(fd, &p, sizeof(struct Product))){

        if (p.id == id && p.quantity > 0){
            write(new_fd, "Duplicate product\n", sizeof("Duplicate product\n"));
            sprintf(response, "Adding product with product id %d failed as product id is duplicate\n", id);
            write(fd_admin, response, strlen(response));
            unlock(fd, lock);
            flg = 1;
            return;
        }
    }

    if (!flg){

        lseek(fd, 0, SEEK_END);
        p.id = id;
        strcpy(p.name, name);
        p.price = price;
        p.quantity = qty;

        write(fd, &p, sizeof(struct Product));
        write(new_fd, "Added successfully\n", sizeof("Added succesfully\n"));
        sprintf(response, "New product with product id %d added successfully\n", id);
        write(fd_admin, response, strlen(response));
        unlock(fd, lock);   
    }
}

void listProducts(int fd, int new_fd){

    struct flock lock;
    productReadLock(fd, lock);

    struct Product p;
    while (read(fd, &p, sizeof(struct Product))){
        if (p.id != -1){
            write(new_fd, &p, sizeof(struct Product));
        }
    }
    
    p.id = -1;
    write(new_fd, &p, sizeof(struct Product));
    unlock(fd, lock);
}

void deleteProduct(int fd, int new_fd, int id, int fd_admin){

    struct flock lock;
    productReadLock(fd, lock);
    char response[100];

    struct Product p;
    int flg = 0;
    while (read(fd, &p, sizeof(struct Product))){
        if (p.id == id){
            
            unlock(fd, lock);
            productWriteLock(fd, lock);

            p.id = -1;
            strcpy(p.name, "");
            p.price = -1;
            p.quantity = -1;

            write(fd, &p, sizeof(struct Product));
            write(new_fd, "Delete successful", sizeof("Delete successful"));
            sprintf(response, "Product with product id %d deleted succesfully\n", id);
            write(fd_admin, response, strlen(response));

            unlock(fd, lock);
            flg = 1;
            return;
        }
    }

    if (!flg){
        sprintf(response, "Deleting product with product id %d failed as product does not exist\n", id);
        write(fd_admin, response, strlen(response));
        write(new_fd, "Product id invalid", sizeof("Product id invalid"));
        unlock(fd, lock);
    }
}

void updateProduct(int fd, int new_fd, int ch, int fd_admin){
    int id;
    int val = -1;
    struct Product p1;
    read(new_fd, &p1, sizeof(struct Product));
    id = p1.id;

    char response[100];
    
    if (ch == 1){
        val = p1.price;
    }else{
        val = p1.quantity;
    }

    struct flock lock;
    productReadLock(fd, lock);

    int flg = 0;
    
    struct Product p;
    while (read(fd, &p, sizeof(struct Product))){
        if (p.id == id){

            unlock(fd, lock);
            productWriteLock(fd, lock);
            int old;
            if (ch == 1){
                old = p.price;
                p.price = val;
            }else{
                old = p.quantity;
                p.quantity = val;
            }

            write(fd, &p, sizeof(struct Product));
            if (ch == 1){
                write(new_fd, "Price modified successfully", sizeof("Price modified successfully"));
                sprintf(response, "Price of product with product id %d modified from %d to %d \n", id, old, val);
                write(fd_admin, response, strlen(response));
            }else{
                sprintf(response, "Quantity of product with product id %d modified from %d to %d \n", id, old, val);
                write(fd_admin, response, strlen(response));
                write(new_fd, "Quantity modified successfully", sizeof("Quantity modified successfully"));               
            }

            unlock(fd, lock);
            flg = 1;
            break;
        }
    }

    if (!flg){
        write(new_fd, "Product id invalid", sizeof("Product id invalid"));
        unlock(fd, lock);
    }
}

void addCustomer(int fd_cart, int fd_custs, int new_fd)
{
    char buf;
    read(new_fd, &buf, sizeof(char));
    if (buf == 'y'){

        struct flock lock;
        setLockCust(fd_custs, lock);
        
        int max_id = -1; 
        struct index id ;
        while (read(fd_custs, &id, sizeof(struct index))){
            if (id.custid > max_id){
                max_id = id.custid;
            }
        }

        max_id ++;
        
        id.custid = max_id;
        id.offset = lseek(fd_cart, 0, SEEK_END);
        lseek(fd_custs, 0, SEEK_END);
        write(fd_custs, &id, sizeof(struct index));

        struct cart c;
        c.custid = max_id;
        for (int i=0; i<MAX_PROD; i++){
            c.products[i].id = -1;
            strcpy(c.products[i].name , "");
            c.products[i].quantity = -1;
            c.products[i].price = -1;
        }

        write(fd_cart, &c, sizeof(struct cart));
        unlock(fd_custs, lock);
        write(new_fd, &max_id, sizeof(int));
    }
}

void viewCart(int fd_cart, int new_fd, int fd_custs)
{
    int cust_id = -1;
    read(new_fd, &cust_id, sizeof(int));

    int offset = getOffset(cust_id, fd_custs);    
    struct cart c;

    if (offset == -1){

        struct cart c;
        c.custid = -1;
        write(new_fd, &c, sizeof(struct cart));
        
    }else{
        struct cart c;
        struct flock lock_cart;
        
        cartOffsetLock(fd_cart, lock_cart, offset, 1);
        read(fd_cart, &c, sizeof(struct cart));
        write(new_fd, &c, sizeof(struct cart));
        unlock(fd_cart, lock_cart);
    }
}

void addProductToCart(int fd, int fd_cart, int fd_custs, int new_fd){
    int cust_id = -1;
    read(new_fd, &cust_id, sizeof(int));
    int offset = getOffset(cust_id, fd_custs);

    write(new_fd, &offset, sizeof(int));

    if (offset == -1){
        return;
    }

    struct flock lock_cart;
    
    int i = -1;
    cartOffsetLock(fd_cart, lock_cart, offset, 1);
    struct cart c;
    read(fd_cart, &c, sizeof(struct cart));

    struct flock lock_prod;
    productReadLock(fd, lock_prod);
    
    struct Product p;
    read(new_fd, &p, sizeof(struct Product));

    int prod_id = p.id;
    int qty = p.quantity;

    struct Product p1;
    int found = 0;
    while (read(fd, &p1, sizeof(struct Product))){
        if (p1.id == p.id) {
            if (p1.quantity >= p.quantity){
                // p1.qty -= p.qty;
                found = 1;
                break;
            }
        }
    }
    unlock(fd_cart, lock_cart);
    unlock(fd, lock_prod);

    if (!found){
        write(new_fd, "Product id invalid or out of stock\n", sizeof("Product id invalid or out of stock\n"));
        return;
    }

    int flg = 0;
    
    int flg1 = 0;
    for (int i=0; i<MAX_PROD; i++){
        if (c.products[i].id == p.id){
            flg1 = 1;
            break;
        }
    }

    if (flg1){
        write(new_fd, "Product already exists in cart\n", sizeof("Product already exists in cart\n"));
        return;
    }

    for (int i=0; i<MAX_PROD; i++){
        if (c.products[i].id == -1){
            flg = 1;
            c.products[i].id = p.id;
            c.products[i].quantity = p.quantity;
            strcpy(c.products[i].name, p1.name);
            c.products[i].price = p1.price;
            break;

        }else if (c.products[i].quantity <= 0){
            flg = 1;
            c.products[i].id = p.id;
            c.products[i].quantity = p.quantity;
            strcpy(c.products[i].name, p1.name);
            c.products[i].price = p1.price;
            break;

        }
    }

    if (!flg){
        write(new_fd, "Cart limit reached\n", sizeof("Cart limit reached\n"));
        return;
    }

    write(new_fd, "Item added to cart\n", sizeof("Item added to cart\n"));

    cartOffsetLock(fd_cart, lock_cart, offset, 2);    
    write(fd_cart, &c, sizeof(struct cart));
    unlock(fd_cart, lock_cart);

}

void editProductInCart(int fd, int fd_cart, int fd_custs, int new_fd)
{

    int cust_id = -1;
    read(new_fd, &cust_id, sizeof(int));

    int offset = getOffset(cust_id, fd_custs);

    write(new_fd, &offset, sizeof(int));
    if (offset == -1){
        return;
    }

    struct flock lock_cart;
    cartOffsetLock(fd_cart, lock_cart, offset, 1);
    struct cart c;
    read(fd_cart, &c, sizeof(struct cart));

    int pid, qty;
    struct Product p;
    read(new_fd, &p, sizeof(struct Product));

    pid = p.id;
    qty = p.quantity;

    int flg = 0;
    int i;
    for (i=0; i<MAX_PROD; i++){
        if (c.products[i].id == pid){
            
            struct flock lock_prod;
            productReadLock(fd, lock_prod);

            struct Product p1;
            while (read(fd, &p1, sizeof(struct Product))){
                if (p1.id == pid && p1.quantity > 0) {
                    if (p1.quantity >= qty){
                        flg = 1;
                        break;
                    }else{
                        flg = 0;
                        break;
                    }
                }
            }

            unlock(fd, lock_prod);
            break;
        }
    }
    unlock(fd_cart, lock_cart);

    if (!flg){
        write(new_fd, "Product id not in the order or out of stock\n", sizeof("Product id not in the order or out of stock\n"));
        return;
    }

    c.products[i].quantity = qty;
    write(new_fd, "Update successful\n", sizeof("Update successful\n"));
    cartOffsetLock(fd_cart, lock_cart, offset, 2);
    write(fd_cart, &c, sizeof(struct cart));
    unlock(fd_cart, lock_cart);
}

void payment(int fd, int fd_cart, int fd_custs, int new_fd){
    int cust_id = -1;
    read(new_fd, &cust_id, sizeof(int));

    int offset;
    offset = getOffset(cust_id, fd_custs);

    write(new_fd, &offset, sizeof(int));
    if (offset == -1){
        return;
    }

    struct flock lock_cart;
    cartOffsetLock(fd_cart, lock_cart, offset, 1);

    struct cart c;
    read(fd_cart, &c, sizeof(struct cart));
    unlock(fd_cart, lock_cart);
    write(new_fd, &c, sizeof(struct cart));

    int total = 0;

    for (int i=0; i<MAX_PROD; i++){

        if (c.products[i].id != -1){
            write(new_fd, &c.products[i].quantity, sizeof(int));

            struct flock lock_prod;
            productReadLock(fd, lock_prod);
            lseek(fd, 0, SEEK_SET);

            struct Product p;
            int flg = 0;
            while (read(fd, &p, sizeof(struct Product))){

                if (p.id == c.products[i].id && p.quantity > 0) {
                    int min ;
                    if (c.products[i].quantity >= p.quantity){
                        min = p.quantity;
                    }else{
                        min = c.products[i].quantity;
                    }

                    flg =1;
                    write(new_fd, &min, sizeof(int));
                    write(new_fd, &p.price, sizeof(int));
                    break;
                }
            }

            unlock(fd, lock_prod);

            if (!flg){
                //product got deleted midway
                int val = 0;
                write(new_fd, &val, sizeof(int));
                write(new_fd, &val, sizeof(int));
            }
        }      
    }

    char ch;
    read(new_fd, &ch, sizeof(char));

    for (int i=0; i<MAX_PROD; i++){

        struct flock lock_prod;
        productReadLock(fd, lock_prod);
        lseek(fd, 0, SEEK_SET);

        struct Product p;
        while (read(fd, &p, sizeof(struct Product))){

            if (p.id == c.products[i].id) {
                int min ;
                if (c.products[i].quantity >= p.quantity){
                    min = p.quantity;
                }else{
                    min = c.products[i].quantity;
                }
                unlock(fd, lock_prod);
                productWriteLock(fd, lock_prod);
                p.quantity = p.quantity - min;

                write(fd, &p, sizeof(struct Product));
                unlock(fd, lock_prod);
            }
        }

        unlock(fd, lock_prod);
    }
    
    cartOffsetLock(fd_cart, lock_cart, offset, 2);

    for (int i=0; i<MAX_PROD; i++){
        c.products[i].id = -1;
        strcpy(c.products[i].name, "");
        c.products[i].price = -1;
        c.products[i].quantity = -1;
    }

    write(fd_cart, &c, sizeof(struct cart));
    write(new_fd, &ch, sizeof(char));
    unlock(fd_cart, lock_cart);

    read(new_fd, &total, sizeof(int));
    read(new_fd, &c, sizeof(struct cart));

    int fd_rec = open("receipt.txt", O_CREAT | O_RDWR, 0777);
    write(fd_rec, "ProductID\tProductName\tQuantity\tPrice\n", strlen("ProductID\tProductName\tQuantity\tPrice\n"));
    char temp[100];
    for (int i=0; i<MAX_PROD; i++){
        if (c.products[i].id != -1){
            sprintf(temp, "%d\t%s\t%d\t%d\n", c.products[i].id, c.products[i].name, c.products[i].quantity, c.products[i].price);
            write(fd_rec, temp, strlen(temp));
        }
    }
    sprintf(temp, "Total - %d\n", total);
    write(fd_rec, temp, strlen(temp));
    close(fd_rec);
}

void generateAdminReceipt(int fd_admin, int fd)
{
    struct flock lock;
    productReadLock(fd, lock);
    write(fd_admin, "Current Inventory:\n", strlen("Current Inventory:\n"));
    write(fd_admin, "ProductID\tProductName\tQuantity\tPrice\n", strlen("ProductID\tProductName\tQuantity\tPrice\n"));

    lseek(fd, 0, SEEK_SET);
    struct Product p;
    while (read(fd, &p, sizeof(struct Product))){
        if (p.id != -1){
            char temp[100];
            sprintf(temp, "%d\t%s\t%d\t%d\n",p.id, p.name, p.quantity, p.price);
            write(fd_admin, temp, strlen(temp));
        }
    }
    unlock(fd, lock);
}