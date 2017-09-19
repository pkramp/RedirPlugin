      #include <unistd.h>
      #include<iostream>
int main(int argc,char**argv){

int err=access( argv[1] , F_OK );
std::cout<<"err="<<err<<std::endl;
return err;
}
