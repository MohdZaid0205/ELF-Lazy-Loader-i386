const char* message = "this is a message that is to be declared";

int _start(){
    int length = 0;
    while (message[length] != '\0'){
        length ++;
    }
    
    return length;
}