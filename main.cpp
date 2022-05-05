#include <stdio.h>
#include "base64.h"


int main(int argc, char **argv)
{
    
    //char filepath[]="E:/Dokument/C/workspace/div/stuff/kopl.jpg";   //path to read from
    char filepath[]="C:/Users/ander/skola/ll1302proj/kodlek/stuff/garb/kopl.jpg";   //path to read from
    FILE *jpgFile;
    jpgFile = fopen(filepath,"rb");//open file in bitread mode
    if(jpgFile!=NULL)
    {

        fseek(jpgFile,0,SEEK_END);
        int jpgSize=ftell(jpgFile);     //get size of file
        fseek (jpgFile,0,SEEK_SET);
        
        unsigned char buffer[jpgSize+1];
        fread(buffer,jpgSize,1,jpgFile);    //read image data
        
        std::string toEncode(reinterpret_cast<char*>(buffer),jpgSize);
        std::string encoded = base64_encode(reinterpret_cast<const unsigned char*>(toEncode.c_str()), toEncode.length());
        
        //decode and write
        std::string decoded = base64_decode(encoded);

        FILE *testFile;
        char filepath2[]="C:/Users/ander/skola/ll1302proj/kodlek/stuff/garb/test4.jpg";     
        testFile = fopen(filepath2,"wb");                                        
        fwrite(decoded.c_str(),decoded.length()+1,1,testFile);

        fclose(testFile);
    }
    else
    {
        //better error handling?
        //printf("failed to load image");
    }
    
    
    fclose(jpgFile);
     
    return 0;
}
