void *Sender(void *);
void *Receive(void *);

struct shared {
    int working;
  	int some_one_writting;  
    char text[1024];
};