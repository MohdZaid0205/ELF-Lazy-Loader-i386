int health = 0x00;

void increment(int n){
    health += n;
}

void decrement(int n){
    health -= n;
}

int _start(){
    increment(5);
    decrement(3);
    return health;
}