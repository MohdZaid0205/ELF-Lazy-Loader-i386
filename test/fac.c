
int factorial(int n){
    if (n<=1)
        return 1;
    return n* factorial(n-1);
}

int _start(){
    int result = factorial(10);
    return result;
}